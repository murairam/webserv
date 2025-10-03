/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:35:23 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/10/03 11:01:52 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParser.hpp"
#include "utility.hpp"

// Private constructor prevents instantiation
HttpRequestParser::HttpRequestParser(void) {}
HttpRequestParser::HttpRequestParser(const HttpRequestParser &other) { (void)other; }
HttpRequestParser &HttpRequestParser::operator=(const HttpRequestParser &other) { (void)other; return *this; }
HttpRequestParser::~HttpRequestParser(void) {}

// Main parsing interface

HttpRequest HttpRequestParser::parse(std::istream &input)
{
	HttpRequest request;

	// Parse request line (GET /path HTTP/1.1)
	if (!parseRequestLine(input, request))
		return (request);  // Error already set

    // Parse headers (Host: example.com, etc.)
	if (!parseHeaders(input, request))
		return (request);  // Error already set
	
		// Check if an error was set during header parsing (e.g., Content-Length too large)
	if (request.getErrorCode())
		return (request);  // Error already set during header parsing
	// Parse body (for POST and PUT requests)
	if (!parseBody(input, request))
		return (request);  // Error already set

	// Validation: Host header is required in HTTP/1.1
	if (!request.hasHeader("host"))
	{
		setError(request, 400);
		return (request);
	}

	return (request);
}

HttpRequest HttpRequestParser::parse(const std::string &input)
{
    std::istringstream iss(input);
    return (parse(iss));
}

// Request line parsing (GET /path?query HTTP/1.1)

bool HttpRequestParser::parseRequestLine(std::istream &input, HttpRequest &request)
{
    std::string line;
    std::string method, url, version;

    // Read first line
    if (!std::getline(input, line))
    {
        setError(request, 400);
        return (false);
    }

    // Remove \r if present
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    // Parse: METHOD URL VERSION
    std::istringstream line_stream(line);
    if (!(line_stream >> method >> url >> version))
    {
        setError(request, 400);
        return (false);
    }

    // Validate method
    if (!isValidMethod(method))
    {
        setError(request, 405);
        return (false);
    }

    // Validate HTTP version
    if (!isValidHttpVersion(version))
    {
        setError(request, 505);
        return (false);
    }

    // Parse and sanitize inbound URL into path and query
    std::string path, query;
   if (!parseUrl(url, path, query))
   {
	setError(request, 400);
	return (false);
   }
#ifdef	_DEBUG
   std::cerr<<"\n---start of sanitized path---\n"<<path<<"\n---end of sanitized path---"<<std::endl;
#endif

    // Set request data
    request.setMethod(method);
    request.setPath(path);
    request.setQuery(query);
    request.setVersion(version);

    return (true);
}

// Header parsing

bool HttpRequestParser::parseHeaders(std::istream &input, HttpRequest &request)
{
    std::string line;

    while (std::getline(input, line))
    {
        // Remove \r if present
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Empty line marks end of headers
        if (line.empty())
            break;

        // Extract header name and value
        std::string name = extractHeaderName(line);
        std::string value = extractHeaderValue(line);

        if (name.empty())
        {
			setError(request, 400);
			return (false);
        }

        // Add to general headers map
		if (!request.addHeader(name, value))
		{
			setError(request, 400);
			return (false);
		}

        // Handle special headers that need extra processing
        std::string lower_name = toLower(name);
        if (lower_name == "host")
            parseHostHeader(value, request);
        else if (lower_name == "connection")
            parseConnectionHeader(value, request);
        else if (lower_name == "cookie")
            parseCookieHeader(value, request);
        else if (lower_name == "content-length")
            parseContentLengthHeader(value, request);
        else if (lower_name == "transfer-encoding")
            parseTransferEncodingHeader(value, request);
    }

    return (true);
}

// Body parsing

