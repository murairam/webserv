/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:51:12 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/10/04 16:01:01 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::~Connection(void)
{
	if (_cgi)
	{
		_cgi->removeFromEventLoop();
		delete _cgi;
		_cgi = NULL;
	}
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

Connection::Connection(int fd, const std::string &server_name, const ServerConfig *server, const std::vector<const ServerConfig*> &servers)
:_fd(fd), _loop(0), _server_name(server_name), _inbuf(),
_outbuf(), _engaged(false), _should_close(false), _server(server),
_available_servers(servers), _method(0), _cgi(0),
_request_metadata_ready(false), _request_chunked(false),
_request_content_length(-1), _request_body_limit(0),
_has_request_body_limit(false),
_request_header_end(0), _chunk_total_bytes(0), _chunk_scan_offset(0), _pending_cgi(false)
{
	(void)set_nonblock_fd_nothrow(_fd);
	if (_available_servers.empty() && _server)
		_available_servers.push_back(_server);
	if (!_server && !_available_servers.empty())
		_server = _available_servers[0];
	if (_server)
		_server_name = _server->getServerName();
	resetRequestState();
}

void	Connection::resetRequestState(void)
{
	_request_metadata_ready = false;
	_request_chunked = false;
	_request_content_length = -1;
	_request_body_limit = 0;
	_has_request_body_limit = false;
	_request_header_end = 0;
	_chunk_total_bytes = 0;
	_chunk_scan_offset = 0;
	_pending_cgi = false;
}

const ServerConfig	*Connection::selectDefaultServer(void) const
{
	if (_server)
		return (_server);
	if (!_available_servers.empty())
		return (_available_servers[0]);
	return (0);
}

bool	Connection::parseRequestMetadata(void)
{
	if (_request_metadata_ready)
		return (true);
	std::string::size_type header_end = _inbuf.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (false);

	_request_header_end = header_end + 4;
	_request_chunked = false;
	_request_content_length = -1;

	std::string header_block = _inbuf.substr(0, header_end);
	std::istringstream stream(header_block);
	std::string line;

	if (!std::getline(stream, line))
		return (false);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			break;
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string name = toLower(trim(line.substr(0, colon)));
		std::string value = trim(line.substr(colon + 1));
		if (name == "content-length")
		{
			long parsed = std::strtol(value.c_str(), NULL, 10);
			if (parsed >= 0)
				_request_content_length = parsed;
		}
		else if (name == "transfer-encoding")
		{
			std::string lower_value = toLower(value);
			if (lower_value.find("chunked") != std::string::npos)
				_request_chunked = true;
		}
	}

	const ServerConfig *active = selectDefaultServer();
	long	configured_limit = (active ? active->getBodyLimit(0) : -1);
	if (configured_limit >= 0)
	{
		_has_request_body_limit = true;
		_request_body_limit = static_cast<size_t>(configured_limit);
	}
	else
	{
		_has_request_body_limit = false;
		_request_body_limit = 0;
	}

	_request_metadata_ready = true;
	_chunk_scan_offset = _request_header_end;
	_chunk_total_bytes = 0;
#ifdef _DEBUG
	std::cerr << "DEBUG: Parsed metadata - content-length ("
			  << _request_content_length << "), chunked "
			  << (_request_chunked ? "true" : "false")
			  << ", limit "
			  << (_has_request_body_limit ? static_cast<long>(_request_body_limit) : -1)
			  << std::endl;
#endif
	return (true);
}

bool	Connection::enforceRequestBodyLimit(void)
{
	if (_inbuf.empty())
		return (false);
	if (!parseRequestMetadata())
		return (false);
	if (!_has_request_body_limit)
		return (false);

	if (_request_content_length >= 0)
	{
		unsigned long long content_length = static_cast<unsigned long long>(_request_content_length);
		if (content_length > static_cast<unsigned long long>(_request_body_limit))
		{
#ifdef _DEBUG
		std::cerr << "DEBUG: Early body limit violation detected (Content-Length exceeds limit)" << std::endl;
#endif
		_should_close = true;
		sendErrorResponse(413);
		_inbuf.clear();
		resetRequestState();
		return (true);
		}
	}

	if (_request_chunked)
	{
		if (checkChunkedBodyLimit())
			return (true);
	}

	size_t body_bytes = 0;
	if (_inbuf.size() > _request_header_end)
		body_bytes = _inbuf.size() - _request_header_end;

	if (!_request_chunked && _request_content_length < 0 && body_bytes > _request_body_limit)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Early body limit violation detected (body bytes exceed limit without Content-Length)" << std::endl;
#endif
		_should_close = true;
		sendErrorResponse(413);
		_inbuf.clear();
		resetRequestState();
		return (true);
	}

	if (_request_content_length >= 0 && body_bytes > _request_body_limit)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Early body limit violation detected (received body exceeds limit)" << std::endl;
#endif
		_should_close = true;
		sendErrorResponse(413);
		_inbuf.clear();
		resetRequestState();
		return (true);
	}

	return (false);
}

