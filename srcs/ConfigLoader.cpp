/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:31 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 21:31:23by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"


ConfigLoader::ConfigLoader(void)
: _path(), _servers(), _use_default_server(true), _server_count(0),
_server_index(-1), _default_server(), _curr_server(), _curr_location(),
_curr_endpoint(), _currline(0), _fatal_error(false), _root_is_folder(false),
_location_start_line(0), _used_endpoints(), _used_server_names()
{}

ConfigLoader::ConfigLoader(std::string path)
:_path(path), _servers(), _use_default_server(true),
_server_count(0), _server_index(-1), _default_server(),
_curr_server(), _curr_location(), _curr_endpoint(), _currline(0),
_fatal_error(false), _root_is_folder(false), _location_start_line(0), _used_endpoints(), _used_server_names()
{
	parse(path);
}

void	ConfigLoader::parse(std::string path)
{
	_servers.clear();
	_used_endpoints.clear();
	_used_server_names.clear();
	_currline = 0;
	_fatal_error = false;
	_location_start_line = 0;
	enum context_enum
	{
		GLOBAL,
		SERVER,
		LOCATION
	};
	int					err_code = 0;
	std::ifstream		file(path.c_str());
	int					inside_bracket = 0;
	context_enum		context = GLOBAL;
	std::istringstream	iss;
	std::string			line;
	std::string			keyword;
	std::string			temp_word;

#ifdef	_DEBUG
	std::cout<<"The config file path is: "<<path.c_str()<<std::endl;
#endif
	/* If input file is fucked, use default server configuration */
	if (!file)
	{
		err_code = errno;
		throw SysError("Unable to open/read config file, using default server config", err_code);
		goto fatal_exit ;
	}
	while(std::getline(file, line))
	{
		_currline++;
#ifdef	_DEBUG
		std::string	context_debug = ((context == GLOBAL) ? "GLOBAL" : (context == SERVER) ? "SERVER" : (context == LOCATION) ? "LOCATION" : "BULLSHIT");
		std::cout<<"Current context is: "<<context_debug<<std::endl; 
#endif
		if (line.empty() || line[0] == '#')
			continue;
		iss.clear();
		iss.str(line);
		keyword.clear();
		iss>>keyword;
		if (!keyword.empty() && keyword[0] == '#')
			continue;
		temp_word.clear();
#ifdef	_DEBUG
		std::cout<<"Current keyword is: "<<keyword<<std::endl;
#endif
		if (context == SERVER)
		{
			if (keyword == "{")
			{
				_curr_server = ServerConfig();
				inside_bracket++;
				_server_count++;
				_server_index++;
			}
			else if (keyword.size() == 1 && keyword == "}")
			{
				inside_bracket--;
				if (inside_bracket)
				{
					std::cerr<<path<<":"<<_currline<<": Unclosed bracket detected; trying to set up default server"<<std::endl;
					goto fatal_exit;
				}
				if (_curr_server.getServerName() == "")
				{
					std::cerr<<path<<":"<<_currline<<": Server must have a name; trying to set up default server"<<std::endl;
					goto fatal_exit;
				}
				_servers[_server_index] = _curr_server;
				_use_default_server = false;
				context = GLOBAL;
			}
			else if (keyword == "server_name")
                        {
                                if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
                                {
                                        std::cerr<<path<<":"<<_currline<<": Server must have a name and must be terminated by a single ';'. trying up set up default server"<<std::endl;
                                        goto fatal_exit;
                                }
                                temp_word.erase(temp_word.size() - 1);
                                if (temp_word.empty() || !std::isalnum(temp_word[temp_word.size() - 1]))
                                {
                                        std::cerr<<path<<":"<<_currline<<": Invalid server name, it must be terminated by a letter or a digit, it also must be temrminated by a single ';'. trying to set up default server"<<std::endl;
                                        goto fatal_exit;
                                }
                                if (!_curr_server.getServerName().empty())
                                {
                                        std::cerr<<path<<":"<<_currline<<": Duplicate 'server_name' directive within the same server block"<<std::endl;
                                        goto fatal_exit;
                                }
                                if (_used_server_names.find(temp_word) != _used_server_names.end())
                                {
                                        std::cerr<<path<<":"<<_currline<<": Duplicate server_name '"<<temp_word<<"' detected"<<std::endl;
                                        goto fatal_exit;
                                }
#ifdef  _DEBUG
                                std::cout<<"parser: server_name: "<<temp_word<<std::endl;
#endif
                                _curr_server.setServerName(temp_word);
                                _used_server_names.insert(temp_word);
                        }
			else if (keyword == "listen")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
{
					std::cerr<<path<<":"<<_currline<<": Invalid 'listen' directive"<<std::endl;
					goto fatal_exit;
				}
				temp_word.erase(temp_word.size() - 1);
				std::string	endpoint_value = temp_word;
				std::string	host;
				std::string	port_str;
				size_t	colon = endpoint_value.find(':');
				if (colon == std::string::npos)
				{
					host = "0.0.0.0";
					port_str = endpoint_value;
				}
				else
				{
					host = endpoint_value.substr(0, colon);
					port_str = endpoint_value.substr(colon + 1);
				}
				if (host.empty() || host == "*")
					host = "0.0.0.0";
				if (port_str.empty())
				{
					std::cerr<<path<<":"<<_currline<<": Missing port in 'listen' directive"<<std::endl;
					goto fatal_exit;
				}
				char	*endptr = NULL;
				long	port_long = std::strtol(port_str.c_str(), &endptr, 10);
				if (*endptr != '\0')
				{
					std::cerr<<path<<":"<<_currline<<": Invalid port in 'listen' directive"<<std::endl;
					goto fatal_exit;
				}
				if (port_long < 0 || port_long > 65535)
				{
					std::cerr<<path<<":"<<_currline<<": Port out of range in 'listen' directive"<<std::endl;
					goto fatal_exit;
				}
				std::pair<std::string, int>	normalized(host, static_cast<int>(port_long));
				if (_used_endpoints.find(normalized) != _used_endpoints.end())
				{
				std::cerr<<path<<":"<<_currline
				<<": Duplicate listen directive for '"<<host<<":"<<port_long<<"'"<<std::endl;
				goto fatal_exit;
				}
#ifdef	_DEBUG
				std::cout<<"parser: listening on: "<<temp_word<<std::endl;
#endif
				_used_endpoints.insert(normalized);
				try
				{
					_curr_server.addEndpoint(endpoint_value);
				}
				catch (const std::exception &e)
				{
					std::cerr<<path<<":"<<_currline<<": "<<e.what()<<std::endl;
					goto fatal_exit;
				}
			}
			else if (keyword == "client_max_body_size")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
					goto fatal_exit;
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto fatal_exit;
				char	unit;
				unit = temp_word[temp_word.size() - 1];
				if (unit != 'K' && unit != 'M' && unit != 'G' && unit < '0' && unit > '9')
					goto fatal_exit;
				if (unit == 'K' || unit == 'M' || unit == 'G')
				{
					temp_word.erase(temp_word.size() - 1);
					if (temp_word.empty())
						goto fatal_exit;
					long	limit = std::strtol(temp_word.c_str(), 0, 10);
					_curr_server.setBodySize(limit, unit);
				}
				else
				{
					long	limit = std::strtol(temp_word.c_str(), 0, 10);
					_curr_server.setBodySize(limit, 'B');
				}
#ifdef	_DEBUG
				std::cout<<"parser: client_max_body_size: "<<temp_word<<unit<<std::endl;
#endif
			}
			else if (keyword == "error_page")
			{
				int			err_page_code;
				std::string	err_page;
				if (!(iss>>err_page_code) || err_page_code > 599 || err_page_code < 100)
					goto fatal_exit;
				if (!(iss>>err_page))
					goto fatal_exit;
#ifdef	_DEBUG
				std::cout<<"parser: error_code: "<<err_page_code<<", err_page: "<<err_page<<std::endl;
#endif
				if (err_page[err_page.size() - 1] == ';')
					err_page.erase(err_page.size() - 1);
				if (!validateErrorPage(err_page, path, _currline))
					goto fatal_exit;
				_curr_server.addErrorPage(err_page_code, err_page);
			}
			else if (keyword == "location")
			{
				std::string	location_path;
				if (!(iss>>location_path))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'location'"<<std::endl;
					goto fatal_exit;
				}
				_curr_location = LocationConfig();
				_curr_location.setPathPrefix(location_path);
				_location_start_line = _currline;
#ifdef	_DEBUG
				std::cout<<"parser: location prefix: "<<location_path<<std::endl;
#endif
				context = LOCATION;
				if (iss>>temp_word)
				{
					if (temp_word != "{")
					{
						std::cerr<<path<<":"<<_currline<<": Unexpected token after location prefix: "<<temp_word<<std::endl;
						goto fatal_exit;
					}
					inside_bracket++;
				}
			}
			else
			{
				std::cerr<<path<<":"<<_currline<<": Invalid keyword: "<<keyword<<", trying to set up default server"<<std::endl;
				goto fatal_exit;
			}
		}
		else if (keyword == "server" && context == GLOBAL)
		{
			context = SERVER;
			if (iss>>temp_word)
			{
				if (temp_word.size() == 1 && temp_word == "{")
				{
					_curr_server = ServerConfig();
					inside_bracket++;
					_server_count++;
					_server_index++;
				}
				else
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'server': "<<temp_word<<", trying to set up default server"<<std::endl;
					goto fatal_exit;
				}
			}
			else
				continue;
		}
		else if (context == LOCATION)
		{
			if (keyword == "}")
			{
				inside_bracket--;
				if (inside_bracket == 1)
				{
					if (!validateLocationResources(_curr_location, path, _location_start_line))
						goto fatal_exit;
					_curr_server.addLocation(_curr_location);
					context = SERVER;
				}
			}
			else if (keyword == "{")
				inside_bracket++;
			else if (keyword.size() == 1 && keyword[0] == '{')
				inside_bracket++;
			else if (keyword == "methods")
			{
				int	method_mask = 0;
				while (iss>>temp_word)
				{
					if (temp_word == "GET" || temp_word == "GET;")
						method_mask |= GET_MASK;
					else if (temp_word == "DELETE" || temp_word == "DELETE;")
						method_mask |= DELETE_MASK;
					else if (temp_word == "POST" || temp_word == "POST;")
						method_mask |= POST_MASK;
					else if (temp_word == "PUT" || temp_word == "PUT;")
						method_mask |= PUT_MASK;
					else
					{
						std::cerr<<"Only the following methods are supported: GET, POST, DELETE, PUT"<<std::endl;
						goto fatal_exit;
					}
					temp_word.clear();
				}
				_curr_location.setMethod(method_mask);
			}
			else if (keyword == "index")
			{
				if (!(iss>>temp_word))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'index'"<<std::endl;
					goto fatal_exit;
				}
				if (temp_word[temp_word.size() - 1] == ';')
					temp_word.erase(temp_word.size() - 1);	
				_curr_location.addIndexFile(temp_word);
				temp_word.clear();
				while (iss>>temp_word)
				{
					if (temp_word[temp_word.size() - 1] == ';')
						temp_word.erase(temp_word.size() - 1);
					_curr_location.addIndexFile(temp_word);
					temp_word.clear();
				}
			}
			else if (keyword == "autoindex")
			{
				if (!(iss>>temp_word) || !(temp_word == "on;" || temp_word == "off;"))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'autoindex'"<<std::endl;
					goto fatal_exit;
				}
				if (temp_word == "on;")
					_curr_location.setAutoindex(true);
				else
					_curr_location.setAutoindex(false);
			}
			else if (keyword == "alias")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'alias'"<<std::endl;
					goto fatal_exit;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto fatal_exit;
				struct stat	alias_stat;
				std::memset(&alias_stat, 0, sizeof(alias_stat));
                                if (stat(temp_word.c_str(), &alias_stat) < 0)
                                {
                                        err_code = errno;
                                        std::cerr<<path<<":"<<_currline<<": Unable to access alias target '"<<temp_word
                                                <<"': "<<std::strerror(err_code)<<std::endl;
                                        goto fatal_exit;
                                }
                                if (!S_ISDIR(alias_stat.st_mode))
                                {
                                        std::cerr<<path<<":"<<_currline<<": Alias target '"<<temp_word<<"' is not a directory"<<std::endl;
                                        goto fatal_exit;
                                }
                                if (access(temp_word.c_str(), R_OK | X_OK) != 0)
                                {
                                        err_code = errno;
                                        std::cerr<<path<<":"<<_currline<<": Alias target '"<<temp_word<<"' is not accessible: "
                                                <<std::strerror(err_code)<<std::endl;
                                        goto fatal_exit;
                                }
				_curr_location.setAlias(temp_word);
