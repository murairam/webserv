/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GlobalConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 18:18:46 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 18:46:27 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GlobalConfig.hpp"
GlobalConfig::GlobalConfig(void):_default_server_name("_default_server") {}
GlobalConfig::GlobalConfig(const GlobalConfig &other)
:_config_path(other._config_path), _poll_timeout_ms(other._poll_timeout_ms),
_idle_connection_ms(other._idle_connection_ms), _max_header_bytes(other._max_header_bytes),
_max_request_line_bytes(other._max_request_line_bytes), _default_mime(other._default_mime),
_mime_overrides(other._mime_overrides), _default_error_pages_root(other._default_error_pages_root),
_default_server_name(other._default_server_name) {}

GlobalConfig	&GlobalConfig::operator=(const GlobalConfig &other)
{
	if (this != &other)
	{
		_config_path = other._config_path;
		_poll_timeout_ms = other._poll_timeout_ms;
		_idle_connection_ms = other._idle_connection_ms;
		_max_header_bytes = other._max_header_bytes;
		_max_request_line_bytes = other._max_request_line_bytes;
		_default_mime = other._default_mime;
		_mime_overrides = other._mime_overrides;
		_default_error_pages_root = other._default_error_pages_root;
		_default_server_name = other._default_server_name;
	}
	return (*this);
}
GlobalConfig::~GlobalConfig(void) {}
		
/* Getters */
const std::string	&GlobalConfig::getConfigPath(void) const
{
	return (_config_path);
}

int	GlobalConfig::getPollTimeoutMs(void) const
{
	return (_poll_timeout_ms);
}

int	GlobalConfig::getIdleConnectionMs(void) const
{
	return (_idle_connection_ms);
}

int	GlobalConfig::getMaxHeaderBytes(void) const
{
	return (_max_header_bytes);
}

int	GlobalConfig::getMaxRequestLineBytes(void) const
{
	return (_max_request_line_bytes);
}

const std::string	&GlobalConfig::getDefaultMime(void) const
{
	return (_default_mime);
}

const std::map<std::string,std::string>	&GlobalConfig::getMimeOverrides(void) const
{
	return (_mime_overrides);
}
const std::string	&GlobalConfig::getDefaultErrorPagesRoot(void) const
{
	return (_default_error_pages_root);
}

const std::string	&GlobalConfig::getHardDefaultServerName(void) const
{
	return (_default_server_name);
}

/* Setters */
void	GlobalConfig::setConfigPath(const std::string &path)
{
	_config_path = path;
}

void	GlobalConfig::setPollTimeoutMs(int n)
{
	_poll_timeout_ms = n;
}

void	GlobalConfig::setIdleConnectionMs(int n)
{
	_idle_connection_ms = n;
}

void	GlobalConfig::setMaxHeaderBytes(int n)
{
	_max_header_bytes = n;
}

void	GlobalConfig::setMaxRequestLineBytes(int n)
{
	_max_request_line_bytes = n;
}

void	GlobalConfig::setDefaultMime(const std::string &mime)
{
	_default_mime = mime;
}

void	GlobalConfig::setMimeOverrides(const std::string &ext, const std::string &mime)
{
	_mime_overrides[ext] = mime;
}

void	GlobalConfig::setDefaultErrorPagesRoot(const std::string &path)
{
	_default_error_pages_root = path;
}