bool	Connection::checkChunkedBodyLimit(void)
{
	if (!_request_chunked || !_has_request_body_limit)
		return (false);
	if (_chunk_scan_offset < _request_header_end)
		_chunk_scan_offset = _request_header_end;

	size_t	cursor = _chunk_scan_offset;
	size_t	total = _chunk_total_bytes;
	const std::string	&buffer = _inbuf;

	while (true)
	{
		size_t header_pos = cursor;

		if (header_pos >= buffer.size())
			break;

		size_t line_end = buffer.find("\r\n", header_pos);
		if (line_end == std::string::npos)
		{
			cursor = header_pos;
			break;
		}
		std::string size_line = buffer.substr(header_pos, line_end - header_pos);
		size_t	semicolon = size_line.find(';');
		if (semicolon != std::string::npos)
			size_line = size_line.substr(0, semicolon);
		size_line = trim(size_line);
		if (size_line.empty())
		{
			cursor = line_end + 2;
			continue;
		}
		int	err_code = 0;
		char	*endptr = NULL;
		errno = 0;
		unsigned long parsed_size = std::strtoul(size_line.c_str(), &endptr, 16);
		err_code = errno;
		if (err_code == ERANGE || endptr == size_line.c_str() || (endptr && *endptr != '\0'))
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: Invalid chunk size line encountered: " << size_line << std::endl;
#endif
			_should_close = true;
			sendErrorResponse(400);
			_inbuf.clear();
			resetRequestState();
			return (true);
		}
		if (parsed_size > static_cast<unsigned long>(std::numeric_limits<size_t>::max()))
		{
			_should_close = true;
			sendErrorResponse(413);
			_inbuf.clear();
			resetRequestState();
			return (true);
		}
		size_t	chunk_size = static_cast<size_t>(parsed_size);
		cursor = line_end + 2;
		if (chunk_size == 0)
		{
			_chunk_total_bytes = total;
			_chunk_scan_offset = cursor;
			return (false);
		}
		size_t	limit = _request_body_limit;
		if (chunk_size > limit)
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: Chunked body exceeds configured limit" << std::endl;
#endif
			_should_close = true;
			sendErrorResponse(413);
			_inbuf.clear();
			resetRequestState();
			return (true);
		}
		if (total > limit - chunk_size)
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: Chunked body exceeds configured limit" << std::endl;
#endif
			_should_close = true;
			sendErrorResponse(413);
			_inbuf.clear();
			resetRequestState();
			return (true);
		}
		if (cursor >= buffer.size())
		{
			cursor = header_pos;
			break;
		}
		size_t	remaining = buffer.size() - cursor;
		if (chunk_size > remaining)
		{
			cursor = header_pos;
			break;
		}
		size_t	chunk_end = cursor + chunk_size;
		if (chunk_end + 1 >= buffer.size())
		{
			cursor = header_pos;
			break;
		}
		if (buffer[chunk_end] != '\r' || buffer[chunk_end + 1] != '\n')
		{
			cursor = header_pos;
			break;
		}
		if (chunk_size > std::numeric_limits<size_t>::max() - total)
		{
			_should_close = true;
			sendErrorResponse(413);
			_inbuf.clear();
			resetRequestState();
			return (true);
		}
		total += chunk_size;
		cursor = chunk_end + 2;
	}
	_chunk_total_bytes = total;
	_chunk_scan_offset = cursor;
	return (false);
}

void	Connection::engageLoop(EventLoop &loop)
{
	int	events = EVENT_READ;

	if (_fd < 0)
		return;
	_loop = &loop;
	if (!_outbuf.empty())
		events = events | EVENT_WRITE;
	_loop->add(_fd, events, this);
	_engaged = true;
}

void	Connection::disengageLoop(void)
{
	if (_engaged && _loop)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::queueWrite(const std::string &data)
{
	int	events = EVENT_READ;

	if (_fd < 0)
		return ;
	_outbuf.append(data);
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
}

void	Connection::takeInput(std::string &dest)
{
	dest.swap(_inbuf);
}

void	Connection::requestClose(void)
{
	_should_close = true;
}

int	Connection::getFD(void) const
{
	return (_fd);
}

const std::string	&Connection::getServerName(void) const
{
	return (_server_name);
}

bool	Connection::isEngaged(void) const
{
	return (_engaged);
}

bool	Connection::isClose(void) const
{
	return (_fd < 0);
}

void	Connection::onReadable(int fd)
{
	char	buf[655360];
	bool	received_data = false;
	bool	remote_closed = false;

	(void)fd;
	ssize_t n = ::recv(_fd, buf, static_cast<int>(sizeof(buf)), 0);
	if (n > 0)
	{
		_inbuf.append(buf, static_cast<size_t>(n));
		received_data = true;
	}
	else if (n == 0)
	{
		remote_closed = true;
		_should_close = true;
	}
	else
	{
		this->onError(_fd);
		_inbuf.clear();
		resetRequestState();
		return;
	}
	if (received_data)
		dispatcher();
	if (remote_closed)
		_should_close = true;
	if (_should_close)
	{
		_inbuf.clear();
		resetRequestState();
	}
}

void	Connection::dispatcher(void)
{
#ifdef _DEBUG
std::string	preview = _inbuf.substr(0, std::min<std::string::size_type>(_inbuf.size(), 1024));

	std::cerr<<"\n=== INBOUND HTTP REQUEST ===\n"<<"Input buffer size: "<<_inbuf.size()<<" bytes\n---begin of request---\n"<<preview<<((preview.size() < _inbuf.size()) ?"\n--- [truncated]" : "---end of request---")<<std::endl;
#endif
	_should_close = false;
	if (_inbuf.empty())
	{
		_should_close = true;
		resetRequestState();
		return;
	}

	if (enforceRequestBodyLimit())
		return;
	// TRY NEW PARSER FIRST
	if (handleRequestWithNewParser())
	{
#ifdef _DEBUG
		std::cerr << "SUCCESS: New parser handled request, clearing input buffer" << std::endl;
#endif
		_inbuf.clear();  // Request handled successfully
		resetRequestState();
		return;
	}

	// If we get here, check if it's just an incomplete request
	if (!HttpRequestParser::isCompleteRequest(_inbuf))
		return;  // Keep connection open, wait for more data

	// If we get here, the new parser failed on a complete request - this shouldn't happen now
#ifdef _DEBUG
	std::cerr << "ERROR: New parser failed on complete request - this should not happen!" << std::endl;
#endif
	_should_close = true;
}

bool Connection::handleRequestWithNewParser(void)
{
	try
	{
		// Check if we have a complete request
		if (!HttpRequestParser::isCompleteRequest(_inbuf))
		{
#ifdef _DEBUG2
			std::cerr << "DEBUG: Incomplete request, waiting for more data" << std::endl;
#endif
			return false;  // Wait for more data
		}

#ifdef _DEBUG
		std::cerr << "DEBUG: Complete request detected, parsing..." << std::endl;
#endif

		// Parse the request
		HttpRequest request = HttpRequestParser::parse(_inbuf);

		// Check for parsing errors
		if (request.getShouldReject())
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: Parser rejected request with code " << request.getErrorCode() << std::endl;
#endif
			sendErrorResponse(request.getErrorCode());
			return true;  // Handled (with error)
		}

#ifdef _DEBUG
		std::cerr << "DEBUG: Successfully parsed " << request.getMethod()
				  << " request for " << request.getPath() << std::endl;
		if (!request.getQuery().empty())
			std::cerr << "DEBUG: Query string: " << request.getQuery() << std::endl;
		if (!request.getBody().empty())
			std::cerr << "DEBUG: Body length: " << request.getBody().length() << " bytes" << std::endl;
		std::cerr << "DEBUG: Headers count: " << request.toCgiEnvironment().size() << std::endl;
#endif
		if (!request.getPersistent())
			_should_close = true;
		else
			_should_close = false;
		// Successful parse - handle the request
		#ifdef	_DEBUG
		std::cerr<<"Server_name for the current connection is: "<<_server_name<<std::endl;
		#endif
		handleParsedRequest(request);
		return true;  // Successfully handled
	}
	catch (const std::exception& e)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: New parser exception: " << e.what() << std::endl;
#endif
		return false;  // This triggers error handling above
	}
}

