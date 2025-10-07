/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CodePage.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 17:49:25 by yanli             #+#    #+#             */
/*   Updated: 2025/10/03 22:44:43 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CodePage.hpp"

std::string	CodePage::buildPage(int code) const
{
	std::string	reason = CodePage::getReason(code);
	std::string	title = CodePage::int_to_string(code) + " " + reason;
	std::string	html;

	html.reserve(_template_prefix.size() + _template_middle.size()
	+ _template_suffix.size() + title.size() * 2);
	html = _template_prefix + title + _template_middle + title + _template_suffix;
	return (html);
}

std::string	CodePage::to_upper(const std::string &s)
{
	std::string	ret;
	std::string::size_type	i = 0;
	char	c = 0;

	ret.reserve(s.size());
	while (i < s.size())
	{
		c = s[i];
		if (c >= 'a' && c <= 'z')
			ret.push_back(static_cast<char>((c - 32)));
		else
			ret.push_back(c);
		i++;
	}
	return (ret);
}

std::string	CodePage::int_to_string(int code)
{
	std::ostringstream	oss;
	oss<<code;
	return (oss.str());
}

CodePage::CodePage(void)
:_template_prefix("<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\">\n"
"    <title>"),
_template_middle("</title>\n"
"    <style>\n"
"        body {\n"
"            background-color: white;\n"
"            text-align: center;\n"
"            margin-top: 20%;\n"
"            font-family: Arial, sans-serif;\n"
"        }\n"
"        h1 {\n"
"            color: skyblue;\n"
"        }\n"
"        p {\n"
"            color: gray;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <h1>"
),
_template_suffix("</h1>\n"
"    <p>Yang and Mari Webserver Default Response Page</p>\n"
"</body>\n"
"</html>\n"
)
{}

CodePage::CodePage(const CodePage &other)
:_template_prefix(other._template_prefix),
_template_middle(other._template_middle),
_template_suffix(other._template_suffix) {(void)other;}

CodePage	&CodePage::operator=(const CodePage &other)
{
	(void)other;
	return (*this);
}

CodePage::~CodePage(void) {}

const std::map<int,std::string>	&CodePage::codes(void)
{
	static std::map<int,std::string>	m;

	if (m.empty())
	{
		m[100] = "Continue";
		m[200] = "OK";
		m[201] = "Created";
		m[202] = "Accepted";
		m[204] = "No Content";
		m[301] = "Moved Permanently";
		m[302] = "Found";
		m[308] = "Permanent Redirect";
		m[400] = "Bad Request";
		m[401] = "Unauthorized";
		m[403] = "Forbidden";
		m[404] = "Not Found";
		m[405] = "Method Not Allowed";
		m[406] = "Not Acceptable";
		m[408] = "Request Timeout";
		m[409] = "Conflict";
		m[411] = "Length Required";
		m[412] = "Precondition Failed";
		m[413] = "Payload Too Large";
		m[421] = "Misdirected Request";
		m[431] = "Request Header Fields Too Large";
		m[500] = "Internal Server Error";
		m[501] = "Not Implemented";
		m[505] = "HTTP Version Not Supported";
	}
	return (m);
}

CodePage	&CodePage::getInstance(void)
{
	static CodePage	instance;
	return (instance);
}

std::string	CodePage::getCodePage(int code) const
{
	return (buildPage(code));
}
/*
std::string	CodePage::getCodePage(int code, std::string s) const
{
	return (buildPage(code, s));
}
*/

std::string	CodePage::getReason(int code) const
{
	std::map<int,std::string>::const_iterator	it = codes().find(code);
	if (it != codes().end())
		return (it->second);
	return ("Unsupported code");
}

namespace
{
	struct CodePageInit
	{
		CodePageInit(void){ (void)CodePage::getInstance();}
		~CodePageInit(void) {}
	};

	CodePageInit	g_codepage_init;
}
