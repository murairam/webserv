/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:42 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 16:24:04 by yanli            ###   ########.fr       */
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
		std::map<std::string,ServerConfig>	_servers;
		bool						_use_default_server;
		int							_server_count;
		int							_server_index;
		ServerConfig				_default_server;
		ServerConfig				_curr_server;
		LocationConfig				_curr_location;
		Endpoint					_curr_endpoint;
		bool						_fatal_error;

		int	_currline;

		int		setDefaultServer(void);

		void	parse(std::string path);
		
	public:
		ConfigLoader(void);
		ConfigLoader(const ConfigLoader &other);
		ConfigLoader	&operator=(const ConfigLoader &other);
		~ConfigLoader(void);
		ConfigLoader(std::string path);
		
		const std::map<std::string,ServerConfig>	&getServers(void) const;
		const ServerConfig				&getServer(const std::string &name) const;
		int								getServerCount(void) const;
		const std::string				&getConfigFilePath(void) const;

		const ServerConfig	&operator[](std::string name) const;
		bool	selfcheck(void) const;

#ifdef	_DEBUG
		void	debug(void) const;
#endif
};

#endif
