/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:39:02 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:50:41 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void) {}

ServerConfig::ServerConfig(const ServerConfig &other)
:_listeners(other._listeners), _server_names(other._server_names),
_client_max_body_size(other._client_max_body_size),
_error_pages(other._error_pages), _locations(other._locations),
_is_default_on_listener(other._is_default_on_listener),
_index_fallback(other._index_fallback),
_autoindex_fallback(other._autoindex_fallback),
_raw_order(other._raw_order) {}

ServerConfig	&ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		_listeners = other._listeners;
		_server_names = other._server_names;
		_client_max_body_size = other._client_max_body_size;
		_error_pages = other._error_pages;
		_locations = other._locations;
		_is_default_on_listener = other._is_default_on_listener;
		_index_fallback = other._index_fallback;
		_autoindex_fallback = other._autoindex_fallback;
		_raw_order = other._raw_order;
	}
	return (*this);	
}

ServerConfig::~ServerConfig(void) {}

const std::vector<Endpoint>	&ServerConfig::getListeners(void) const
{
	return this->_listeners;
}

const std::vector<std::string>	&ServerConfig::getServerNames(void) const
{
	return this->_server_names;
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

const std::vector<bool>	&ServerConfig::getIsDefaultOnListener(void) const
{
	return this->_is_default_on_listener;
}

const std::vector<std::string>	&ServerConfig::getIndexFallback(void) const
{
	return this->_index_fallback;
}

int	ServerConfig::getAutoindexFallback(void) const
{
	return this->_autoindex_fallback;
}

int	ServerConfig::getRawOrder(void) const
{
	return this->_raw_order;
}
