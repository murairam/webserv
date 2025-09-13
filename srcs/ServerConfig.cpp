/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:39:02 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 17:56:55 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void) {}

ServerConfig::ServerConfig(const ServerConfig &other)
:_listeners(other._listeners), _server_name(other._server_name),
_client_max_body_size(other._client_max_body_size),
_error_pages(other._error_pages), _locations(other._locations),
_index_fallback(other._index_fallback),
_autoindex_fallback(other._autoindex_fallback) {}

ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		_listeners = other._listeners;
		_server_name = other._server_name;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_locations = other._locations;
		_index_fallback = other._index_fallback;
		_autoindex_fallback = other._autoindex_fallback;
	}
	return (*this);	
}

ServerConfig::~ServerConfig(void) {}

const std::vector<Endpoint>	&ServerConfig::getListeners(void) const
{
	return this->_listeners;
}

const std::string	&ServerConfig::getServerName(void) const
{
	return this->_server_name;
}

long	ServerConfig::getClientMaxBodySize(void) const
{
	return this->_client_max_body_size;
}

const ErrorPagesConfig	&ServerConfig::getErrorPages(void) const
{
	return this->_error_pages;
}

const std::vector<LocationConfig>	&ServerConfig::getLocations(void) const
{
	return this->_locations;
}

const std::vector<std::string>	&ServerConfig::getIndexFallback(void) const
{
	return this->_index_fallback;
}

int	ServerConfig::getAutoindexFallback(void) const
{
	return this->_autoindex_fallback;
}