void Connection::handleParsedRequest(const HttpRequest& request)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Handling parsed request - Method: " << request.getMethod()
			  << ", Path: " << request.getPath() << std::endl;
#endif

	// Set method for compatibility with existing code
	_method = MethodTokenToMask(request.getMethod());

	if (!selectServerForRequest(request))
		return;
	if (!_server)
	{
		sendErrorResponse(500);
		return;
	}

#ifdef _DEBUG
std::cerr << "DEBUG: Looking for location match for: " << request.getPath() << std::endl;
std::cerr << "DEBUG: About to call matchLocation..." << std::endl;
#endif

	const LocationConfig *loc = _server->matchLocation(request.getPath());

#ifdef _DEBUG
std::cerr << "DEBUG: matchLocation returned" << std::endl;
#endif
	if (!loc) {
#ifdef _DEBUG
    std::cerr << "DEBUG: No matching location found for " << request.getPath() << std::endl;
#endif
		sendErrorResponse(404);
		return;
	}

#ifdef _DEBUG
std::cerr << "DEBUG: Found location match, prefix: " << loc->getPathPrefix() << std::endl;
#endif

	if (loc->getRedirectCode() > 0)
	{
		sendRedirectResponse(loc->getRedirectCode(),
		resolveRedirectLocation(loc->getRedirectTarget(), request));
		return;
	}

	// Check if method is allowed
	if (!loc->MethodIsAllowed(_method)) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Method " << request.getMethod() << " not allowed in this location" << std::endl;
#endif
		sendErrorResponse(405);  // Method Not Allowed
		return;
	}

	// Check body size limits (for POST requests)
	if (request.getMethod() == "POST") {
		long body_limit = _server->getBodyLimit(loc);
		if (body_limit >= 0) {
			size_t limit_bytes = static_cast<size_t>(body_limit);
			if (request.getBody().length() > limit_bytes) {
#ifdef _DEBUG
			std::cerr << "DEBUG: Body size " << request.getBody().length()
					  << " exceeds limit " << body_limit << std::endl;
#endif
			sendErrorResponse(413);  // Payload Too Large
			return;
			}
		}
	}

	// Handle by method
	if (request.getMethod() == "GET")
		handleGetRequest(request, loc);
	else if (request.getMethod() == "POST" || request.getMethod() == "PUT")
		handlePostRequest(request, loc, request.getMethod());
	else if (request.getMethod() == "DELETE")
		handleDeleteRequest(request, loc);
	else
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Unsupported method: " << request.getMethod() << std::endl;
#endif
		sendErrorResponse(405);  // Method Not Allowed
	}
}

void	Connection::handleGetRequest(const HttpRequest& request, const LocationConfig* loc)
{
	int	err_code = 0;
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING GET REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
	std::cerr << "DEBUG: Location alias: " << loc->getAlias() << std::endl;
#endif

	// Check for redirect
	if (loc->getRedirectCode() > 0) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Redirect configured, code: " << loc->getRedirectCode()
				  << ", target: " << loc->getRedirectTarget() << std::endl;
#endif
		std::string location = resolveRedirectLocation(loc->getRedirectTarget(), request);
		sendRedirectResponse(loc->getRedirectCode(), location);
		return;
	}
	// Check for CGI first
	std::string extension = getFileExtension(request.getPath());
	if (!extension.empty())
	{
		std::string cgi_program = loc->getCgi(extension);
		if (!cgi_program.empty())
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: CGI handler found for extension " << extension<< ": " << cgi_program << std::endl;
#endif
			handleCgiRequest(request, loc, cgi_program);
			return;  // Important: return after CGI handling
		}
	}
#ifdef	_TESTER_VERSION
// Build file path
	std::string file_path = buildFilePath(loc, request.getPath());
#ifdef _DEBUG
	std::cerr << "DEBUG: Built file path: " << file_path << std::endl;
#endif

	// Check if it's a directory
	if (isDirectory(file_path)) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Path is a directory, checking for index files" << std::endl;