bool HttpRequestParser::parseBody(std::istream &input, HttpRequest &request)
{
    // GET and DELETE typically don't have bodies
    if (request.getMethod() == "GET" || request.getMethod() == "DELETE")
    {
        request.setBody("");
        return (true);
    }

    // Check for conflicting length indicators (forbidden by HTTP spec)
    if (request.getChunked() && request.getContentLength() >= 0)
    {
        setError(request, 400);
        return (false);
    }

    // Parse chunked body
    if (request.getChunked())
        return (parseChunkedBody(input, request));

    // Parse fixed-length body
    if (request.getContentLength() >= 0)
	{/*
		// Sanity check: prevent parsing extremely large bodies that could cause infinite loops
		// The actual body size limits will be enforced by the server configuration later
		const long	max = 50L * 1024L * 1024L;
		if (request.getContentLength() > max)
		{
			setError(request, 413);
			return (false);
		}*/
	/*	Let me try if we can do it without the hard limit.
	*/
        return (parseFixedLengthBody(input, request));
	}
    // No body
    request.setBody("");
    return (true);
}

// Chunked body parsing

bool HttpRequestParser::parseChunkedBody(std::istream &input, HttpRequest &request)
{
    std::string body;
    std::string line;

    while (true)
    {
        // Read chunk size line
        if (!std::getline(input, line))
        {
            setError(request, 400);
            return (false);
        }

        // Remove \r if present
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Parse chunk size (hexadecimal)
        std::string size_str = trim(line);
        if (size_str.empty())
        {
            setError(request, 400);
            return (false);
        }

        // Convert hex to decimal
        long chunk_size = std::strtol(size_str.c_str(), NULL, 16);
        if (chunk_size < 0)
        {
            setError(request, 400);
            return (false);
        }

        // Zero chunk size means end of body
        if (chunk_size == 0)
        {
            // Read trailing headers (we ignore them for simplicity)
            while (std::getline(input, line))
            {
                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.erase(line.size() - 1);
                if (line.empty())
                    break;
            }
            break;
        }

        // Read chunk data
        std::string chunk_data;
        chunk_data.resize(static_cast<size_t>(chunk_size));
        input.read(&chunk_data[0], static_cast<std::streamsize>(chunk_size));

        if (input.gcount() != static_cast<std::streamsize>(chunk_size))
        {
            setError(request, 400);
            return (false);
        }

        body.append(chunk_data);

        // Read trailing \r\n after chunk data
        std::getline(input, line);  // Should be empty line
    }

    request.setBody(body);
    return (true);
}

// Fixed-length body parsing

bool HttpRequestParser::parseFixedLengthBody(std::istream &input, HttpRequest &request)
{
    long length = request.getContentLength();

    if (length < 0)
    {
        request.setBody("");
        return (true);
    }

    if (length == 0)
    {
        request.setBody("");
        return (true);
    }

    // Read exact number of bytes
    std::string body;
    body.resize(static_cast<size_t>(length));
    input.read(&body[0], static_cast<std::streamsize>(length));

    if (input.gcount() != static_cast<std::streamsize>(length))
    {
        setError(request, 400);
        return (false);
    }

    request.setBody(body);
    return (true);
}

// Header-specific parsing helpers

void HttpRequestParser::parseHostHeader(const std::string &value, HttpRequest &request)
{
    // Host header can be: "example.com" or "example.com:8080"
    std::string	host = trim(value);
    std::string::size_type colon_pos = host.find(':');

    if (colon_pos != std::string::npos)
    {
        // Extract port (we don't store it in HttpRequest for now)
        std::string	port_str = host.substr(colon_pos + 1);
        host = host.substr(0, colon_pos);
    }

    // Host is already added to headers map, no need to store separately
    (void)request;  // Suppress unused parameter warning
}

void	HttpRequestParser::parseConnectionHeader(const std::string &value, HttpRequest &request)
{
    std::string conn = toLower(trim(value));

    if (conn.find("close") != std::string::npos)
        request.setPersistent(false);
    else if (conn.find("keep-alive") != std::string::npos)
        request.setPersistent(true);
	else
	{
		request.setPersistent(false);
		request.setErrorCode(400);
		request.setShouldReject(true);
	}
}

void HttpRequestParser::parseCookieHeader(const std::string &value, HttpRequest &request)
{
    // Parse "name1=value1; name2=value2; name3=value3"
    std::string cookies = trim(value);
    std::istringstream cookie_stream(cookies);
    std::string pair;

    while (std::getline(cookie_stream, pair, ';'))
    {
        pair = trim(pair);
        if (pair.empty())
            continue;

        std::string::size_type eq_pos = pair.find('=');
        if (eq_pos == std::string::npos)
            continue;

        std::string name = trim(pair.substr(0, eq_pos));
        std::string cookie_value = trim(pair.substr(eq_pos + 1));

        if (!name.empty())
            request.addCookie(name, cookie_value);
    }
}

