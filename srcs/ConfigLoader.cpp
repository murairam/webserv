/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:31 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 16:56:41 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"

ConfigLoader::ConfigLoader(void) {}

ConfigLoader::ConfigLoader(std::string path)
:_path(path), _servers(), _use_default_server(true),
_server_count(0), _server_index(-1), _default_server(),
_curr_server(), _curr_location(), _curr_endpoint()
{
	parse(path);
}

void	ConfigLoader::parse(std::string path)
{
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
	int					temp_int;

	_servers.clear();
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
#ifdef	_DEBUG
		std::string	context_debug = ((context == GLOBAL) ? "GLOBAL" : (context == SERVER) ? "SERVER" : (context == LOCATION) ? "LOCATION" : "BULLSHIT");
		std::cout<<"Current context is: "<<context_debug<<std::endl; 
#endif
		if (line.empty() || line[1] == '#')
			continue;
		iss.clear();
		iss.str(line);
		iss>>keyword;
#ifdef	_DEBUG
		std::cout<<"Current keyword is: "<<keyword<<std::endl;
#endif
		if (context == SERVER)
		{
			if (keyword == "server_name")
			{
				if (!(iss>>temp_word) || temp_word[temp_word.size() - 1] != ';')
					goto use_default_server;
				temp_word.erase(temp_word.size() - 1);
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
				char	unit;
				unit = temp_word[temp_word.size() - 1];
				if (unit != 'K' && unit != 'M' && unit != 'G')
					goto use_default_server;
				temp_word.erase(temp_word.size() - 1);
#ifdef	_DEBUG
				std::cout<<"parser: client_max_body_size: "<<temp_word<<std::endl;
#endif
			}
			else if (keyword == "error_page")
			{
				int			err_code;
				std::string	err_page;
				if (!(iss>>err_code) || err_code > 599 || err_code < 100)
					goto use_default_server;
				if (!(iss>>err_page))
					goto use_default_server;
#ifdef	_DEBUG
				std::cout<<"parser: error_code: "<<err_code<<", err_page: "<<err_page<<std::endl;
#endif
				_curr_server.addErrorPage(err_code, err_page);
			}
			else if (keyword == "location")
			{
				if (iss>>temp_word)
				{
					if (temp_word == "{")
					{
						context = LOCATION;
						inside_bracket++;
						_curr_location = LocationConfig();
					}
					else
					{
						std::cerr<<"Unexpected token after 'location': "<<temp_word<<", trying to set up default server"<<std::endl;
						goto use_default_server;
					}
				}
				else
					continue;
			}
			else
			{
				std::cerr<<"Invalid keyword: "<<keyword<<", trying to set up default server"<<std::endl;
				goto use_default_server;
			}
		}
		else if (keyword == "server" && context == GLOBAL)
		{
#ifdef	_DEBUG
#endif
			if (iss>>temp_word)
			{
				if (temp_word == "{")
				{
					context = SERVER;
					_curr_server = ServerConfig();
					inside_bracket++;
					_server_count++;
				}
				else
				{
					std::cerr<<"Unexpected token after 'server': "<<temp_word<<", trying to set up default server"<<std::endl;
					goto use_default_server;
				}
			}
			else
				continue;
		}
	}
	use_default_server:
		_servers.clear();
		_use_default_server = true;
		if (setDefaultServer())
			goto fatal_exit;
		else
			return;
	fatal_exit:
		std::cerr<<"Fatal error: Unable to set up default server, abort"<<std::endl;
		std::exit(1);
}

ConfigLoader::ConfigLoader(const ConfigLoader &other)
:_path(other._path), _servers(other._servers),
_use_default_server(other._use_default_server),
_server_count(other._server_count), _server_index(other._server_index),
_default_server(other._default_server), _curr_server(other._curr_server),
_curr_location(other._curr_location), _curr_endpoint(other._curr_endpoint) {}

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
	}
	return (*this);
}
ConfigLoader::~ConfigLoader(void) {}

int	ConfigLoader::setDefaultServer(void)
{
	return (0);
}
