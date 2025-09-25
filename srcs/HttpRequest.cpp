/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:19:01 by mmiilpal          #+#    #+#             */
/*   Updated: 2025/09/25 11:19:03 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "utility.hpp"

// Orthodox Canonical Form

HttpRequest::HttpRequest(void)
    : _method(""), _path(""), _query(""), _version("HTTP/1.1"),
      _headers(), _cookies(), _body(""),
      _persistent(true), _chunked(false), _content_length(-1),
      _should_reject(false), _error_code(0)
{
}

HttpRequest::HttpRequest(const HttpRequest &other)
    : _method(other._method), _path(other._path), _query(other._query),
      _version(other._version), _headers(other._headers), _cookies(other._cookies),
      _body(other._body), _persistent(other._persistent), _chunked(other._chunked),
      _content_length(other._content_length), _should_reject(other._should_reject),
      _error_code(other._error_code)
{
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
    if (this != &other)
    {
        _method = other._method;
        _path = other._path;
        _query = other._query;
        _version = other._version;
        _headers = other._headers;
        _cookies = other._cookies;
        _body = other._body;
        _persistent = other._persistent;
        _chunked = other._chunked;
        _content_length = other._content_length;
        _should_reject = other._should_reject;
        _error_code = other._error_code;
    }
    return (*this);
}

HttpRequest::~HttpRequest(void)
{
    // STL containers clean themselves up
}

// Getters

const std::string &HttpRequest::getMethod(void) const
{
    return (_method);
}

const std::string &HttpRequest::getPath(void) const
{
    return (_path);
}

const std::string &HttpRequest::getQuery(void) const
{
    return (_query);
}

const std::string &HttpRequest::getVersion(void) const
{
    return (_version);
}

const std::string &HttpRequest::getBody(void) const
{
    return (_body);
}

bool HttpRequest::getPersistent(void) const
{
    return (_persistent);
}

bool HttpRequest::getChunked(void) const
{
    return (_chunked);
}

long HttpRequest::getContentLength(void) const
{
    return (_content_length);
}

bool HttpRequest::getShouldReject(void) const
{
    return (_should_reject);
}

int HttpRequest::getErrorCode(void) const
{
    return (_error_code);
}

// Setters

void HttpRequest::setMethod(const std::string &method)
{
    _method = method;
}

void HttpRequest::setPath(const std::string &path)
{
    _path = path;
}

void HttpRequest::setQuery(const std::string &query)
{
    _query = query;
}

void HttpRequest::setVersion(const std::string &version)
{
    _version = version;
}

void HttpRequest::setBody(const std::string &body)
{
    _body = body;
}

void HttpRequest::setPersistent(bool persistent)
{
    _persistent = persistent;
}

void HttpRequest::setChunked(bool chunked)
{
    _chunked = chunked;
}

void HttpRequest::setContentLength(long length)
{
    _content_length = length;
}

void HttpRequest::setShouldReject(bool reject)
{
    _should_reject = reject;
}

void HttpRequest::setErrorCode(int code)
{
    _error_code = code;
}

// Header and cookie management

void HttpRequest::addHeader(const std::string &key, const std::string &value)
{
    std::string normalized_key = toLower(trim(key));
    if (!normalized_key.empty())
        _headers[normalized_key] = trim(value);
}

void HttpRequest::addCookie(const std::string &key, const std::string &value)
{
    std::string clean_key = trim(key);
    if (!clean_key.empty())
        _cookies[clean_key] = trim(value);
}

std::string HttpRequest::getHeader(const std::string &name) const
{
    std::string normalized = toLower(trim(name));
    std::map<std::string, std::string>::const_iterator it = _headers.find(normalized);
    if (it != _headers.end())
        return (it->second);
    return ("");
}

std::string HttpRequest::getCookie(const std::string &name) const
{
    std::string clean_name = trim(name);
    std::map<std::string, std::string>::const_iterator it = _cookies.find(clean_name);
    if (it != _cookies.end())
        return (it->second);
    return ("");
}

bool HttpRequest::hasHeader(const std::string &name) const
{
    std::string normalized = toLower(trim(name));
    return (_headers.find(normalized) != _headers.end());
}

// Future CGI support
std::map<std::string, std::string> HttpRequest::toCgiEnvironment(void) const
{
    std::map<std::string, std::string> env;

    // Standard CGI environment variables
    env["REQUEST_METHOD"] = _method;
    env["PATH_INFO"] = _path;
    if (!_query.empty())
        env["QUERY_STRING"] = _query;
    if (_content_length >= 0)
    {
        std::ostringstream oss;
        oss << _content_length;
        env["CONTENT_LENGTH"] = oss.str();
    }

    // Convert HTTP headers to CGI format
    std::map<std::string, std::string>::const_iterator it = _headers.begin();
    while (it != _headers.end())
    {
        std::string cgi_name = "HTTP_" + it->first;
        // Convert to uppercase and replace - with _
        std::string::iterator str_it = cgi_name.begin();
        while (str_it != cgi_name.end())
        {
            if (*str_it == '-')
                *str_it = '_';
            else
                *str_it = std::toupper(*str_it);
            ++str_it;
        }
        env[cgi_name] = it->second;
        ++it;
    }

    return (env);
}

#ifdef _DEBUG
void HttpRequest::debug(void) const
{
    std::cout << "=== HttpRequest Debug ===" << std::endl;
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "Query: " << _query << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "Persistent: " << (_persistent ? "true" : "false") << std::endl;
    std::cout << "Chunked: " << (_chunked ? "true" : "false") << std::endl;
    std::cout << "Content-Length: " << _content_length << std::endl;
    std::cout << "Should reject: " << (_should_reject ? "true" : "false") << std::endl;
    std::cout << "Error code: " << _error_code << std::endl;

    std::cout << "Headers (" << _headers.size() << "):" << std::endl;
    std::map<std::string, std::string>::const_iterator it = _headers.begin();
    while (it != _headers.end())
    {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
        ++it;
    }

    std::cout << "Cookies (" << _cookies.size() << "):" << std::endl;
    it = _cookies.begin();
    while (it != _cookies.end())
    {
        std::cout << "  " << it->first << "=" << it->second << std::endl;
        ++it;
    }

    std::cout << "Body length: " << _body.length() << std::endl;
    std::cout << "=========================" << std::endl;
}
#endif
