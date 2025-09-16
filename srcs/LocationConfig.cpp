/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:14:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 14:16:17 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(void)
: _path_prefix(), _allowed_methods(0), _root(), _index_files(), _autoindex(false),
_upload_enabled(false), _upload_path(), _redirect_code(0), _redirect_target(),
_client_max_body_size_override(-1), _priority(0), _cgi_handlers() {}

LocationConfig::LocationConfig(const LocationConfig &other)
:_path_prefix(other._path_prefix), _allowed_methods(other._allowed_methods),
_root(other._root), _index_files(other._index_files), _autoindex(other._autoindex),
_upload_enabled(other._upload_enabled), _upload_path(other._upload_path),
_redirect_code(other._redirect_code),
_redirect_target(other._redirect_target), _client_max_body_size_override(other._client_max_body_size_override),
_priority(other._priority), _cgi_handlers(other._cgi_handlers) {}

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
		_cgi_handlers = other._cgi_handlers;
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
	std::map<std::string, std::string>::const_iterator it = _cgi_handlers.find(ext);
	if (it != _cgi_handlers.end())
		return (it->second);
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

void	LocationConfig::setPathPrefix(const std::string &prefix)
{
	_path_prefix = prefix;
}

void	LocationConfig::setRoot(std::string path)
{
	_root = path;
}

void	LocationConfig::setMethod(int method_mask)
{
	_allowed_methods = method_mask;	
}

void	LocationConfig::addIndexFile(std::string file)
{
	_index_files.push_back(file);
}

void	LocationConfig::setAutoindex(bool enabled)
{
	_autoindex = enabled;
}

void	LocationConfig::setUploadPath(const std::string &path)
{
	_upload_path = path;
	_upload_enabled = true;
}

void	LocationConfig::setRedirect(int code, const std::string &target)
{
	_redirect_code = code;
	_redirect_target = target;
}

void	LocationConfig::setClientBodyLimit(long limit)
{
	_client_max_body_size_override = limit;
}

void	LocationConfig::setPriority(int priority)
{
	_priority = priority;
}

void	LocationConfig::addCgiHandler(const std::string &ext, const std::string &program)
{
	_cgi_handlers[ext] = program;
}

void	LocationConfig::debug(void) const
{
	std::cout<<"LocationConfig debug info:\n"
	<<"path_prefix: "<<_path_prefix<<"\n"<<"methods mask: "<<_allowed_methods<<"\n"
	<<"root path: "<<_root<<"\n"<<"index files: ";
	size_t	i = 0;
	while (i < _index_files.size())
	{
		std::cout<<i<<": "<<_index_files[i]<<"\n";
		i++;
	}
	if (_autoindex)
		std::cout<<"autoindex is enabled\n";
	else
		std::cout<<"autoindex is disabled\n";
	if (_upload_enabled)
		std::cout<<"upload is enabled\n"<<"upload path: "<<_upload_path<<"\n";
	else
		std::cout<<"upload is disabled\n";
	std::cout<<"redirect code: "<<_redirect_code<<"\nredirect target: "<<_redirect_target
	<<"\nclient max body size override: "<<_client_max_body_size_override<<"\npriority: "<<_priority<<std::endl;
	if (!_cgi_handlers.empty())
	{
		std::map<std::string, std::string>::const_iterator it = _cgi_handlers.begin();
		while (it != _cgi_handlers.end())
		{
			std::cout<<"cgi handler: "<<it->first<<" -> "<<it->second<<"\n";
			++it;
		}
	}
}