#ifdef	_DEBUG
				std::cout<<"parser: alias: "<<temp_word<<std::endl;
#endif
			}
			else if (keyword == "return")
			{
				int	code;
				if (!(iss>>code))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'return'"<<std::endl;
					goto fatal_exit;
				}
				if (!(iss>>temp_word))
				{
					std::cerr<<path<<":"<<_currline<<": Missing redirect target after 'return'"<<std::endl;
					goto fatal_exit;
				}
				if (temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Missing ';' at the end of 'return' directive"<<std::endl;
					goto fatal_exit;
				}
				temp_word.erase(temp_word.size() - 1);
				_curr_location.setRedirect(code, temp_word);
#ifdef	_DEBUG
				std::cout<<"parser: return code "<<code<<", target: "<<temp_word<<std::endl;
#endif
			}
			else if (keyword == "cgi")
                        {
                                std::string     extension;
                                std::string     handler;
                                if (!(iss>>extension) || !(iss>>handler))
                                {
                                        std::cerr<<path<<":"<<_currline<<": Unexpected token after 'cgi'"<<std::endl;
                                        goto fatal_exit;
                                }
                                if (handler[handler.size() - 1] != ';')
                                {
                                        std::cerr<<path<<":"<<_currline<<": Missing ';' at the end of 'cgi' directive"<<std::endl;
                                        goto fatal_exit;
                                }
                                handler.erase(handler.size() - 1);
                                if (!extension.empty() && extension[0] != '.')
                                        extension.insert(extension.begin(), '.');
                                struct stat     cgi_stat;
                                std::memset(&cgi_stat, 0, sizeof(cgi_stat));
                                if (stat(handler.c_str(), &cgi_stat) < 0)
                                {
                                        err_code = errno;
                                        std::cerr<<path<<":"<<_currline<<": Unable to access CGI executable '"<<handler<<"': "<<std::strerror(err_code)<<std::endl;
                                        goto fatal_exit;
                                }
                                if (!S_ISREG(cgi_stat.st_mode))
                                {
                                        std::cerr<<path<<":"<<_currline<<": CGI executable '"<<handler<<"' is not a regular file"<<std::endl;
                                        goto fatal_exit;
                                }
                                if (access(handler.c_str(), X_OK) != 0)
                                {
                                        err_code = errno;
                                        std::cerr<<path<<":"<<_currline<<": CGI executable '"<<handler<<"' is not executable: "<<std::strerror(err_code)<<std::endl;
                                        goto fatal_exit;
                                }
                                _curr_location.addCgiHandler(extension, handler);
#ifdef  _DEBUG
                                std::cout<<"parser: cgi handler for "<<extension<<": "<<handler<<std::endl;
#endif
                        }
			else if (keyword == "client_max_body_size")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'client_max_body_size'"<<std::endl;
					goto fatal_exit;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto fatal_exit;
				char unit = temp_word[temp_word.size() - 1];
				if (unit != 'B' && unit != 'K' && unit != 'M' && unit != 'G')
				{
					std::cerr<<path<<":"<<_currline<<": Invalid unit for 'client_max_body_size'"<<std::endl;
					goto fatal_exit;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto fatal_exit;
				long value = std::strtol(temp_word.c_str(), 0, 10);
				long multiplier = (unit == 'B') ? 1L : (unit == 'K') ? 1024L : (unit == 'M') ? (1024L * 1024L) : (1024L * 1024L * 1024L);
				_curr_location.setClientBodyLimit(value * multiplier);
#ifdef	_DEBUG
				std::cout<<"parser: location body size limit override is: "<<value<<unit<<" , which is "<<_curr_location.getClientBodyLimit()<<std::endl;
#endif
			}
			else
			{
				std::cerr<<path<<":"<<_currline<<": Invalid keyword inside location block: "<<keyword<<std::endl;
				goto fatal_exit;
			}
		}
	}
	return;
	fatal_exit:
	{
		_fatal_error = true;
		std::cerr<<"Fatal error: Unable to set up the server, abort"<<std::endl;
	}
}

