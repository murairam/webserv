/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:51:12 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/09/26 14:44:15 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "EventLoop.hpp"
#include "HttpRequestParser.hpp"
#include "Response.hpp"

Connection::~Connection(void)
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

Connection::Connection(int fd, const std::string &server_name, const ServerConfig *server)
:_fd(fd), _loop(0), _server_name(server_name), _inbuf(),
_outbuf(), _engaged(false), _should_close(false), _server(server),
_method(0)
{
	(void)set_nonblock_fd_nothrow(_fd);
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
	char	buf[8192];
	ssize_t	n = ::recv(_fd, buf, static_cast<int>(8192u), 0);
	(void)fd;

	if (n > 0)
	{
		_inbuf.append(buf, static_cast<size_t>(n));
		dispatcher();
	}
	else if (!n)
		_should_close = true;
	/* n < 0 must be ignored, otherwise this shit blocks and all fuck up*/
}

void	Connection::dispatcher(void)
{
#ifdef _DEBUG
	std::cerr << "\n=== CONNECTION DISPATCHER START ===" << std::endl;
	std::cerr << "Input buffer size: " << _inbuf.size() << " bytes" << std::endl;
#endif

	if (_inbuf.empty())
	{
		_should_close = true;
		return;
	}

	// TRY NEW PARSER FIRST
	if (handleRequestWithNewParser())
	{
#ifdef _DEBUG
		std::cerr << "SUCCESS: New parser handled request, clearing input buffer" << std::endl;
#endif
		_inbuf.clear();  // Request handled successfully
		return;
	}

	// If we get here, the new parser failed - this shouldn't happen now
#ifdef _DEBUG
	std::cerr << "ERROR: New parser failed - this should not happen!" << std::endl;
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
#ifdef _DEBUG
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

		// Successful parse - handle the request
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
		if (body_limit >= 0 && static_cast<long>(request.getBody().length()) > body_limit) {
#ifdef _DEBUG
			std::cerr << "DEBUG: Body size " << request.getBody().length()
					  << " exceeds limit " << body_limit << std::endl;
#endif
			sendErrorResponse(413);  // Payload Too Large
			return;
		}
	}

	// Handle by method
	if (request.getMethod() == "GET")
	{
		handleGetRequest(request, loc);
	}
	else if (request.getMethod() == "POST")
	{
		handlePostRequest(request, loc);
	}
	else if (request.getMethod() == "DELETE")
	{
		handleDeleteRequest(request, loc);
	}
	else
	{
#ifdef _DEBUG
		std::cerr << "DEBUG: Unsupported method: " << request.getMethod() << std::endl;
#endif
		sendErrorResponse(405);  // Method Not Allowed
	}
}

void Connection::handleGetRequest(const HttpRequest& request, const LocationConfig* loc)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING GET REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
	std::cerr << "DEBUG: Location root: " << loc->getRoot() << std::endl;
#endif

	// Check for redirect
	if (loc->getRedirectCode() > 0) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Redirect configured, code: " << loc->getRedirectCode()
				  << ", target: " << loc->getRedirectTarget() << std::endl;
#endif
		std::string location = loc->getRedirectTarget();
		sendRedirectResponse(loc->getRedirectCode(), location);
		return;
	}
	// Check for CGI first
	std::string extension = getFileExtension(request.getPath());
	if (!extension.empty()) {
		std::string cgi_program = loc->getCgi(extension);
		if (!cgi_program.empty()) {
#ifdef _DEBUG
        std::cerr << "DEBUG: CGI handler found for extension " << extension
                  << ": " << cgi_program << std::endl;
#endif
        handleCgiRequest(request, loc, cgi_program);
        return;  // Important: return after CGI handling
	    }
	}

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
				if (serveFile(index_path)) {
#ifdef _DEBUG
					std::cerr << "DEBUG: Successfully served index file: " << index_files[i] << std::endl;
#endif
					found_index = true;
					break;
				}
			}
		}

		// If no index file found, try autoindex
		if (!found_index) {
			if (loc->getAutoindex()) {
#ifdef _DEBUG
				std::cerr << "DEBUG: No index file found, generating directory listing" << std::endl;
#endif
				sendDirectoryListing(file_path, request.getPath());
			} else {
#ifdef _DEBUG
				std::cerr << "DEBUG: No index file found and autoindex disabled" << std::endl;
#endif
				sendErrorResponse(403);  // Forbidden
			}
		}
		return;
	}

	// Regular file serving
#ifdef _DEBUG
	std::cerr << "DEBUG: Attempting to serve regular file: " << file_path << std::endl;
