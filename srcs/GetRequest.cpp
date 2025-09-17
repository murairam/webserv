/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:12 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:56:22 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetRequest.hpp"

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
}

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
		if (line.size() == 1 && line[0] == '\r')
		{
			#ifdef	_DEBUG
			std::cout<<"GetRequest parser debug info: parsing succeeded"<<std::endl;
			#endif
			return ;
		}
		iss>>keyword;
		if (keyword == "GET")
		{
			keyword.clear();
			iss>>keyword;
			std::string::size_type	query_mark_pos = keyword.find('?');
			if (query_mark_pos != std::string::npos)
			{
				std::string	target = keyword.substr(0, query_mark_pos);
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
			#ifdef	_DEBUG
			std::cout<<"GetRequest parser debug info:\ntarget: "<<_target<<(_query_set == true ? "\nquery: " : "")<<(_query_set == true ? _query : "")<<std::endl;
			#endif
		}
	}/*
	reject:
	{
		_should_reject = true;
		std::cerr<<"Incoming invalid GET request has been rejected"<<std::endl;
	}*/
}

GetRequest::GetRequest(void)
:_target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false), _should_reject(false) {}

GetRequest::GetRequest(std::string s):_target(""), _target_set(false), _query(""), _query_set(false),
_host(""), _host_set(false), _port(0), _port_set(false),
_auth(), _auth_set(false), _cookie(), _cookie_set(false), _should_reject(false)
{
	this->process(s);
}

GetRequest::GetRequest(const GetRequest &other)
:_target(other._target), _target_set(other._target_set), _query(other._query),
_query_set(other._query_set), _host(other._host), _host_set(other._host_set),
_port(other._port), _port_set(other._port_set), _auth(other._auth),
_auth_set(other._auth_set), _cookie(other._cookie), _cookie_set(other._cookie_set),
_should_reject(other._should_reject) {}

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
	}
	return (*this);
}

GetRequest::~GetRequest(void) {}

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
bool	GetRequest::selfcheck(void) const
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
