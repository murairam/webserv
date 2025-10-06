/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 19:37:19 by yanli             #+#    #+#             */
/*   Updated: 2025/10/01 16:51:19 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
:_server_name(), _listeners(), _client_max_body_size(-1), _error_pages(),
_index_fallback(), _locations(), _priority(0)
{

}

ServerConfig::ServerConfig(const ServerConfig &other)
:_server_name(other._server_name), _listeners(other._listeners),
_client_max_body_size(other._client_max_body_size),
_error_pages(other._error_pages), _index_fallback(other._index_fallback),
_locations(other._locations) , _priority(other._priority)
{}

ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		_server_name = other._server_name;
		_listeners = other._listeners;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_index_fallback = other._index_fallback;
		_locations = other._locations;
		_priority = other._priority;
	}
	return (*this);
}

ServerConfig::~ServerConfig(void) {}

const std::string	&ServerConfig::getServerName(void) const
{
	return (_server_name);
}
const std::vector<Endpoint>	&ServerConfig::getListeners(void) const
{
	return (_listeners);
}
const std::vector<LocationConfig>	&ServerConfig::getLocations(void) const
{
	return (_locations);
}

/*	Longest prefix matach
*/
const LocationConfig	*ServerConfig::matchLocation(const std::string &path) const
{
	if (path.empty())
		return (0);

	const std::string		cleanPath = stripSlash(path);
	size_t					best_len = 0;
	const LocationConfig	*best = 0;
	int						best_priority = 0;
	bool					has_best = false;
	size_t					i = 0;

	while( i < _locations.size())
	{
		std::string	cleanPrefix = stripSlash(_locations[i].getPathPrefix());
		if (cleanPrefix.empty() || !matchPath(cleanPath,cleanPrefix))
		{
			i++;
			continue;
		}
		size_t	prefix_len = cleanPrefix.size();
		int	priority = _locations[i].getPriority();
		if (!has_best || prefix_len > best_len ||
		(prefix_len == best_len && priority > best_priority))
		{
			best = &_locations[i];
			best_len = prefix_len;
			best_priority = priority;
			has_best = true;
		}
		i++;
	}
	return (best);
}

long	ServerConfig::getBodyLimit(const LocationConfig *loc) const
{
#ifdef	_DEBUG
	std::cerr<<"\nDEBUG: client max body size limit is: "<<_client_max_body_size<<std::endl;
#endif
	if (!loc || loc->getClientBodyLimit() < 0)
		return (_client_max_body_size);
	return (loc->getClientBodyLimit());
}

/*	It is guaranteed that basic version of error pages
	is existent through CodePage; here it searches if
	any custom error page is set;	
*/
std::string	ServerConfig::getErrorPage(int code) const
{
	std::map<int,std::string>::const_iterator	it = _error_pages.find(code);
	if (it != _error_pages.end())
		return (it->second);
	return (std::string());
}

void	ServerConfig::setServerName(std::string name)
{
	_server_name = name;
}

void	ServerConfig::addEndpoint(std::string data)
{
	std::string	host;
	size_t		pos = data.find(':');
	if (pos == std::string::npos)
		host = ("0.0.0.0");
	else
		host = data.substr(0, pos);
	if (host == "*")
		host = ("0.0.0.0");
	int			port = std::atoi(data.substr(pos + 1).c_str());
#ifdef	_DEBUG
	std::cout<<"ServerConfig::addEndpoint debug info: "<<host<<":"<<port<<std::endl;
#endif
	if (port < 0 || port > 65535)
		throw std::runtime_error("Invalid endpoint(port) has been ignored: " + data);
	_listeners.push_back(Endpoint(host, port));
}

void	ServerConfig::setBodySize(long n, char c)
{
	if (c == 'B')
		_client_max_body_size = n;
	else if (c == 'K')
		_client_max_body_size = 1024L * n;
	else if (c == 'M')
		_client_max_body_size = 1024L * 1024L * n;
	else if (c == 'G')
		_client_max_body_size = 1024L * 1024L * 1024L * n;
	else
		_client_max_body_size = -1;
}

void	ServerConfig::addErrorPage(int err_code, std::string err_page)
{
	_error_pages[err_code] = err_page;
}

#ifdef	_DEBUG
void	ServerConfig::debug(void) const
{
	std::cout<<"ServerConfig debug info:\n"
	<<"server name: "<<_server_name<<"\n";

	size_t	i = 0;
	while (i < _listeners.size())
		_listeners[i++].debug();
	std::cout<<"client max body size: "<<_client_max_body_size
	<<"\n";
	std::map<int,std::string>::const_iterator	it = _error_pages.begin();
	while (it != _error_pages.end())
	{
		std::cout<<"error pages: "<<it->first<<" "<<it->second<<"\n";
		it++;
	}
	i = 0;
	while (i < _index_fallback.size())
		std::cout<<"index fallback: "<<_index_fallback[i++]<<"\n";
	i = 0;
	while (i < _locations.size())
		_locations[i++].debug();
}
#endif

void	ServerConfig::addLocation(LocationConfig lc)
{
	_locations.push_back(lc);
}