#endif
		// Try index files
		const std::vector<std::string> &index_files = loc->getIndexFiles();
		bool found_index = false;

		for (size_t i = 0; i < index_files.size(); ++i) {
			std::string index_path = file_path + "/" + index_files[i];
#ifdef _DEBUG
			std::cerr << "DEBUG: Trying index file: " << index_path << std::endl;
#endif
			if (!isDirectory(index_path)) {
				if (serveFile(index_path, err_code)) {
#ifdef _DEBUG
					std::cerr << "DEBUG: Successfully served index file: " << index_files[i] << std::endl;
#endif
					found_index = true;
					break;
				}
			}
		}
		// If no index file found, try autoindex
		if (!found_index)
		{
			// Only allow directory listing if path ends with '/' or if explicitly requested
		std::string path = request.getPath();
		bool allow_directory_listing = (path.empty() || path[path.length() - 1] == '/');
		if (allow_directory_listing && loc->getAutoindex())
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: No index file found, generating directory listing" << std::endl;
#endif
			sendDirectoryListing(file_path, request.getPath());
		}
		else
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: No index file found, returning 404" << std::endl;
#endif
				sendErrorResponse(404);  // Not Found
		}
		}
		return;
	}
#else
	// Build file path with normalized target
	std::string	raw_path = request.getPath();
	std::string	effective_path = raw_path.empty() ? std::string("/") : raw_path;
	bool	request_has_trailing_slash = (!effective_path.empty() 
		&& effective_path[effective_path.size() - 1] == '/');
	std::string normalized_target = effective_path;
	if (normalized_target.size() > 1)
		while (normalized_target.size() > 1 
			&& normalized_target[normalized_target.size() - 1] == '/')
				normalized_target.erase(normalized_target.size() - 1);
	std::string	file_path = buildFilePath(loc, normalized_target);
#ifdef _DEBUG
	std::cerr << "DEBUG: Built file path: " << file_path << std::endl;
#endif
		
	struct stat	path_stat;
	bool	path_exists = (::stat(file_path.c_str(), &path_stat) == 0);
	if (request_has_trailing_slash && path_exists && S_ISREG(path_stat.st_mode))
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: File requested with trailing slash, returning 404" << std::endl;
#endif
		sendErrorResponse(404);
		return;
	}

	if (!request_has_trailing_slash && path_exists && S_ISDIR(path_stat.st_mode))
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Directory requested without trailing slash, redirecting" << std::endl;
#endif
		std::string redirect_target = effective_path;
		redirect_target += "/";
		if (!request.getQuery().empty())
		{
			redirect_target += "?";
			redirect_target += request.getQuery();
		}
			sendRedirectResponse(301, redirect_target);
			return;
		}

		// Check if it's a directory
		if (path_exists && S_ISDIR(path_stat.st_mode))
		{
#ifdef _DEBUG
			std::cerr << "DEBUG: Path is a directory, checking for index files" << std::endl;
#endif
			// Try index files
			const std::vector<std::string>	&index_files = loc->getIndexFiles();
			bool	found_index = false;

			for (size_t i = 0; i < index_files.size(); ++i)
			{
				std::string index_path = file_path + "/" + index_files[i];
#ifdef _DEBUG
				std::cerr << "DEBUG: Trying index file: " << index_path << std::endl;
#endif
				if (!isDirectory(index_path))
				{
					if (serveFile(index_path, err_code))
					{
#ifdef _DEBUG
						std::cerr << "DEBUG: Successfully served index file: " << index_files[i] << std::endl;
#endif
						found_index = true;
						break;
					}
				}
			}
			// If no index file found, try autoindex
			if (!found_index)
			{
				// Only allow directory listing if path ends with '/' or if explicitly requested
				bool	allow_directory_listing = (!effective_path.empty() 
					&& effective_path[effective_path.length() - 1] == '/');

				if (allow_directory_listing && loc->getAutoindex())
                {
#ifdef _DEBUG
					std::cerr << "DEBUG: No index file found, generating directory listing" << std::endl;
#endif
					sendDirectoryListing(file_path, effective_path);
				}
				else
				{
#ifdef _DEBUG
					std::cerr << "DEBUG: No index file found, returning 404" << std::endl;
#endif
					sendErrorResponse(404);  // Not Found
				}
			}
			return;
		}
#endif /* _TESTER_VERSION */
	// Regular file serving
#ifdef _DEBUG
	std::cerr << "DEBUG: Attempting to serve regular file: " << file_path << std::endl;
#endif
	if (!serveFile(file_path, err_code))
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Failed to serve file: " << file_path << std::endl;
#endif
		sendErrorResponse(err_code);
	}
	else
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Successfully served file: " << file_path << std::endl;
#endif
	}
}

void	Connection::handlePostRequest(const HttpRequest& request, const LocationConfig* loc, const std::string &method)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING POST/PUT REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
	std::cerr << "DEBUG: Body length: " << request.getBody().length() << " bytes" << std::endl;
	std::cerr << "DEBUG: Content-Type: " << request.getHeader("content-type") << std::endl;
	if (!loc)
		std::cerr << "DEBUG: No matching location for upload check" << std::endl;
	else
		std::cerr<<"DEBUG: Upload is "<<(loc->getUploadEnabled() ? "enabled" : "disabled")
			<<" for this path: "<< loc->getPathPrefix()<<"/"<<loc->getAlias()<<std::endl;
#endif
	if (!loc)
	{
		sendErrorResponse(500);
		return;
	}
	// Check for CGI first (before upload handling)
	std::string extension = getFileExtension(request.getPath());
	if (!extension.empty())
	{
		std::string cgi_program = loc->getCgi(extension);
		if (!cgi_program.empty())
		{
			std::string script_path = buildFilePath(loc, request.getPath());
#ifdef _DEBUG
			std::cerr << "DEBUG: Evaluating CGI script candidate: " << script_path << std::endl;
			struct stat	st;
			bool script_ok = (!::stat(script_path.c_str(), &st)
				&& ::access(script_path.c_str(), R_OK) == 0);
			std::cerr << "DEBUG: CGI script candidate accessible: " << (script_ok ? "yes" : "no") << std::endl;
#endif
			std::cerr << "DEBUG: CGI handler found for extension " << extension<< ": " << cgi_program << std::endl;handleCgiRequest(request, loc, cgi_program);
			return;
		}
	}
	if (loc->getUploadEnabled())
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Handling file upload, upload path: " << loc->getAlias() << std::endl;
		std::cerr << "DEBUG: Upload body size: " << request.getBody().length() << " bytes" << std::endl;
		std::cerr << "DEBUG: Upload content-type: " << request.getHeader("content-type") << std::endl;
