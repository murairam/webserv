/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 01:24:56 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:03:52 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "timestring.hpp"
#include "utility.hpp"
#include "Directory.hpp"

Response::Response(void)
:_status_code(200), _headers(), _body(), _code_page()
{
	setHeader("Server", "webserv/1.0");
	setHeader("Date", getTimeString());
	setHeader("Connection", "keep-alive");
}

Response::Response(int status_code)
:_status_code(status_code), _headers(), _body(), _code_page()
{
	setHeader("Server", "webserv/1.0");
	setHeader("Date", getTimeString());
	setHeader("Connection", "keep-alive");
}

Response::Response(const Response &other)
:_status_code(other._status_code), _headers(other._headers),
_body(other._body), _code_page() {}  // ✅ Default construct CodePage

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_status_code = other._status_code;
		_headers = other._headers;
		_body = other._body;
		// Don't assign _code_page - it's stateless
	}
	return (*this);
}

Response::~Response(void) {}

void	Response::setStatus(int code)
{
	if (code < 100 || code > 599)
		_status_code = 500;
	else
		_status_code = code;
}

void	Response::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void	Response::setBody(const std::string &body)
{
	_body = body;
	setHeader("Content-Length", intToString(_body.size()));
}

void	Response::setBodyFromFile(const std::string &filepath)
{
	std::ifstream	file(filepath.c_str(), std::ios::binary);
	std::string		content;

	if (!file.is_open())
		throw SysError("Cannot open file: " + filepath, errno);

	file.seekg(0, std::ios::end);
	std::streamsize	size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size > 0)
	{
		content.reserve(static_cast<size_t>(size));
		content.assign(std::istreambuf_iterator<char>(file),
					   std::istreambuf_iterator<char>());
	}
	file.close();
	setBody(content);

	std::string	extension = getFileExtension(filepath);
	std::string	mime_type = getMimeType(extension);
	if (!mime_type.empty())
		setHeader("Content-Type", mime_type);
}

std::string	Response::serialize(void) const
{
	std::string	response;
	std::string	reason = _code_page.getReason(_status_code);

	response = "HTTP/1.1 " + intToString(_status_code) + " " + reason + "\r\n";
	std::map<std::string, std::string>::const_iterator	it = _headers.begin();
	while (it != _headers.end())
	{
		response += it->first + ": " + it->second + "\r\n";
		++it;
	}
	response += "\r\n";
	response += _body;
	return (response);
}

Response	Response::createErrorResponse(int code, const std::string &error_page_path)
{
	Response	resp(code);

	if (!error_page_path.empty())
	{
		try
		{
			resp.setBodyFromFile(error_page_path);
			return (resp);
		}
		catch (const std::exception &)
		{
			/* Fall back to generated error page */
		}
	}
	resp.setBody(resp._code_page.getCodePage(code));
	resp.setHeader("Content-Type", "text/html");
	return (resp);
}

Response	Response::createFileResponse(const std::string &filepath)
{
	try
	{
		Response	resp(200);
		resp.setBodyFromFile(filepath);
		return (resp);
	}
	catch (const std::exception &)
	{
		return (createErrorResponse(404));
	}
}

Response	Response::createRedirectResponse(int code, const std::string &location)
{
	Response	resp(code);

	resp.setHeader("Location", location);
	resp.setBody("");
	return (resp);
}

Response	Response::createDirectoryListing(const std::string &path, const std::string &uri)
{
	try
	{
		Response	resp(200);
		std::string	html = resp.generateDirectoryListingHTML(path, uri);

		resp.setBody(html);
		resp.setHeader("Content-Type", "text/html");
		return (resp);
	}
	catch (const std::exception &)
	{
		return (createErrorResponse(403));
	}
}

std::string	Response::generateDirectoryListingHTML(const std::string &path, const std::string &uri) const
{
	std::string		html;
	Directory		dir(path);
	std::string		entry;

	html = "<!DOCTYPE html>\n<html><head><title>Index of " + uri + "</title></head>\n";
	html += "<body><h1>Index of " + uri + "</h1><hr><pre>\n";

	dir.ft_opendir();
	while (!(entry = dir.nextEntry()).empty())
	{
		std::string	full_path = path + "/" + entry;
		if (isDirectory(full_path))
			html += "<a href=\"" + entry + "/\">" + entry + "/</a>\n";
		else
			html += "<a href=\"" + entry + "\">" + entry + "</a>\n";
	}
	html += "</pre><hr></body></html>";
	return (html);
}

std::string	Response::intToString(size_t value) const
{
	std::ostringstream	oss;

	oss << value;
	return (oss.str());
}

std::string	Response::getFileExtension(const std::string &filepath) const
{
	std::string::size_type	pos = filepath.find_last_of('.');

	if (pos != std::string::npos && pos < filepath.length() - 1)
		return (filepath.substr(pos));
	return ("");
}

std::string	Response::getMimeType(const std::string &extension) const
{
	if (extension == ".html" || extension == ".htm")
		return ("text/html");
	else if (extension == ".css")
		return ("text/css");
	else if (extension == ".js")
		return ("application/javascript");
	else if (extension == ".png")
		return ("image/png");
	else if (extension == ".jpg" || extension == ".jpeg")
		return ("image/jpeg");
	else if (extension == ".gif")
		return ("image/gif");
	else if (extension == ".txt")
		return ("text/plain");
	else if (extension == ".ico")
		return ("image/x-icon");
	else
		return ("application/octet-stream");
}

#ifdef _DEBUG
void	Response::debug(void) const
{
	std::cout << "=== Response Debug Info ===" << std::endl;
	std::cout << "Status Code: " << _status_code << std::endl;
	std::cout << "Headers (" << _headers.size() << "):" << std::endl;

	std::map<std::string, std::string>::const_iterator it = _headers.begin();
	while (it != _headers.end())
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
		++it;
	}

	std::cout << "Body length: " << _body.length() << std::endl;
	if (_body.length() > 0 && _body.length() <= 100)
		std::cout << "Body preview: " << _body << std::endl;
	else if (_body.length() > 100)
		std::cout << "Body preview: " << _body.substr(0, 100) << "..." << std::endl;
	std::cout << "===========================" << std::endl;
}
#endif
