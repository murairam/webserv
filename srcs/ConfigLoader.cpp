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

ConfigLoader::ConfigLoader(void) {}

ConfigLoader::ConfigLoader(std::string path)
:_path(path), _servers(), _use_default_server(true),
_server_count(0), _server_index(-1), _default_server(),
_curr_server(), _curr_location(), _curr_endpoint(), _currline(0),
_fatal_error(false)
{
	parse(path);
}

void	ConfigLoader::parse(std::string path)
{
	_servers.clear();
	enum context_enum
	{
		GLOBAL,
		SERVER,
		LOCATION
	};
	int					err_code;
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
		if (setDefaultServer())
			goto fatal_exit;
		throw SysError("Unable to read config file, using default server config", errno);
		return ;
	}
	while(getline(file, line))
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
					goto use_default_server;
				}
				if (_curr_server.getServerName() == "")
				{
					std::cerr<<path<<":"<<_currline<<": Server must have a name; trying to set up default server"<<std::endl;
					goto use_default_server;
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
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty() || !std::isalnum(temp_word[temp_word.size() - 1]))
				{
					std::cerr<<path<<":"<<_currline<<": Invalid server name, it must be terminated by a letter or a digit, it also must be temrminated by a single ';'. trying to set up default server"<<std::endl;
					goto use_default_server;
				}
#ifdef	_DEBUG
				std::cout<<"parser: server_name: "<<temp_word<<std::endl;
#endif
				_curr_server.setServerName(temp_word);
			}
			else if (keyword == "listen")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
					goto use_default_server;
				temp_word.erase(temp_word.size() - 1);
#ifdef	_DEBUG
				std::cout<<"parser: listening on: "<<temp_word<<std::endl;
#endif
				_curr_server.addEndpoint(temp_word);
			}
			else if (keyword == "client_max_body_size")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
					goto use_default_server;
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto use_default_server;
				char	unit;
				unit = temp_word[temp_word.size() - 1];
				if (unit != 'K' && unit != 'M' && unit != 'G' && unit < '0' && unit > '9')
					goto use_default_server;
				if (unit == 'K' || unit == 'M' || unit == 'G')
				{
					temp_word.erase(temp_word.size() - 1);
					if (temp_word.empty())
						goto use_default_server;
					long	limit = std::strtol(temp_word.c_str(), 0, 10);
					_curr_server.setBodySize(limit, unit);
				}
				long	limit = std::strtol(temp_word.c_str(), 0, 10);
				_curr_server.setBodySize(limit, 'B');
#ifdef	_DEBUG
				std::cout<<"parser: client_max_body_size: "<<temp_word<<unit<<std::endl;
#endif
			}
			else if (keyword == "error_page")
			{
				int			err_page_code;
				std::string	err_page;
				if (!(iss>>err_page_code) || err_page_code > 599 || err_page_code < 100)
					goto use_default_server;
				if (!(iss>>err_page))
					goto use_default_server;
#ifdef	_DEBUG
				std::cout<<"parser: error_code: "<<err_page_code<<", err_page: "<<err_page<<std::endl;
#endif
				if (err_page[err_page.size() - 1] == ';')
					err_page.erase(err_page.size() - 1);
				_curr_server.addErrorPage(err_page_code, err_page);
			}
			else if (keyword == "location")
			{
				std::string	location_path;
				if (!(iss>>location_path))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'location'"<<std::endl;
					goto use_default_server;
				}
				_curr_location = LocationConfig();
				_curr_location.setPathPrefix(location_path);
#ifdef	_DEBUG
				std::cout<<"parser: location prefix: "<<location_path<<std::endl;
#endif
				context = LOCATION;
				if (iss>>temp_word)
				{
					if (temp_word != "{")
					{
						std::cerr<<path<<":"<<_currline<<": Unexpected token after location prefix: "<<temp_word<<std::endl;
						goto use_default_server;
					}
					inside_bracket++;
				}
			}
			else
			{
				std::cerr<<path<<":"<<_currline<<": Invalid keyword: "<<keyword<<", trying to set up default server"<<std::endl;
				goto use_default_server;
			}
		}
		else if (keyword == "server" && context == GLOBAL)
		{
#ifdef	_DEBUG
#endif
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
					goto use_default_server;
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
					_curr_server.addLocation(_curr_location);
					context = SERVER;
				}
			}
			else if (keyword == "{")
				inside_bracket++;
			else if (keyword == "root")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'root'"<<std::endl;
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				Directory	folder(temp_word);
#ifdef	_DEBUG
				std::cout<<"Parser: location root path: "<<temp_word<<std::endl;
