/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:35:23 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/09/25 11:35:25 by mmiilpal         ###   ########.fr       */
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

    // Parse body (for POST requests)
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

    // Parse URL into path and query
    std::string path, query;
    parseUrl(url, path, query);

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
        request.addHeader(name, value);

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
        return (parseFixedLengthBody(input, request));

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
    std::string host = trim(value);
    std::string::size_type colon_pos = host.find(':');

    if (colon_pos != std::string::npos)
    {
        // Extract port (we don't store it in HttpRequest for now)
        std::string port_str = host.substr(colon_pos + 1);
        host = host.substr(0, colon_pos);
    }

    // Host is already added to headers map, no need to store separately
    (void)request;  // Suppress unused parameter warning
}

void HttpRequestParser::parseConnectionHeader(const std::string &value, HttpRequest &request)
{
    std::string conn = toLower(trim(value));

    if (conn.find("close") != std::string::npos)
        request.setPersistent(false);
    else if (conn.find("keep-alive") != std::string::npos)
        request.setPersistent(true);
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
    return (method == "GET" || method == "POST" || method == "DELETE" ||
            method == "HEAD" || method == "PUT" || method == "OPTIONS");
}

bool HttpRequestParser::isValidHttpVersion(const std::string &version)
{
    return (version == "HTTP/1.1" || version == "HTTP/1.0");
}

void HttpRequestParser::setError(HttpRequest &request, int error_code)
{
    request.setShouldReject(true);
    request.setErrorCode(error_code);
}

void HttpRequestParser::parseUrl(const std::string &url, std::string &path, std::string &query)
{
    std::string::size_type query_pos = url.find('?');

    if (query_pos == std::string::npos)
    {
        path = url;
        query = "";
    }
    else
    {
        path = url.substr(0, query_pos);
        query = (query_pos + 1 < url.size()) ? url.substr(query_pos + 1) : "";
    }
}

// Public utility methods

bool HttpRequestParser::isCompleteRequest(const std::string &input)
{
    // Simple check: must contain \r\n\r\n (end of headers)
    return (input.find("\r\n\r\n") != std::string::npos);
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
