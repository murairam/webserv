/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:42 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 15:43:30 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOADER_HPP
# define CONFIGLOADER_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"
# include "Endpoint.hpp"

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

		int		setDefaultServer(void);
		void	syntaxError(const std::string &msg) const;
		void	require(bool toggle, const std::string &msg) const;

		void	parse(std::string path);
		
	public:
		ConfigLoader(void);
		ConfigLoader(const ConfigLoader &other);
		ConfigLoader	&operator=(const ConfigLoader &other);
		~ConfigLoader(void);
		ConfigLoader(std::string path);
		
		const std::vector<ServerConfig>	&getServers(void) const;
		const ServerConfig				&getServer(int index) const;
		int								ServerCount(void) const;
		const std::string				&getPath(void) const;

		/* Find server index by its name */
		int findServerIndex(const std::string &name) const;
		/* Refresh parsed data (maybe useful or not) */
		void	reload(std::string path);
};

#endif
