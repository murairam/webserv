/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 19:37:19 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 16:09:08 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
ServerConfig::ServerConfig(void)
:_server_name(), _listeners(), _client_max_body_size(-1), _error_pages(),
_index_fallback(), _autoindex(-1), _locations()
{

}

ServerConfig::ServerConfig(const ServerConfig &other)
:_server_name(other._server_name), _listeners(other._listeners),
_client_max_body_size(other._client_max_body_size),
_error_pages(other._error_pages), _index_fallback(other._index_fallback),
_autoindex(other._autoindex), _locations(other._locations) {}

ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		_server_name = other._server_name;
		_listeners = other._listeners;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_index_fallback = other._index_fallback;
		_autoindex = other._autoindex;
		_locations = other._locations;
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
		
	size_t					best_len = 0;
	size_t					i = 0;
	const LocationConfig	*best = 0;
	while (i < _locations.size())
	{
		const std::string	&prefix = _locations[i].getPathPrefix();
		const size_t		plen = prefix.size();

		if (!plen || plen > path.size())
			continue;
		if (!path.compare(0, plen, prefix))
		{
			if (plen > best_len)
			{
				best = &_locations[i];
				best_len = plen;
			}
		}
		i++;
	}
	return (best);
}

long	ServerConfig::getBodyLimit(const LocationConfig *loc) const
{
	if (!loc || loc->getClientBodyLimit() < 0)
		return (_client_max_body_size);
	return (loc->getClientBodyLimit());
}

/*	Error page for 404 is guaranteed to be existent;
	that emptystr is there to avoid compiler warning
*/
const std::string	&ServerConfig::getErrorPage(int code) const
{
	std::map<int,std::string>::const_iterator	it = _error_pages.find(code);
	if (it != _error_pages.end())
		return (it->second);
	std::map<int,std::string>::const_iterator	it2 = _error_pages.find(404);
	if (it2 != _error_pages.end())
		return (it2->second);
	static const std::string	emptystr;
	return (emptystr);
}

void	ServerConfig::setServerName(std::string name)
{
	_server_name = name;
}

void	ServerConfig::addEndpoint(std::string data)
{
	std::string::size_type	pos = data.find(':');
	if (pos == std::string::npos)
		throw std::runtime_error("Invalid endpoint has been ignored: " + data);
	std::string	host = data.substr(0, pos);
	int			port = std::atoi(data.substr(pos + 1).c_str());

	if (port < 0 || port > 65536)
		throw std::runtime_error("Invalid endpoint has been ignored: " + data);
	_listeners.push_back(Endpoint(host, port));
}

void	ServerConfig::setBodySize(long n, char c)
{
	if (c == 'K')
		_client_max_body_size = 1024 * n;
	else if (c == 'M')
		_client_max_body_size = 1024 * 1024 * n;
	else if (c == 'G')
		_client_max_body_size = 1024 * 1024 * 1024 * n;
	else
		_client_max_body_size = -1;
}

void	ServerConfig::addErrorPage(int err_code, std::string err_page)
{
	_error_pages[err_code] = err_page;
}