void HttpRequestParser::parseContentLengthHeader(const std::string &value, HttpRequest &request)
{
    std::string length_str = trim(value);
    if (length_str.empty())
        return;

    long length = std::strtol(length_str.c_str(), NULL, 10);
	/*	We cannot set a hard limit because it is supposed to 
		be able to work with large file uploader;
	*/
    request.setContentLength(length);
}

void HttpRequestParser::parseTransferEncodingHeader(const std::string &value, HttpRequest &request)
{
    std::string encoding = toLower(trim(value));
    if (encoding == "chunked")
        request.setChunked(true);
}

// Utility methods

std::string HttpRequestParser::extractHeaderName(const std::string &line)
{
    std::string::size_type colon_pos = line.find(':');
    if (colon_pos == std::string::npos)
        return ("");

    return (trim(line.substr(0, colon_pos)));
}

std::string HttpRequestParser::extractHeaderValue(const std::string &line)
{
    std::string::size_type colon_pos = line.find(':');
    if (colon_pos == std::string::npos)
        return ("");

    if (colon_pos + 1 >= line.size())
        return ("");

    return (trim(line.substr(colon_pos + 1)));
}

bool HttpRequestParser::isValidMethod(const std::string &method)
{
    return (method == "GET" || method == "POST" || method == "DELETE" || method == "PUT");
}

bool HttpRequestParser::isValidHttpVersion(const std::string &version)
{
    return (version == "HTTP/1.1");
}

void HttpRequestParser::setError(HttpRequest &request, int error_code)
{
    request.setShouldReject(true);
    request.setErrorCode(error_code);
}

bool HttpRequestParser::parseUrl
(const std::string &url, std::string &path, std::string &query)
{
	std::string::size_type query_pos = url.find('?');
	std::string	raw_path;

	if (query_pos == std::string::npos)
	{
		raw_path = url;
		query.clear();
	}
	else
	{
		raw_path = url.substr(0, query_pos);
		query = (query_pos + 1 < url.size()) ? url.substr(query_pos + 1) : "";
	}
	if (raw_path.empty())
		raw_path = "/";
	if (!raw_path.empty() && raw_path[0] != '/')
	{
		std::string::size_type	scheme_pos = raw_path.find("://");
		if (scheme_pos != std::string::npos)
		{
			std::string::size_type	path_pos = raw_path.find('/', scheme_pos + 3);
			if (path_pos != std::string::npos)
				raw_path = raw_path.substr(path_pos);
			else
				raw_path = "/";
		}
	}
	std::string	decoded_path;
	if (!urlDecode(raw_path, decoded_path))
		return (false);
	return (sanitizePath(decoded_path, path));
}

bool	HttpRequestParser::urlDecode(const std::string &encoded, std::string &decoded)
{
	decoded.clear();
	decoded.reserve(encoded.size());
	size_t	i = 0;
	while (i < encoded.size())
	{
		unsigned char	c = static_cast<unsigned char>(encoded[i]);
		if (c == '%')
		{
			if (i + 2 >= encoded.size())
				return (false);
			unsigned char	c2 = static_cast<unsigned char>(encoded[i + 1]);
			unsigned char	c3 = static_cast<unsigned char>(encoded[i + 2]);
			if (!std::isxdigit(c2) || !std::isxdigit(c3))
				return (false);
			int	value = (std::isdigit(c2) ? (c2 - '0') : (((std::toupper(c2)) - 'A' + 10) * 16));
			value += (std::isdigit(c3) ? (c3 - '0') : (std::toupper(c3) - 'A' + 10));
			decoded += static_cast<char>(value);
			i += 2;
		}
		else
			decoded += static_cast<char>(c);
		i++;
	}
	return (true);
}