#endif
	if (!serveFile(file_path)) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Failed to serve file: " << file_path << std::endl;
#endif
		sendErrorResponse(404);
	} else {
#ifdef _DEBUG
		std::cerr << "DEBUG: Successfully served file: " << file_path << std::endl;
#endif
	}
}

void Connection::handlePostRequest(const HttpRequest& request, const LocationConfig* loc)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING POST REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
	std::cerr << "DEBUG: Body length: " << request.getBody().length() << " bytes" << std::endl;
	std::cerr << "DEBUG: Content-Type: " << request.getHeader("content-type") << std::endl;
#endif

	// Handle file upload
	if (loc->getUploadEnabled()) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Upload is enabled for this location" << std::endl;
#endif
		handleFileUpload(request, loc);
		return;
	}

	// Check for CGI
	std::string extension = getFileExtension(request.getPath());
	if (!extension.empty()) {
		std::string cgi_program = loc->getCgi(extension);
		if (!cgi_program.empty()) {
#ifdef _DEBUG
			std::cerr << "DEBUG: CGI handler found for extension " << extension
					  << ": " << cgi_program << std::endl;
#endif
			handleCgiRequest(request, loc, cgi_program);
			return;
		}
	}

	// Default POST response
#ifdef _DEBUG
	std::cerr << "DEBUG: No special POST handling, sending default response" << std::endl;
#endif
	std::string response_body = "POST request processed successfully";
	sendSimpleResponse(200, "text/plain", response_body);
}

void Connection::handleDeleteRequest(const HttpRequest& request, const LocationConfig* loc)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: === HANDLING DELETE REQUEST ===" << std::endl;
	std::cerr << "DEBUG: Path: " << request.getPath() << std::endl;
#endif

	// Build file path
	std::string file_path = buildFilePath(loc, request.getPath());
#ifdef _DEBUG
	std::cerr << "DEBUG: Target file: " << file_path << std::endl;
#endif

	// Check if file exists
	struct stat st;
	if (stat(file_path.c_str(), &st) != 0) {
#ifdef _DEBUG
		std::cerr << "DEBUG: File does not exist: " << file_path << std::endl;
#endif
		sendErrorResponse(404);
		return;
	}

	// Don't allow deleting directories
	if (S_ISDIR(st.st_mode)) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Cannot delete directory: " << file_path << std::endl;
#endif
		sendErrorResponse(403);
		return;
	}

	// Attempt to delete
	if (unlink(file_path.c_str()) == 0) {
#ifdef _DEBUG
		std::cerr << "DEBUG: Successfully deleted file: " << file_path << std::endl;
#endif
		sendSimpleResponse(200, "text/plain", "File deleted successfully");
	} else {
#ifdef _DEBUG
		std::cerr << "DEBUG: Failed to delete file (permission denied): " << file_path << std::endl;
#endif
		sendErrorResponse(403);  // Forbidden (permission denied)
	}
}

// Helper methods

void Connection::sendSimpleResponse(int code, const std::string& content_type, const std::string& body)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Sending simple response - Code: " << code
			  << ", Content-Type: " << content_type
			  << ", Body length: " << body.length() << std::endl;
#endif

	std::ostringstream oss;
	oss << body.length();

	std::string response =
		"HTTP/1.1 " + intToString(code) + " " + getReasonPhrase(code) + "\r\n"
		"Content-Type: " + content_type + "\r\n"
		"Content-Length: " + oss.str() + "\r\n"
		"Connection: close\r\n"
		"\r\n" + body;

	queueWrite(response);
	requestClose();
}

void Connection::sendRedirectResponse(int code, const std::string& location)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Sending redirect response - Code: " << code
			  << ", Location: " << location << std::endl;
#endif

	std::string response =
		"HTTP/1.1 " + intToString(code) + " " + getReasonPhrase(code) + "\r\n"
		"Location: " + location + "\r\n"
		"Content-Length: 0\r\n"
		"Connection: close\r\n\r\n";

	queueWrite(response);
	requestClose();
}

void Connection::sendDirectoryListing(const std::string& dir_path, const std::string& uri)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: Generating directory listing for: " << dir_path << std::endl;
#endif

	Response response = Response::createDirectoryListing(dir_path, uri);
	queueWrite(response.serialize());
	requestClose();
}