#endif
		std::string	response_body;
		int	status_code = 0;
		if (!uploadFile(request, loc, response_body, status_code, method))
		{
#ifdef _DEBUG
			std::cerr<<"DEBUG: File upload failed"<<std::endl;
#endif
			if (status_code < 1)
				status_code = 500;
			sendErrorResponse(status_code);
			return;
		}
		if (status_code < 1)
			status_code = 201;
		sendSimpleResponse(status_code, "text/plain", response_body);
		return;
	}
	std::string response_body = "POST/PUT request processed successfully";
	sendSimpleResponse(200, "text/plain", response_body);
}

void	Connection::handleDeleteRequest(const HttpRequest& request, const LocationConfig* loc)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING DELETE REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
#endif

	std::string file_path = buildFilePath(loc, request.getPath());
#ifdef _DEBUG
	std::cerr << "DEBUG: Resolved delete target: " << file_path << std::endl;
#endif

	struct stat st;
	if (stat(file_path.c_str(), &st) != 0)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: File does not exist for deletion" << std::endl;
#endif
		sendErrorResponse(404);
		return;
	}

	if (S_ISDIR(st.st_mode))
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Refusing to delete directory" << std::endl;
#endif
		sendErrorResponse(403);
		return;
	}

	if (!std::remove(file_path.c_str()))
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: File deleted successfully" << std::endl;
#endif
		sendSimpleResponse(204);
		return;
	}

	int err_code = errno;
#ifdef _DEBUG
	std::cerr << "DEBUG: std::remove failed: " << std::strerror(err_code) << std::endl;
#endif
	if (err_code == ENOENT)
		sendErrorResponse(404);
	else if (err_code == EACCES || err_code == EPERM || err_code == EBUSY || err_code == EROFS)
		sendErrorResponse(403);
	else
		sendErrorResponse(400);
}

bool	Connection::selectServerForRequest(const HttpRequest &request)
{
	if (_available_servers.empty())
		return (true);
	std::string host_header = request.getHeader("host");
	if (host_header.empty())
	{
		sendErrorResponse(400);
		return (false);
	}
	std::string	host = trim(host_header);
	std::string::size_type	colon = host.find(':');
	if (colon != std::string::npos)
		host = host.substr(0, colon);
	host = toLower(host);
	if (host.empty())
	{
		sendErrorResponse(400);
		return (false);
	}
	for (std::vector<const ServerConfig*>::const_iterator it = _available_servers.begin(); it != _available_servers.end(); ++it)
	{
		const ServerConfig *candidate = *it;
		if (!candidate)
			continue;
		if (toLower(candidate->getServerName()) == host)
		{
			_server = candidate;
			_server_name = candidate->getServerName();
			return (true);
		}
	}
	sendErrorResponse(421);
	return (false);
}

// Helper methods
void	Connection::sendSimpleResponse(int code, std::string content_type, std::string body)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Sending simple response - Code: " << code
			  << ", Content-Type: "<<content_type<< ", Body length: " << body.length() << std::endl;
#endif
	std::ostringstream oss;
	oss << body.length();
	std::string response =
		"HTTP/1.1 " + intToString(code) + " " + CodePage::getInstance().getReason(code) + "\r\nDate: " + getTimeString() + (body.empty() ? std::string() : std::string("\r\nContent-Type: "  + content_type + "\r\n" + "Content-Length: " + oss.str())) + "\r\n" + "Connection: " + ((!_should_close) ? "keep-alive\r\n" : "close\r\n") + "\r\n" + body;
	queueWrite(response);
}

std::string	Connection::resolveRedirectLocation
(const std::string &target, const HttpRequest &request) const
{
	if (target.empty())
		return (target);
	std::string	resolved = target;
	size_t		scheme_pos = target.find("://");
	if (scheme_pos != std::string::npos)
	{
		size_t	domain_start = scheme_pos + 3;
		size_t	path_pos = target.find('/', domain_start);
		if (path_pos == std::string::npos)
		{
			const std::string	&path = request.getPath();
			if (path.empty() || path == "/")
				resolved += "/";
		}
		if (resolved.find('?') == std::string::npos && !request.getQuery().empty())
			resolved += ('?' + request.getQuery());
		return (resolved);
	}
	if (!target.empty() && target[0] != '/')
	{
		std::string	base_path = request.getPath();
		if (base_path.empty() || base_path[0] != '/')
			base_path = std::string("/");
		if (!base_path.empty()
		&& base_path[base_path.size() - 1] != '/')
			base_path += '/';
		resolved = base_path + target;
	}
	if (resolved.find('?') == std::string::npos
	&& !request.getQuery().empty())
		resolved += ('?' + request.getQuery());
	return (resolved);
}

void	Connection::sendRedirectResponse(int code, const std::string &location)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Sending redirect response - Code: "<<code<< ", Location: " << location << std::endl;
#endif

	std::string	response =
		"HTTP/1.1 " + intToString(code) + " " + CodePage::getInstance().getReason(code) + "\r\nDate: " + getTimeString() + "\r\nLocation: " + location + "\r\n" + "Content-Length: 0\r\n" + "Connection: " + (!_should_close ? "keep-alive\r\n\r\n" : "close\r\n\r\n");

	queueWrite(response);
}

void	Connection::sendDirectoryListing
(const std::string& dir_path, const std::string& uri)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Generating directory listing for: " << dir_path << "\nDEBUG uri is: "<<uri<<std::endl;
#endif
	Response	response = Response::createDirectoryListing(dir_path, uri);
	response.setHeader("Connection", _should_close ? "close" : "keep-alive");
	queueWrite(response.serialize());
	requestClose();
}

