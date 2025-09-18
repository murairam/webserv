/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:12 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 23:14:36 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetRequest.hpp"
/*
bool	GetRequest::containInvalidSection(const std::string &s) const
{
#ifdef	_DEBUG
	std::cout<<"GetRequest::containInvalidSection debug info:\n'"<<s<<"'"<<std::endl;
#endif
	if (s == "Host" || s == "User-Agent" || s == "Accept" || s == "Accept-Language"
		|| s == "Agent-Encoding" || s == "Connection" || s == "If-None-Match"
		|| s == "If-Modified-Since" || s == "Range" || s == "If-Range"
		|| s == "Authorization" || s == "Cookie" || s == "Cache-Control"
		|| s == "Pragma" || s == "Upgrade-Insecure-Requests" || s == "Referer"
		|| s == "Origin" || s == "Upgrade" || s == "Sec-WebSocket-Version"
		|| s == "Sec-WebSocket-Key" || s == "X-Requested-With" || s == "X-Forwarded-For"
		|| s == "Via" || s == "TE")
		return (false);
	return (true);
}

bool	GetRequest::containInvalidLineEnd(const std::string &s) const
{
	if (s.size() < 1 || s[s.size() - 1] != '\r')
		return (true);
	return (false);
}*/

void	GetRequest::process(const std::string &s)
{
	std::istringstream	file(s);
	std::istringstream	iss;
	std::string			line;
	std::string			keyword;
	std::string			valueword;
	
	while (std::getline(file, line))
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
		if (keyword == "GET")
		{
			if (_should_reject)
				return ;
			keyword.clear();
			iss>>keyword;
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
			if (valueword.size() < 8 || valueword[5] != '1' || valueword[6] !='.' || valueword[7] != '1')
			{
				_err_code = 501;
				_err_code_set = true;
				goto reject;
			}
		}
		else if (keyword == "Connection:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
			{
				_err_code = 400;
				_err_code_set = true;
				goto reject;
			}
			if (valueword == "close")
				_persistent = false;
			else if (valueword == "keep-alive")
				continue;
			else
			{
				_err_code = 501;
				_err_code_set = true;
				goto reject;			
			}
		}
		else if (keyword == "Host:")
		{
			if (_host_set)
			{
				_err_code = 400;
				_err_code_set = true;
				goto reject;
			}
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r'))
			{
				_err_code = 400;
				_err_code_set = true;
				goto reject;
			}
			std::string::size_type	colon_mark_pos = valueword.find(':');
			if (colon_mark_pos != std::string::npos)
			{
				std::string	host = valueword.substr(0, colon_mark_pos);
				if (colon_mark_pos + 1 >= valueword.size())
				{
					_host = host;
					_host_set = true;
					continue;
				}
				std::string	portstr = valueword.substr(colon_mark_pos + 1, valueword.size());
				int	portvalue = std::atoi(portstr.c_str());
				if (portvalue < 0 || portvalue > 65535)
				{
					_err_code = 400;
					_err_code_set = true;
					goto reject;
				}
				_host = host;
				_host_set = true;
				_port = portvalue;
				_port_set = true;
			}
		}
		else if (keyword == "Authorization:")
		{
			std::string	auth_type;
			std::string	auth_value;
			auth_type.clear();
			auth_value.clear();
			iss>>auth_type;
			iss>>std::ws;
			if (!std::getline(iss, auth_type, '\r') || auth_type.empty() || auth_value.empty())
			{
				_err_code = 400;
				_err_code_set = true;
				goto reject;
			}
			_auth[auth_type] = auth_value;
			_auth_set = true;
		}
		else if (keyword == "Cookie:")
		{
			std::string	combined;
			while (iss>>combined)
			{
				std::string::size_type	equal_mark_pos = combined.find('=');
				if (equal_mark_pos == std::string::npos || equal_mark_pos + 1 >= combined.size())
					continue;
				std::string	c1 = combined.substr(0, equal_mark_pos);
				std::string	c2 = combined.substr(equal_mark_pos + 1, combined.size());
				if (c2[c2.size() - 1] == ';')
					c2.erase(c2.size() - 1);
				_cookie[c1] = c2;
				_cookie_set = true;
				combined.clear();
				c1.clear();
				c2.clear();
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
			{
				_err_code = 501;
				_err_code_set = true;
				goto reject;
			}
			_chunked = true;
		}
		else if (keyword == "Content-Length:")
		{
			iss>>std::ws;
			if (!std::getline(iss, valueword, '\r') || valueword.find_first_not_of("0123456798") != std::string::npos)
			{
				_err_code = 400;
				_err_code_set = true;
				goto reject;
			}
			long	len = std::atol(valueword.c_str());
			_body_length = len;
		}
		else
			continue;
	}
	if (_chunked && _body_length_set)
		goto reject;
	return;
/*
	reject:
	{
		_should_reject = true;
		std::cerr<<"Incoming invalid GET request has been rejected"<<std::endl;
	}
*/
	reject:
	{
		_should_reject = true;
		return;
	}
}

GetRequest::GetRequest(void)
: _target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false),_persistent(true), _chunked(false),
_body_length(0), _body_length_set(false), _err_code(0)
{}

GetRequest::GetRequest(std::string s)
:_target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false),
_should_reject(false), _persistent(true), _chunked(false),
_body_length(0), _body_length_set(false), _err_code(0)
{
	this->process(s);
	this->setErrCode();
}

GetRequest::GetRequest(const GetRequest &other)
:_target(other._target), _target_set(other._target_set), _query(other._query),
_query_set(other._query_set), _host(other._host), _host_set(other._host_set),
_port(other._port), _port_set(other._port_set), _auth(other._auth),
_auth_set(other._auth_set), _cookie(other._cookie), _cookie_set(other._cookie_set),
_should_reject(other._should_reject), _persistent(other._persistent),
_chunked(other._chunked), _body_length(other._body_length),
_body_length_set(other._body_length_set), _err_code(other._err_code)
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
		_body_length = other._body_length;
		_body_length_set = other._body_length_set;
		_err_code = other._err_code;
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
	return (_err_code);
}

long	GetRequest::getBodyLength(void) const
{
	return (_body_length);
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

int			GetRequest::getPort(void) const
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

void	GetRequest::setErrCode(void)
{
	if (_err_code_set)
		return ;
	if (_body_length_set && _chunked)
	{
		_err_code = 501;
		_err_code_set = true;
	}
}
