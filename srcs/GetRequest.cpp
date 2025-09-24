/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:12 by yanli             #+#    #+#             */
/*   Updated: 2025/09/21 21:53:06 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetRequest.hpp"

void	GetRequest::process(std::istream &s)
{
	std::istringstream	iss;
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	bool				toggle = true;
	
	while (std::getline(s, line))
	{
		iss.clear();
		iss.str(line);
		keyword.clear();
		valueword.clear();
		if (!line.empty() && line[0] == '\r')
			return ;
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<"'"<<keyword<<"'"<<std::endl;
#endif
		if (toggle)
		{
			toggle = false;
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
				continue;
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
			_body_length_set = true;
		else
			continue;
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

GetRequest::GetRequest(void)
: _target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false),_persistent(true), _chunked(false),
_body_length_set(false), _err_code(0), _err_code_set(false)
{}

GetRequest::GetRequest(std::istream &s)
:_target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false), _persistent(true), _chunked(false),
_body_length_set(false), _err_code(0), _err_code_set(false)
{
	this->process(s);
}

GetRequest::GetRequest(const GetRequest &other)
:_target(other._target), _target_set(other._target_set), _query(other._query),
_query_set(other._query_set), _host(other._host), _host_set(other._host_set),
_port(other._port), _port_set(other._port_set), _auth(other._auth),
_auth_set(other._auth_set), _cookie(other._cookie), _cookie_set(other._cookie_set),
_should_reject(other._should_reject), _persistent(other._persistent),
_chunked(other._chunked),
_body_length_set(other._body_length_set), _err_code(other._err_code),
_err_code_set(other._err_code_set)
{}

GetRequest	&GetRequest::operator=(const GetRequest &other)
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
		_body_length_set = other._body_length_set;
		_err_code = other._err_code;
		_err_code_set = other._err_code_set;
		
	}
	return (*this);
}

GetRequest::~GetRequest(void) {}



bool	GetRequest::isErrCodeSet(void) const
{
	return (_err_code_set);
}

int	GetRequest::getErrCode(void) const
{
	if (this->isErrCodeSet())
		return (this->_err_code);
	return (400);
}

bool	GetRequest::isBodyLengthSet(void) const
{
	return (_body_length_set);
}

bool	GetRequest::isChunked(void) const
{
	return (_chunked);
}

bool	GetRequest::isPersistent(void) const
{
	return (_persistent);
}

bool	GetRequest::isTargetSet(void) const
{
	return (_target_set);
}

bool	GetRequest::isQuerySet(void) const
{
	return (_query_set);
}

bool	GetRequest::isHostSet(void) const
{
	return (_host_set);
}

bool	GetRequest::isPortSet(void) const
{
	return (_port_set);
}

bool	GetRequest::isAuthSet(void) const
{
	return (_auth_set);
}

bool	GetRequest::isCookieSet(void) const
{
	return (_cookie_set);
}

bool	GetRequest::shouldReject(void) const
{
	return (_should_reject);
}

std::string	GetRequest::getTarget(void) const
{
	return (_target);
}

std::string	GetRequest::getQuery(void) const
{
	return (_query);
}

std::string	GetRequest::getHost(void) const
{
	return (_host);
}

int	GetRequest::getPort(void) const
{
	return (_port);
}

std::map<std::string,std::string>	GetRequest::getAuth(void) const
{
	return (_auth);
}

std::map<std::string,std::string>	GetRequest::getCookie(void) const
{
	return (_cookie);
}