void	Connection::handleCgiRequest
(const HttpRequest& request, const LocationConfig* loc,
	const std::string& cgi_program)
{
	std::string	script_path = buildFilePath(loc, request.getPath());
#ifdef _DEBUG
	std::cerr<<"DEBUG: CGI script path: "<<script_path<<std::endl;
#endif

	struct stat	st;
	int	stat_result = ::stat(script_path.c_str(), &st);
#ifdef	_DEBUG
	if (stat_result)
		std::cerr<<"DEBUG: stat() on CGI script failed: "<<std::strerror(errno)<<std::endl;
#endif
	if (!stat_result && S_ISDIR(st.st_mode))
	{
		sendErrorResponse(404);
		return;
	}
#ifdef	_DEBUG
	else if (stat_result || ::access(script_path.c_str(), R_OK))
		std::cerr<<"DEBUG: CGI script not accessible (continuing): "<<script_path<<std::endl;
	std::cerr << "DEBUG: Creating CGI handler..." << std::endl;
#endif

	try
	{
		_cgi = new CgiHandler(request, cgi_program, script_path, loc, this);
	}
	catch (const std::exception &e)
	{
		std::cerr<<"Unable to create CGI Handler: "<<e.what()<<std::endl;
		_cgi = NULL;
		sendErrorResponse(500);
		return;
	}
	catch (...)
	{
		std::cerr<<"Non-standard exception caught"<<std::endl;
		_cgi = NULL;
		sendErrorResponse(500);
		return;
	}

#ifdef _DEBUG
	std::cerr << "DEBUG: Executing CGI..." << std::endl;
#endif
	if (!_cgi->execute(*_loop))
	{
		delete _cgi;
		_cgi = NULL;
		sendErrorResponse(500);
	}
#ifdef _DEBUG
	std::cerr << "DEBUG: CGI execution started successfully" << std::endl;
#endif
}

void	Connection::checkCgi(void)
{
	// First check: ensure _cgi exists
	if (!_cgi)
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: checkCgi - _cgi is NULL, returning" << std::endl;
#endif
		return;
	}
#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: checkCgi called, _cgi->isDone()=" << (_cgi->isDone() ? "true" : "false") << std::endl;
#endif

	// Second check: ensure _cgi is done (this also validates _cgi is still valid)
	if (!_cgi->isDone())
		return;

#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: CGI is done, checking timeout..." << std::endl;
#endif

	// Check timeout
	if (_cgi->isTimeout())
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: CGI timeout detected" << std::endl;
#endif
		_cgi->removeFromEventLoop();
		delete _cgi;
		_cgi = NULL;
		sendErrorResponse(500);
		return;
	}

#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: Getting CGI response..." << std::endl;
#endif

	queueWrite(_cgi->getResponse());

#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: CGI response queued, removing from event loop..." << std::endl;
#endif

	// Remove CGI from event loop before deleting
	_cgi->removeFromEventLoop();

#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: CGI removed from event loop, deleting handler..." << std::endl;
#endif

	delete _cgi;
	_cgi = NULL;

#ifdef _DEBUG_CGI
	std::cerr << "DEBUG: CGI handler deleted successfully" << std::endl;
#endif
}
void	Connection::markCgiReady(void)
{
	_pending_cgi = true;
	if (_loop)
		_loop->notify();
}

std::string	Connection::buildFilePath(const LocationConfig *loc, const std::string &target)
{
	if (!loc)
		return (target);
	std::string	base_path = loc->getAlias();
	if (base_path.empty())
		base_path = std::string(".");
#ifdef _DEBUG
	std::cerr << "DEBUG: buildFilePath - base_path: " << base_path << std::endl;
	std::cerr << "DEBUG: buildFilePath - target: " << target << std::endl;
#endif
	std::string	prefix = loc->getPathPrefix();
	if (prefix.empty())
		prefix = "/";
	if (prefix[0] != '/')
		prefix.insert(prefix.begin(), '/');
	while (prefix.size() > 1 && prefix[prefix.size() - 1] == '/')
		prefix.erase(prefix.size() - 1);
	std::string	relative_path = target.empty() ? std::string("/") : target;
	if (prefix != "/" && !relative_path.compare(0, prefix.size(), prefix))
	{
		relative_path.erase(0, prefix.size());
		if (relative_path.empty())
			relative_path = "/";
	}
	if (relative_path == "/" || relative_path.empty())
	{
		if (base_path.size() > 1 && base_path[base_path.size() - 1] == '/')
			base_path.erase(base_path.size() - 1);
		return (base_path);
	}
	if (relative_path[0] == '/')
		relative_path.erase(0, 1);
#ifdef _DEBUG
	std::cerr << "DEBUG: buildFilePath - relative_path: " << relative_path << std::endl;
	std::cerr << "DEBUG: buildFilePath - final result: " << joinPath(base_path, relative_path) << std::endl;
#endif
	return (joinPath(base_path, relative_path));
}

bool	Connection::serveFile(const std::string &file_path, int &err_code)
{
	int	fd = ::open(file_path.c_str(), O_NOFOLLOW | O_RDONLY | O_NONBLOCK);
if (fd < 0)
	{
		err_code = errno;
		if (err_code == ENOENT || err_code == ENOTDIR)
			err_code = 404;  // Not Found
		else if (err_code == EACCES)
			err_code = 403;  // Forbidden
		else if (err_code == EISDIR || err_code == ELOOP || err_code == ENAMETOOLONG || err_code == EOVERFLOW || err_code == ENXIO)
			err_code = 400;  // Bad Request
		else if (err_code == ENOSR || err_code == ENFILE || err_code == EIO || err_code == EINVAL || err_code == EINTR)
			err_code = 500;  // Internal Server Error
		else
			err_code = 500;  // Default to Internal Server Error for unknown errors
		return (false);
	}
	(void)::close(fd);
	// Read file content
	std::ifstream	file(file_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		err_code = 500;
		return (false);
	}
	std::string	content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	file.close();

	// Determine content type
	std::string	content_type = getContentType(file_path);

	// Build and send response
	std::ostringstream	oss;
	oss << content.length();
	std::string	response = std::string("HTTP/1.1 200 OK\r\nDate: " + getTimeString() + "\r\nContent-Type: " + content_type + "\r\n" + "Content-Length: " + oss.str() + "\r\n" + +"Connection: " + (!_should_close ? "keep-alive\r\n" : "close\r\n") + "\r\n" + content);
	queueWrite(response);
	return (true);
}