bool	HttpRequestParser::sanitizePath
(const std::string &decoded_path, std::string &sanitized)
{
	if (decoded_path.empty())
	{
		sanitized = "/";
		return (true);
	}
	if (decoded_path[0] != '/')
		return (false);
	std::vector<std::string>	segments;
	size_t						i = 1;
	while (i < decoded_path.size() + 1)
	{
		size_t	k = decoded_path.find('/', i);
		if (k == std::string::npos)
			k = decoded_path.size();
		std::string	segment = decoded_path.substr(i, k - i);
		if (!segment.empty() && segment.find('\0') != std::string::npos)
			return (false);
		if (segment.empty() || segment == ".")
		{}
		else if (segment == "..")
		{
			if (segments.empty())
				return (false);
			segments.pop_back();
		}
		else
			segments.push_back(segment);
		if (k == decoded_path.size())
			break;
		i = k + 1;
	}
	sanitized = "/";
	i = 0;
	while (i < segments.size())
	{
		sanitized += segments[i];
		if (i + 1 < segments.size())
			sanitized += '/';
		i++;
	}
	if (!segments.empty() && decoded_path[decoded_path.size() - 1] == '/')
		sanitized += '/';
	return (true);
}

// Request completeness helpers

bool HttpRequestParser::hasCompleteChunkedBody(const std::string &body)
{
	size_t	pos = 0;

	while (true)
	{
		size_t	line_end = body.find("\r\n", pos);
		if (line_end == std::string::npos)
			return (false);

		std::string	size_line = body.substr(pos, line_end - pos);
		std::string::size_type	semicolon = size_line.find(';');
		if (semicolon != std::string::npos)
			size_line = size_line.substr(0, semicolon);

		std::string	size_str = trim(size_line);
		if (size_str.empty())
			return (true);

		long	chunk_size = std::strtol(size_str.c_str(), NULL, 16);
		if (chunk_size < 0)
			return (true);

		pos = line_end + 2;

		if (chunk_size > static_cast<long>(body.size() - pos))
			return (false);

		pos += static_cast<size_t>(chunk_size);

		if (pos + 1 >= body.size())
			return (false);

		if (body[pos] != '\r' || body[pos + 1] != '\n')
			return (true);

		pos += 2;

		if (chunk_size == 0)
		{
			if (pos >= body.size())
				return (true);
			while (true)
			{
				size_t	trailer_end = body.find("\r\n", pos);
				if (trailer_end == std::string::npos)
					return (false);
				if (trailer_end == pos)
					return (true);
				pos = trailer_end + 2;
			}
		}
	}

	return (false);
}

bool HttpRequestParser::hasCompleteFixedLengthBody(const std::string &body, long length)
{
	if (length <= 0)
		return (true);
	return (static_cast<long>(body.size()) >= length);
}

// Public utility methods

bool HttpRequestParser::isCompleteRequest(const std::string &input)
{
	std::string::size_type	header_end = input.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (false);

	std::string	headers = input.substr(0, header_end);
	std::istringstream	stream(headers);
	std::string	line;

	if (!std::getline(stream, line))
		return (false);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	bool	chunked = false;
	long	content_length = -1;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		std::string::size_type	colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			continue;

		std::string	name = toLower(trim(line.substr(0, colon_pos)));
		std::string	value = trim(line.substr(colon_pos + 1));

		if (name == "transfer-encoding")
		{
			std::string	lower_value = toLower(value);
			if (lower_value.find("chunked") != std::string::npos)
			{
				chunked = true;
				content_length = -1;
			}
		}
		else if (name == "content-length" && !chunked)
		{
			long	parsed = std::strtol(value.c_str(), NULL, 10);
			if (parsed >= 0)
				content_length = parsed;
		}
	}

	std::string	body = input.substr(header_end + 4);

	if (chunked)
		return (hasCompleteChunkedBody(body));
	if (content_length >= 0)
		return (hasCompleteFixedLengthBody(body, content_length));
	return (true);
}

bool HttpRequestParser::hasValidRequestLine(const std::string &input)
{
    std::string::size_type first_line_end = input.find("\r\n");
    if (first_line_end == std::string::npos)
        return (false);

    std::string first_line = input.substr(0, first_line_end);
    std::istringstream iss(first_line);
    std::string method, url, version;

    return (iss >> method >> url >> version &&
            isValidMethod(method) &&
            isValidHttpVersion(version));
}