#endif
				folder.ft_opendir();
				if (!folder.isOpen())
				{
					int	temp_fd = ::open(temp_word.c_str(), O_WRONLY | O_NOFOLLOW);
					if (temp_fd < 0)
					{
						err_code = errno;
						std::cerr<<path<<":"<<_currline<<": Unable to open the file '"<<temp_word<<"': "<<std::strerror(err_code)<<std::endl;
						goto use_default_server;
					}
					::close(temp_fd);
					_curr_location.setRoot(temp_word);
					continue;
				}
				_curr_location.setRoot(temp_word);
				folder.ft_closedir();
			}
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
					else if (temp_word == "OPTIONS" || temp_word == "OPTIONS;")
						method_mask |= OPTIONS_MASK;
					else if (temp_word == "CONNECT" || temp_word == "CONNECT;")
						method_mask |= CONNECT_MASK;
					else if (temp_word == "HEAD" || temp_word == "HEAD;")
						method_mask |= HEAD_MASK;
					temp_word.clear();
				}
				_curr_location.setMethod(method_mask);
			}
			else if (keyword == "index")
			{
				if (!(iss>>temp_word))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'index'"<<std::endl;
					goto use_default_server;
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
					goto use_default_server;
				}
				if (temp_word == "on;")
					_curr_location.setAutoindex(true);
				else
					_curr_location.setAutoindex(false);
			}
			else if (keyword == "upload_path")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'upload_path'"<<std::endl;
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto use_default_server;
				_curr_location.setUploadPath(temp_word);
#ifdef	_DEBUG
				std::cout<<"parser: upload_path: "<<temp_word<<std::endl;
#endif
			}
			else if (keyword == "return")
			{
				int	code;
				if (!(iss>>code))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'return'"<<std::endl;
					goto use_default_server;
				}
				if (!(iss>>temp_word))
				{
					std::cerr<<path<<":"<<_currline<<": Missing redirect target after 'return'"<<std::endl;
					goto use_default_server;
				}
				if (temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Missing ';' at the end of 'return' directive"<<std::endl;
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				_curr_location.setRedirect(code, temp_word);
#ifdef	_DEBUG
				std::cout<<"parser: return code "<<code<<", target: "<<temp_word<<std::endl;
#endif
			}
			else if (keyword == "cgi")
			{
				std::string	extension;
				std::string	handler;
				if (!(iss>>extension) || !(iss>>handler))
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'cgi'"<<std::endl;
					goto use_default_server;
				}
				if (handler[handler.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Missing ';' at the end of 'cgi' directive"<<std::endl;
					goto use_default_server;
				}
				handler.erase(handler.size() - 1);
				if (!extension.empty() && extension[0] != '.')
					extension.insert(extension.begin(), '.');
				_curr_location.addCgiHandler(extension, handler);
#ifdef	_DEBUG
				std::cout<<"parser: cgi handler for "<<extension<<": "<<handler<<std::endl;
#endif
			}
			else if (keyword == "client_max_body_size")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
				{
					std::cerr<<path<<":"<<_currline<<": Unexpected token after 'client_max_body_size'"<<std::endl;
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto use_default_server;
				char unit = temp_word[temp_word.size() - 1];
				if (unit != 'K' && unit != 'M' && unit != 'G')
				{
					std::cerr<<path<<":"<<_currline<<": Invalid unit for 'client_max_body_size'"<<std::endl;
					goto use_default_server;
				}
				temp_word.erase(temp_word.size() - 1);
				if (temp_word.empty())
					goto use_default_server;
				long value = std::strtol(temp_word.c_str(), 0, 10);
				long multiplier = (unit == 'K') ? 1024L : (unit == 'M') ? (1024L * 1024L) : (1024L * 1024L * 1024L);
				_curr_location.setClientBodyLimit(value * multiplier);
#ifdef	_DEBUG
				std::cout<<"parser: location body limit: "<<value<<unit<<std::endl;
#endif
			}
			else
			{
				std::cerr<<path<<":"<<_currline<<": Invalid keyword inside location block: "<<keyword<<std::endl;
				goto use_default_server;
			}
		}
	}
	return;
	use_default_server:
	{
		_servers.clear();
		_use_default_server = true;
		if (setDefaultServer())
			goto fatal_exit;
		return;
	}
	fatal_exit:
	{
		_fatal_error = true;
		std::cerr<<"Fatal error: Unable to set up default server, abort"<<std::endl;
	}
}

ConfigLoader::ConfigLoader(const ConfigLoader &other)
:_path(other._path), _servers(other._servers),
_use_default_server(other._use_default_server),
_server_count(other._server_count), _server_index(other._server_index),
_default_server(other._default_server), _curr_server(other._curr_server),
_curr_location(other._curr_location), _curr_endpoint(other._curr_endpoint),
_currline(other._currline), _fatal_error(other._fatal_error) {}

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
	}
	return (*this);
}
ConfigLoader::~ConfigLoader(void) {}

int	ConfigLoader::setDefaultServer(void)
{
	std::cout<<"Setting up default server"<<std::endl;
	return (0);
}

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