void	Connection::sendErrorResponse(int code)
{
#ifdef _DEBUG
    std::cerr << "DEBUG: Sending error response: " << code << " " << CodePage::getInstance().getReason(code) << std::endl;
#endif
	std::string	error_page;
	if (_server)
		error_page = _server->getErrorPage(code);
	if (!error_page.empty())
	{
		std::ifstream	file(error_page.c_str());
		if (file)
		{
			std::stringstream	buffer;
			buffer << file.rdbuf();
			std::string	body = buffer.str();
			std::string	response =
			"HTTP/1.1 " + intToString(code) + " " +  CodePage::getInstance().getReason(code) + "\r\nDate: " + getTimeString() + "\r\nContent-Type: text/html\r\nContent-Length: " + intToString(body.size()) + "\r\nConnection: " + ((!_should_close) ? "keep-alive\r\n" : "close\r\n") + "\r\n" + body;
			queueWrite(response);
			return;
		}
	}
	else
	{
		std::string	response =
			"HTTP/1.1 " + intToString(code) + " " +  CodePage::getInstance().getReason(code) + "\r\nDate: " + getTimeString() + "\r\nContent-Type: text/html\r\nContent-Length: " + intToString(CodePage::getInstance().getCodePage(code).size()) + "\r\nConnection: " + ((!_should_close) ? "keep-alive\r\n" : "close\r\n") + "\r\n" + CodePage::getInstance().getCodePage(code);
		queueWrite(response);
	}
}

std::string	Connection::getContentType(const std::string &file_path) const
{
	if (file_path.find(".html") != std::string::npos ||
		file_path.find(".htm") != std::string::npos) {
		return "text/html";
	} else if (file_path.find(".css") != std::string::npos) {
		return "text/css";
	} else if (file_path.find(".js") != std::string::npos) {
		return "application/javascript";
	} else if (file_path.find(".png") != std::string::npos) {
		return "image/png";
	} else if (file_path.find(".jpg") != std::string::npos ||
			   file_path.find(".jpeg") != std::string::npos) {
		return "image/jpeg";
	}
	return "text/plain";
}

std::string	Connection::getFileExtension(const std::string& path) const
{
	size_t	dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos && dot_pos < path.length() - 1)
		return (path.substr(dot_pos));
	return (std::string());
}

std::string	Connection::intToString(int value) const
{
	std::ostringstream	oss;
	oss << value;
	return oss.str();
}

void	Connection::onWritable(int fd)
{
	int	events = EVENT_READ;
	bool	should_drop = false;
	(void)fd;

	if (!_outbuf.empty())
	{
#ifdef	_DEBUG
	std::string	preview = _outbuf.substr(0, std::min<std::string::size_type>(_outbuf.size(), 1024));
	std::cerr<<"\n===begin of response===\n"<<preview;
	if (preview.size() < _outbuf.size())
		std::cerr<<"\n... [truncated]\n";
	std::cerr<<"\n===end of response"<<std::endl;
#endif
	ssize_t n = ::send(_fd, _outbuf.data(), static_cast<int>(_outbuf.size()), 0);
	if (n > 0)
	{
		_outbuf.erase(0, static_cast<size_t>(n));
	}
	else if (n == 0)
	{
		_should_close = true;
	}
	else
	{
		this->onError(_fd);
		_outbuf.clear();
		should_drop = true;
	}
	}
	if (should_drop)
		return;
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
	if (_should_close && _outbuf.empty() && !_cgi)
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}

