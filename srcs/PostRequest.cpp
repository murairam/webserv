/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:48 by yanli             #+#    #+#             */
/*   Updated: 2025/09/21 21:56:22 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostRequest.hpp"

void	PostRequest::process(std::istream &s)
{
	std::istringstream	iss;
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	bool				toggle = true;
	
	bool				header_ended = false;

	while (std::getline(s, line))
	{
		iss.clear();
		iss.str(line);
		keyword.clear();
		valueword.clear();
		if (!line.empty() && line[0] == '\r')
		{
			header_ended = true;
			break ;
		}
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<"'"<<keyword<<"'"<<std::endl;
#endif
		if (toggle)
		{
			toggle = false;
			if (_should_reject)
				return ;
			if (keyword.empty())
				goto reject_400;
			std::string::size_type	query_mark_pos = keyword.find('?');
			if (query_mark_pos != std::string::npos)
			{
				std::string	target = keyword.substr(0, query_mark_pos);
				if (query_mark_pos + 1 >= keyword.size())
				{
					_target = target;
					_target_set = true;
					continue;
				}
				std::string	query = keyword.substr(query_mark_pos + 1, keyword.size());
				_target = target;
				_target_set = true;
				_query = query;
				_query_set = true;
			}
			else
			{
				std::string	target = keyword;
				_target = target;
				_target_set = true;
			}
			iss>>std::ws;
			std::getline(iss, valueword, '\r');
			valueword = trim(valueword);
			if (valueword != "HTTP/1.1")
				goto reject_505;
		}
		else if (keyword == "Connection:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			std::string	lowered = toLower(valueword);
			if (lowered.empty())
				goto reject_400;
			std::istringstream	conn_stream(lowered);
			std::string			conn_token;
			while (std::getline(conn_stream, conn_token, ','))
			{
				conn_token = trim(conn_token);
				if (conn_token == "close")
					_persistent = false;
				else if (conn_token == "keep-alive")
					continue;
			}
		}
		else if (keyword == "Host:")
		{
			if (_host_set)
				goto reject_400;
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty())
				goto reject_400;
			std::string	host;
			std::string	port_candidate;
			bool		port_present = false;
			if (valueword[0] == '[')
				goto reject_505;
			else
			{
				std::string::size_type	colon_mark_pos = valueword.find(':');
				if (colon_mark_pos != std::string::npos)
				{
					host = valueword.substr(0, colon_mark_pos);
					port_candidate = trim(valueword.substr(colon_mark_pos + 1));
					port_present = true;
				}
				else
					host = valueword;
			}
			host = trim(host);
			if (host.empty())
				goto reject_400;
			if (port_present)
			{
				if (port_candidate.empty())
					goto reject_400;
				if (port_candidate.find_first_not_of("0123456789") != std::string::npos)
					goto reject_400;
				long	portvalue = std::strtol(port_candidate.c_str(), NULL, 10);
				if (portvalue < 0 || portvalue > 65535)
					goto reject_400;
				_port = static_cast<int>(portvalue);
				_port_set = true;
			}
			_host = host;
			_host_set = true;
		}
		else if (keyword == "Authorization:")
		{
			std::string	auth_type;
			std::string	auth_value;
			auth_type.clear();
			auth_value.clear();
			iss>>auth_type;
			iss>>std::ws;
			if (!std::getline(iss, auth_value, '\r') || auth_type.empty())
				goto reject_400;
			auth_value = trim(auth_value);
			if (auth_value.empty())
				goto reject_400;
			_auth[auth_type] = auth_value;
			_auth_set = true;
		}
		else if (keyword == "Cookie:")
		{
			std::string	cookie_line;
			iss>>std::ws;
			if (!std::getline(iss, cookie_line, '\r'))
				goto reject_400;
			cookie_line = trim(cookie_line);
			if (cookie_line.empty())
				continue;
			std::istringstream	cookie_stream(cookie_line);
			std::string	cookie_pair;
			while (std::getline(cookie_stream, cookie_pair, ';'))
			{
				cookie_pair = trim(cookie_pair);
				if (cookie_pair.empty())
					continue;
				std::string::size_type	equal_mark_pos = cookie_pair.find('=');
				if (equal_mark_pos == std::string::npos)
					continue;
				std::string	c1 = trim(cookie_pair.substr(0, equal_mark_pos));
				std::string	c2 = trim(cookie_pair.substr(equal_mark_pos + 1));
				if (c1.empty())
					continue;
				_cookie[c1] = c2;
				_cookie_set = true;
			}
			#ifdef	_DEBUG
			std::map<std::string,std::string>::const_iterator	it = _cookie.begin();
			while (it != _cookie.end())
			{
				std::cout<<it->first<<"\n"<<it->second<<std::endl;
				it++;
			}
			#endif
		}
		else if (keyword == "Transfer-Encoding:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r') || valueword != "chunked")
				goto reject_501;
			_chunked = true;
		}
		else if (keyword == "Content-Length:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
				goto reject_400;
			valueword = trim(valueword);
			if (valueword.empty() || valueword.find_first_not_of("0123456789") != std::string::npos)
				goto reject_400;
			long	len = std::atol(valueword.c_str());
			_body_length = len;
			_body_length_set = true;
		}
		else
			continue;
	}
	if (!header_ended)
			goto reject_400;
	if (!_target_set || !_host_set)
			goto reject_400;
	if (_chunked && _body_length_set)
			goto reject_501;
	if (_chunked)
	{
		std::string	body_buffer;
		std::string	chunk_line;
		bool		saw_chunk_header = false;
		while (true)
		{
			if (!std::getline(s, chunk_line))
			{
				if (!saw_chunk_header && body_buffer.empty() && s.eof())
				{
					_body.clear();
					_body_set = true;
					return ;
				}
					goto reject_400;
			}
			saw_chunk_header = true;
			if (!chunk_line.empty() && chunk_line[chunk_line.size() - 1] == '\r')
				chunk_line.erase(chunk_line.size() - 1);
			chunk_line = trim(chunk_line);
			if (chunk_line.empty())
				goto reject_400;
			std::string::size_type	semicolon_pos = chunk_line.find(';');
			if (semicolon_pos != std::string::npos)
				chunk_line = trim(chunk_line.substr(0, semicolon_pos));
			if (chunk_line.empty() || chunk_line.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos)
				goto reject_400;
			long	chunk_size = std::strtol(chunk_line.c_str(), NULL, 16);
			if (chunk_size < 0)
				goto reject_400;
			if (chunk_size == 0)
			{
				while (true)
				{
					std::string	trailer_line;
					if (!std::getline(s, trailer_line))
						goto reject_400;
					if (!trailer_line.empty() && trailer_line[trailer_line.size() - 1] == '\r')
						trailer_line.erase(trailer_line.size() - 1);
					if (trim(trailer_line).empty())
						break ;
				}
				break ;
			}
			std::string	chunk_buffer;
			try
			{
				chunk_buffer.resize(static_cast<std::size_t>(chunk_size));
			}
			catch (const std::exception &)
			{
				goto reject_413;
			}
			s.read(&chunk_buffer[0], static_cast<std::streamsize>(chunk_size));
			if (s.gcount() != static_cast<std::streamsize>(chunk_size))
				goto reject_400;
			body_buffer.append(chunk_buffer);
			int	cr = s.get();
			int	lf = s.get();
			if (cr == std::char_traits<char>::eof() || lf == std::char_traits<char>::eof()
				|| cr != '\r' || lf != '\n')
				goto reject_400;
		}
		_body = body_buffer;
		_body_set = true;
	}
	else if (_body_length_set)
	{
		long	length = _body_length;
		if (length < 0)
				goto reject_400;
		std::string	body_buffer;
		if (length > 0)
		{
			try
			{
				body_buffer.resize(static_cast<std::size_t>(length));
			}
			catch (const std::exception &)
			{
				goto reject_413;
			}
			s.read(&body_buffer[0], static_cast<std::streamsize>(length));
			if (s.gcount() != static_cast<std::streamsize>(length))
				goto reject_400;
		}
		_body = body_buffer;
		_body_set = true;
	}
	else
	{
		_body.clear();
		_body_set = true;
	}
	if (!_target_set || !_host_set)
		goto reject_400;
	if (_chunked && _body_length_set)
		goto reject_501;
	return;

	reject_400:
	{
		_err_code = 400;
		_err_code_set = true;
		_should_reject = true;
		return;
	}
	reject_413:
	{
		_err_code = 413;
		_err_code_set = true;
		_should_reject = true;
		return;
	}
	reject_501:
	{
		_err_code = 501;
		_err_code_set = true;
		_should_reject = true;
		return;
	}
	reject_505:
	{
		_err_code = 505;
		_err_code_set = true;
		_should_reject = true;
		return;
	}
}


PostRequest::PostRequest(void)
: _target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false),_persistent(true), _chunked(false),
_body_length(0), _body_length_set(false), _err_code(0), _err_code_set(false),
_body(""), _body_set(false)
{}

PostRequest::PostRequest(std::istream &s)
:_target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false), _persistent(true), _chunked(false),
_body_length(0), _body_length_set(false), _err_code(0), _err_code_set(false),
_body(""), _body_set(false)
{
	this->process(s);
}

PostRequest::PostRequest(const PostRequest &other)
:_target(other._target), _target_set(other._target_set), _query(other._query),
_query_set(other._query_set), _host(other._host), _host_set(other._host_set),
_port(other._port), _port_set(other._port_set), _auth(other._auth),
_auth_set(other._auth_set), _cookie(other._cookie), _cookie_set(other._cookie_set),
_should_reject(other._should_reject), _persistent(other._persistent),
_chunked(other._chunked), _body_length(other._body_length),
_body_length_set(other._body_length_set), _err_code(other._err_code),
_err_code_set(other._err_code_set), _body(other._body),
_body_set(other._body_set)
{}

PostRequest	&PostRequest::operator=(const PostRequest &other)
{
	if (this != &other)
	{
		_target = other._target;
		_target_set = other._target_set;
		_query = other._query;
		_query_set = other._query_set;
		_host = other._host;
		_host_set = other._host_set;
		_port = other._port;
		_port_set = other._port_set;
		_auth = other._auth;
		_auth_set = other._auth_set;
		_cookie = other._cookie;
		_cookie_set = other._cookie_set;
		_should_reject = other._should_reject;
		_persistent = other._persistent;
		_chunked = other._chunked;
		_body_length = other._body_length;
		_body_length_set = other._body_length_set;
		_err_code = other._err_code;
		_err_code_set = other._err_code_set;
		_body = other._body;
		_body_set = other._body_set;
	}
	return (*this);
}

PostRequest::~PostRequest(void) {}

bool	PostRequest::isErrCodeSet(void) const
{
	return (_err_code_set);
}

int	PostRequest::getErrCode(void) const
{
	return (_err_code);
}

long	PostRequest::getBodyLength(void) const
{
	return (_body_length);
}

bool	PostRequest::isBodyLengthSet(void) const
{
	return (_body_length_set);
}

bool	PostRequest::isChunked(void) const
{
	return (_chunked);
}

bool	PostRequest::isPersistent(void) const
{
	return (_persistent);
}

bool	PostRequest::isTargetSet(void) const
{
	return (_target_set);
}

bool	PostRequest::isQuerySet(void) const
{
	return (_query_set);
}

bool	PostRequest::isHostSet(void) const
{
	return (_host_set);
}

bool	PostRequest::isPortSet(void) const
{
	return (_port_set);
}

bool	PostRequest::isAuthSet(void) const
{
	return (_auth_set);
}

bool	PostRequest::isCookieSet(void) const
{
	return (_cookie_set);
}

bool	PostRequest::shouldReject(void) const
{
	return (_should_reject);
}

std::string	PostRequest::getTarget(void) const
{
	return (_target);
}

std::string	PostRequest::getQuery(void) const
{
	return (_query);
}

std::string	PostRequest::getHost(void) const
{
	return (_host);
}

int	PostRequest::getPort(void) const
{
	return (_port);
}

std::map<std::string,std::string>	PostRequest::getAuth(void) const
{
	return (_auth);
}

std::map<std::string,std::string>	PostRequest::getCookie(void) const
{
	return (_cookie);
}

bool	PostRequest::isBodySet(void) const
{
	return (_body_set);
}

std::string	PostRequest::getBody(void) const
{
	return (_body);
}