bool    ConfigLoader::validateErrorPage(const std::string &page, const std::string &config_path, int line)
{
        if (page.empty())
        {
                std::cerr<<config_path<<":"<<line<<": Error page path cannot be empty"<<std::endl;
                return (false);
        }
        struct stat     file_stat;
        std::memset(&file_stat, 0, sizeof(file_stat));
        if (stat(page.c_str(), &file_stat) < 0)
        {
                int     err_code = errno;
                std::cerr<<config_path<<":"<<line<<": Unable to access error page '"<<page<<"': "<<std::strerror(err_code)<<std::endl;
                return (false);
        }
        if (!S_ISREG(file_stat.st_mode))
        {
                std::cerr<<config_path<<":"<<line<<": Error page '"<<page<<"' is not a regular file"<<std::endl;
                return (false);
        }
        if (access(page.c_str(), R_OK) != 0)
        {
                int     err_code = errno;
                std::cerr<<config_path<<":"<<line<<": Error page '"<<page<<"' is not readable: "<<std::strerror(err_code)<<std::endl;
                return (false);
        }
        return (true);
}

bool    ConfigLoader::validateLocationResources(const LocationConfig &loc, const std::string &config_path, int line)
{
        const std::string       &alias = loc.getAlias();
        struct stat     resource_stat;
        std::memset(&resource_stat, 0, sizeof(resource_stat));
        if (!alias.empty())
        {
                if (stat(alias.c_str(), &resource_stat) < 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": Unable to access alias target '"<<alias<<"': "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
                if (!S_ISDIR(resource_stat.st_mode))
                {
                        std::cerr<<config_path<<":"<<line<<": Alias target '"<<alias<<"' is not a directory"<<std::endl;
                        return (false);
                }
                if (access(alias.c_str(), R_OK | X_OK) != 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": Alias target '"<<alias<<"' is not accessible: "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
        }
        std::string     base_path = alias.empty() ? std::string(".") : alias;
        const std::vector<std::string>    &index_files = loc.getIndexFiles();
        for (std::vector<std::string>::const_iterator it = index_files.begin(); it != index_files.end(); ++it)
        {
                const std::string &index = *it;
                if (index.empty())
                        continue;
                std::string     index_path = index;
                if (index[0] != '/')
                {
                        if (!base_path.empty() && base_path != ".")
                        {
                                index_path = base_path;
                                if (!index_path.empty() && index_path[index_path.size() - 1] != '/')
                                        index_path += '/';
                                index_path += index;
                        }
                }
                std::memset(&resource_stat, 0, sizeof(resource_stat));
                if (stat(index_path.c_str(), &resource_stat) < 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": Unable to access index file '"<<index_path<<"': "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
                if (!S_ISREG(resource_stat.st_mode))
                {
                        std::cerr<<config_path<<":"<<line<<": Index file '"<<index_path<<"' is not a regular file"<<std::endl;
                        return (false);
                }
                if (access(index_path.c_str(), R_OK) != 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": Index file '"<<index_path<<"' is not readable: "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
        }
        const std::map<std::string, std::string> &cgi_handlers = loc.getCgiHandlers();
        for (std::map<std::string, std::string>::const_iterator it = cgi_handlers.begin(); it != cgi_handlers.end(); ++it)
        {
                const std::string &handler = it->second;
                if (handler.empty())
                        continue;
                std::memset(&resource_stat, 0, sizeof(resource_stat));
                if (stat(handler.c_str(), &resource_stat) < 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": Unable to access CGI executable '"<<handler<<"': "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
                if (!S_ISREG(resource_stat.st_mode))
                {
                        std::cerr<<config_path<<":"<<line<<": CGI executable '"<<handler<<"' is not a regular file"<<std::endl;
                        return (false);
                }
                if (access(handler.c_str(), X_OK) != 0)
                {
                        int     err_code = errno;
                        std::cerr<<config_path<<":"<<line<<": CGI executable '"<<handler<<"' is not executable: "<<std::strerror(err_code)<<std::endl;
                        return (false);
                }
        }
        return (true);
}

ConfigLoader::ConfigLoader(const ConfigLoader &other)
:_path(other._path), _servers(other._servers),
_use_default_server(other._use_default_server),
_server_count(other._server_count), _server_index(other._server_index),
_default_server(other._default_server), _curr_server(other._curr_server),
_curr_location(other._curr_location), _curr_endpoint(other._curr_endpoint),
_currline(other._currline), _fatal_error(other._fatal_error),
_root_is_folder(other._root_is_folder), _location_start_line(other._location_start_line),
_used_endpoints(other._used_endpoints), _used_server_names(other._used_server_names)
{}

ConfigLoader	&ConfigLoader::operator=(const ConfigLoader &other)
{
	if (this != &other)
	{
		_path = other._path;
		_servers = other._servers;
		_use_default_server = other._use_default_server;
		_server_count = other._server_count;
		_server_index = other._server_index;
		_default_server = other._default_server;
		_curr_server = other._curr_server;
		_curr_location = other._curr_location;
		_curr_endpoint = other._curr_endpoint;
		_currline = other._currline;
		_fatal_error = other._fatal_error;
		_root_is_folder = other._root_is_folder;
		_location_start_line = other._location_start_line;
		_used_endpoints = other._used_endpoints;
		_used_server_names = other._used_server_names;
	}
	return (*this);
}
ConfigLoader::~ConfigLoader(void) {}

const ServerConfig	&ConfigLoader::operator[](std::string name) const
{
	std::map<int,ServerConfig>::const_iterator	it = _servers.begin();
	
	while (it != _servers.end())
	{
		if (it->second.getServerName() == name)
			return (it->second);
		it++;
	}
	throw SysError("Unable to find the server called '" + name + "'", 0);
}

#ifdef	_DEBUG
void	ConfigLoader::debug(void) const
{
	std::cout<<"ConfigLoader debug info:\nconfig file path: "<<_path<<"\n";
	int	index = 0;
	while (index < _server_count)
		(*this)[index++].debug();
	if (_use_default_server)
		std::cout<<"using default server: YES\n";
	else
		std::cout<<"using default server: NO\n";
	std::cout<<"server count: "<<_server_count<<std::endl;	
}
#endif

const std::map<int,ServerConfig>	&ConfigLoader::getServers(void) const
{
	return (_servers);
}

const ServerConfig	&ConfigLoader::getServerByIndex(int index) const
{
	std::map<int,ServerConfig>::const_iterator	it = _servers.find(index);
	if (it != _servers.end())
		return (it->second);
	std::ostringstream	oss;
	oss<<index;
	std::string	str = oss.str();
	throw std::runtime_error("No server with index'" + str + "'could be found");
}

int	ConfigLoader::getServerCount(void) const
{
	return (_server_count);
}

const std::string	&ConfigLoader::getConfigFilePath(void) const
{
	return (_path);
}

bool	ConfigLoader::selfcheck(void) const
{
	return (_fatal_error);
}

const ServerConfig	&ConfigLoader::getServerByName(std::string name) const
{
	return ((*this)[name]);
}

const ServerConfig	&ConfigLoader::operator[](int index) const
{
	return ((*this).getServerByIndex(index));
}