void	Connection::onError(int fd)
{
	(void)fd;

	if (_cgi)
	{
		_cgi->removeFromEventLoop();
		delete _cgi;
		_cgi = NULL;
	}
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onHangup(int fd)
{
	(void)fd;

	if (_cgi)
	{
		_cgi->removeFromEventLoop();
		delete _cgi;
		_cgi = NULL;
	}
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onTick(int fd)
{
	(void)fd;

	if (_pending_cgi || _cgi)
	{
		_pending_cgi = false;
		checkCgi();
	}
	if (_should_close && _outbuf.empty() && !_cgi)
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}

bool	Connection::handleMultipart(const HttpRequest &request, std::string &filename, std::size_t &content_offset, std::size_t &content_length)
{
	const std::string	content_type = request.getHeader("content-type");
	if (content_type.empty())
		return (false);
	std::string	lower_content_type = toLower(content_type);
	size_t		boundary_pos = lower_content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
		return (false);
	std::string	boundary = content_type.substr(boundary_pos + 9);
	size_t	semicolon = boundary.find(';');
	if (semicolon != std::string::npos)
		boundary.erase(semicolon);
	boundary = trim(boundary);
	if (!boundary.empty() && boundary[0] == '"'
	&& boundary[boundary.size() - 1] == '"')
		boundary = boundary.substr(1, boundary.size() - 2);
	if (boundary.empty())
		return (false);
	const std::string	boundary_marker = std::string("--") + boundary;
	const std::string	&body = request.getBody();
	size_t				search_pos = 0;
	content_offset = 0;
	content_length = 0;
	while (search_pos < body.size())
	{
		size_t	part_start = body.find(boundary_marker, search_pos);
		if (part_start == std::string::npos)
			break;
		size_t	cursor = part_start + boundary_marker.size();
		if (cursor + 2 <= body.size() && !body.compare(cursor, 2, "--"))
			break;
		if (cursor + 2 <= body.size() && !body.compare(cursor, 2, "\r\n"))
			cursor += 2;
		size_t	header_end = body.find("\r\n\r\n", cursor);
		if (header_end == std::string::npos)
			break;
		std::string	header_block = body.substr(cursor, header_end - cursor);
		cursor = header_end + 4;
		size_t	next_boundary = body.find(boundary_marker, cursor);
		if (next_boundary == std::string::npos)
			break;
		size_t	data_end = next_boundary;
		if (data_end >= 2 && body[data_end - 2] == '\r'
		&& body[data_end - 1] == '\n')
			data_end -= 2;
		size_t	data_start = cursor;
		cursor = next_boundary + boundary_marker.size();
		bool	final_boundary = false;
		if (cursor + 2 <= body.size() && !body.compare(cursor, 2, "--"))
		{
			final_boundary = true;
			cursor += 2;
		}
		else if (cursor + 2 <= body.size() && !body.compare(cursor, 2, "\r\n"))
			cursor += 2;
		search_pos = cursor;
		std::istringstream	header_stream(header_block);
		std::string			header_line;
		std::string			disposition;
		while (std::getline(header_stream, header_line))
		{
			if (!header_line.empty() && header_line[header_line.size() - 1] == '\r')
				header_line.erase(header_line.size() - 1);
			if (header_line.empty())
				continue;
   			size_t	colon = header_line.find(':');
			if (colon == std::string::npos)
				continue;
			std::string	name = toLower(trim(header_line.substr(0, colon)));
			std::string	value = trim(header_line.substr(colon + 1));
			if (name == "content-disposition")
				disposition = value;
		}
		if (!disposition.empty())
		{
			std::istringstream	disp_stream(disposition);
			std::string			token;
			while (std::getline(disp_stream, token, ';'))
			{
				std::string	trimmed = trim(token);
				std::string	lower = toLower(trimmed);
				if (!lower.find("filename="))
				{
					std::string	value = trimmed.substr(9);
					value = trim(value);
					if (!value.empty() && value[0] == '"' && value[value.size() - 1] == '"')
						value = value.substr(1, value.size() - 2);
					filename = value;
					content_offset = data_start;
					content_length = (data_end > data_start ? data_end - data_start : 0);
					return (true);
				}
			}
		}
		if (final_boundary)
			break;
	}
	return (false);
}

bool	Connection::uploadFile
(const HttpRequest &request, const LocationConfig *loc,
std::string &response_body, int &status_code, const std::string &method)
{
	if (!loc)
	{
		status_code = 500;
		return (false);
	}
	const std::string	upload_dir = loc->getAlias();
	if (upload_dir.empty())
	{
		status_code = 400;
		return (false);
	}
	struct stat	st;
	std::memset(&st, 0, sizeof(st));
	if (stat(upload_dir.c_str(), &st) || !S_ISDIR(st.st_mode))
	{
		status_code = 400;
		return (false);
	}
	std::string	filename;
	const std::string	&body = request.getBody();
	std::size_t	body_offset = 0;
	std::size_t	body_length = 0;
	const std::string	content_type = request.getHeader("content-type");
	if (content_type.find("multipart/form-data") != std::string::npos)
	{
		if (!Connection::handleMultipart(request, filename, body_offset, body_length))
		{
			status_code = 400;
			return (false);
		}
	}
	else
	{
		body_offset = 0;
		body_length = body.size();
		if (!extractFilename(request.getPath(), loc->getPathPrefix(), filename))
			filename = request.getUploadFilename();
	}
	if (filename.empty())
		filename = "file_" + getUniqueTimeString();
	std::string	cleanFilename;
	if (!sanitizeFilename(filename, cleanFilename))
	{
		status_code = 400;
		return (false);
	}
	std::string	destination = joinPath(upload_dir, cleanFilename);
	if (!::access(destination.c_str(), F_OK))
	{
		if (method == "POST")
		{
#ifdef	_TESTER_VERSION
			std::string	base = cleanFilename;
			unsigned int	attempts = 0;
			while (!::access(destination.c_str(), F_OK))
			{
				std::string	candidate = base + "_" + getUniqueTimeString();
				std::string	sanitized;
				if (!sanitizeFilename(candidate, sanitized))
				{
					status_code = 400;
					return (false);
				}
				cleanFilename.swap(sanitized);
				destination = joinPath(upload_dir, cleanFilename);
				if (++attempts > 64)
				{
					status_code = 500;
					return (false);
				}
			}
#else
			status_code = 409;
			return (false);
#endif
		}
		else if (method == "PUT")
		{
			if (std::remove(destination.c_str()) < 0)
			{
				if (errno != ENOENT)
				{
					#ifdef	_DEBUG
					std::cerr<<"std::remove failed: "<<destination<<strerror(errno)<<std::endl;
					#endif
					status_code = 400;
					return (false);
				}
			}
			if (!::access(destination.c_str(), F_OK))
			{
				status_code = 500;
				return (false);
			}
		}
	}
	std::ofstream	output(destination.c_str(), std::ios::binary | std::ios::trunc);
	if (!output.is_open())
	{
#ifdef	_DEBUG
		std::cerr<<destination.c_str()<<": "<<strerror(errno)<<std::endl;
#endif
		status_code = 500;
		return (false);
	}
	if (body_length > 0)
		output.write(body.c_str() + body_offset, static_cast<std::streamsize>(body_length));
	output.close();
	std::ostringstream	oss;
	oss<<"File stored successfully\nFilename: "<<cleanFilename
	<<"\nSize: "<<body_length<<" bytes\nLocation: "<<destination<<"\n";
	response_body = oss.str();
	status_code = 201;
	return (true);
}
