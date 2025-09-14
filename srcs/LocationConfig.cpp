/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 20:15:11 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(void)
: _path_prefix(), _allowed_methods(0), _root(), _index_files(), _autoindex(false),
_upload_enabled(false), _upload_path(), _redirect_code(0), _redirect_target(),
_client_max_body_size_override(-1), _priority(0) {}

LocationConfig::LocationConfig(const LocationConfig &other)
:_path_prefix(other._path_prefix), _allowed_methods(other._allowed_methods),
_root(other._root), _index_files(other._index_files), _autoindex(other._autoindex),
_upload_enabled(other._upload_enabled), _upload_path(other._upload_path),
_redirect_code(other._redirect_code),
_redirect_target(other._redirect_target), _client_max_body_size_override(other._client_max_body_size_override),
_priority(other._priority) {}

LocationConfig	&LocationConfig::operator=(const LocationConfig &other)
{
	if (this != &other)
	{
		_path_prefix = other._path_prefix;
		_allowed_methods = other._allowed_methods;
		_root = other._root;
		_index_files = other._index_files;
		_autoindex = other._autoindex;
		_upload_enabled = other._upload_enabled;
		_upload_path = other._upload_path;
		_redirect_code = other._redirect_code;
		_redirect_target = other._redirect_target;
		_client_max_body_size_override = other._client_max_body_size_override;
		_priority = other._priority;
	}
	return (*this);
}

LocationConfig::~LocationConfig(void) {}

const std::string	&LocationConfig::getPathPrefix(void) const
{ return this->_path_prefix; }

int	LocationConfig::getAllowedMethods(void) const
{ return this->_allowed_methods; }

const std::string	&LocationConfig::getRoot(void) const
{ return this->_root; }

const std::vector<std::string>	&LocationConfig::getIndexFiles(void) const
{ return this->_index_files; }

bool	LocationConfig::getAutoindex(void) const
{ return this->_autoindex; }

bool	LocationConfig::getUploadEnabled(void) const
{ return this->_upload_enabled; }

const std::string	&LocationConfig::getUploadPath(void) const
{ return this->_upload_path; }

int	LocationConfig::getRedirectCode(void) const
{ return this->_redirect_code; }

std::string	LocationConfig::getCgi(std::string ext) const
{
	if (ext == ".php")
		return (std::string("/usr/bin/php-cgi"));
	if (ext == ".rb")
		return (std::string("/usr/bin/ruby"));
	return (std::string());
}

const std::string	&LocationConfig::getRedirectTarget(void) const
{ return this->_redirect_target; }

long	LocationConfig::getClientBodyLimit(void) const
{ return this->_client_max_body_size_override; }

int	LocationConfig::getPriority(void) const
{ return this->_priority; }
