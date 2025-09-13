/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:46:45 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(void) {}
LocationConfig::LocationConfig(const LocationConfig &other)
:_path_prefix(other._path_prefix), _allowed_methods(other._allowed_methods),
_root(other._root), _index_files(other._index_files), _autoindex(other._autoindex),
_upload_enabled(other._upload_enabled), _upload_path(other._upload_path),
_cgi(other._cgi), _redirect_code(other._redirect_code),
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
		_cgi = other._cgi;
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

const CgiMapping	&LocationConfig::getCgi(void) const
{ return this->_cgi; }

int	LocationConfig::getRedirectCode(void) const
{ return this->_redirect_code; }

const std::string	&LocationConfig::getRedirectTarget(void) const
{ return this->_redirect_target; }

long	LocationConfig::getClientMax(void) const
{ return this->_client_max_body_size_override; }

int	LocationConfig::getPriority(void) const
{ return this->_priority; }
