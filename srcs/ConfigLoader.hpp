/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:42 by yanli             #+#    #+#             */
/*   Updated: 2025/09/29 16:47:16 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOADER_HPP
# define CONFIGLOADER_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"
# include "Endpoint.hpp"
# include "Directory.hpp"

class	ConfigLoader
{
	private:
		std::string					_path;
		std::map<int,ServerConfig>	_servers;
		bool						_use_default_server;
		int							_server_count;
		int							_server_index;
		ServerConfig				_default_server;
		ServerConfig				_curr_server;
		LocationConfig				_curr_location;
		Endpoint					_curr_endpoint;
		int							_currline;
		bool						_fatal_error;
		bool						_root_is_folder;
		std::set<std::pair<std::string,int> >	_used_endpoints;

		void	parse(std::string path);

	public:
		ConfigLoader(void);
		ConfigLoader(const ConfigLoader &other);
		ConfigLoader	&operator=(const ConfigLoader &other);
		~ConfigLoader(void);
		ConfigLoader(std::string path);
		
		const std::map<int,ServerConfig>	&getServers(void) const;
		const ServerConfig					&getServerByIndex(int index) const;
		const ServerConfig					&getServerByName(std::string name) const;
		int									getServerCount(void) const;
		const std::string					&getConfigFilePath(void) const;

		const ServerConfig	&operator[](std::string name) const;
		const ServerConfig	&operator[](int index) const;
		bool	selfcheck(void) const;

#ifdef	_DEBUG
		void	debug(void) const;
#endif
};

#endif