void Connection::handleFileUpload(const HttpRequest& request, const LocationConfig* loc)
{
#ifdef _DEBUG
    std::cerr << "DEBUG: Handling file upload, upload path: " << loc->getUploadPath() << std::endl;
    std::cerr << "DEBUG: Upload body size: " << request.getBody().length() << " bytes" << std::endl;
    std::cerr << "DEBUG: Upload content-type: " << request.getHeader("content-type") << std::endl;
#endif

    std::string upload_path = loc->getUploadPath();
    if (upload_path.empty()) {
#ifdef _DEBUG
        std::cerr << "DEBUG: Upload path is empty" << std::endl;
#endif
        sendErrorResponse(500);
        return;
    }

    // Build response with upload info
    std::ostringstream response_body;
    response_body << "File upload received successfully!\n";
    response_body << "Content-Type: " << request.getHeader("content-type") << "\n";
    response_body << "Body size: " << request.getBody().length() << " bytes\n";
    response_body << "Upload path: " << upload_path << "\n";

    sendSimpleResponse(200, "text/plain", response_body.str());
}

void Connection::handleCgiRequest(const HttpRequest& request, const LocationConfig* loc, const std::string& cgi_program)
{
#ifdef _DEBUG
	std::cerr << "DEBUG: CGI request detected but not implemented yet" << std::endl;
	std::cerr << "DEBUG: CGI program: " << cgi_program << std::endl;
	std::cerr << "DEBUG: Request path: " << request.getPath() << std::endl;
#endif

	(void)request;
	(void)loc;
	(void)cgi_program;
	sendSimpleResponse(501, "text/plain", "CGI not implemented yet - perfect place for CgiHandler!");
}

std::string Connection::buildFilePath(const LocationConfig *loc, const std::string &target)
{
	std::string file_path = loc->getRoot();

	// Handle root path "/"
	if (target == "/") {
		const std::vector<std::string> &index_files = loc->getIndexFiles();
		if (!index_files.empty()) {
			file_path += "/" + index_files[0];  // Use first index file
		} else {
			file_path += "/index.html";  // Default fallback
		}
	} else {
		// Remove leading slash if root doesn't end with one
		if (file_path.size() > 0 && file_path[file_path.size() - 1] != '/' && target.size() > 0 && target[0] == '/') {
			file_path += target;
		} else if (file_path.size() > 0 && file_path[file_path.size() - 1] == '/' && target.size() > 0 && target[0] == '/') {
			file_path += target.substr(1);  // Remove duplicate slash
		} else {
			file_path += "/" + target;
		}
	}

	return file_path;
}

bool Connection::serveFile(const std::string &file_path)
{
	std::ifstream file(file_path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	// Read file content
	std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
	file.close();

	// Determine content type
	std::string content_type = getContentType(file_path);

	// Build and send response
	std::ostringstream oss;
	oss << content.length();
	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: " + content_type + "\r\n"
		"Content-Length: " + oss.str() + "\r\n"
		"Connection: close\r\n"
		"\r\n" + content;

	queueWrite(response);
	requestClose();
	return true;
}

void Connection::sendErrorResponse(int code)
{
#ifdef _DEBUG
    std::cerr << "DEBUG: Sending error response: " << code << " " << getReasonPhrase(code) << std::endl;
#endif

	const std::string& error_page = _server->getErrorPage(code);
	std::ifstream file(error_page.c_str());
	if (file) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		std::string response =
			"HTTP/1.1 " + intToString(code) + " " + getReasonPhrase(code) + "\r\n"
			"Content-Type: text/html\r\n"
			"Content-Length: " + intToString(body.size()) + "\r\n"
			"Connection: close\r\n\r\n" +
			body;
		queueWrite(response);
		requestClose();
		return;
	}

	std::string error_response =
		"HTTP/1.1 " + intToString(code) + " " + getReasonPhrase(code) + "\r\n"
		"Content-Length: 0\r\n"
		"Connection: close\r\n\r\n";
	queueWrite(error_response);
	requestClose();
}

std::string Connection::getContentType(const std::string &file_path) const
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

std::string Connection::getFileExtension(const std::string& path) const
{
	size_t dot_pos = path.find_last_of('.');
	if (dot_pos != std::string::npos && dot_pos < path.length() - 1) {
		return path.substr(dot_pos);
	}
	return "";
}

std::string Connection::getReasonPhrase(int code) const
{
	switch (code) {
		case 200: return "OK";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		default: return "Unknown";
	}
}

std::string Connection::intToString(int value) const
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

void	Connection::onWritable(int fd)
{
	ssize_t	n;
	int		events = EVENT_READ;
	(void)fd;

	if (!_outbuf.empty())
	{
		n = ::send(_fd, _outbuf.data(), static_cast<int>(_outbuf.size()), 0);
		if (n > 0)
			_outbuf.erase(0, static_cast<size_t>(n));
	}
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
	if (_should_close && _outbuf.empty())
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

	if (_should_close && _outbuf.empty())
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
