/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 20:40:42 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 21:00:36 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOADER_HPP
# define CONFIGLOADER_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"

class	ConfigLoader
{
	private:
		std::string					_path;
		std::vector<ServerConfig>	_servers;
		std::map<std::string,int>	_server_index;
		/* The following are only used during parsing */
		int							_line;
		int							_cursor;
		std::vector<std::string>	_tokens;
		std::string					_lookahead;
		ServerConfig				_current_server;
		LocationConfig				_current_location;

		void	parseFile(const std::string &path);
		void	parseServerBlock(void);
		void	parseLocationBlock(ServerConfig &srv);
		void	parseListen(ServerConfig &srv, const std::vector<std::string> &token);
		void	parseErrorPage(ServerConfig &srv, const std::vector<std::string> &token);
		void	parseIndexFallback(ServerConfig &srv, const std::vector<std::string> &token);
		void	parseAutoindexFallback(ServerConfig &srv, const std::vector<std::string> &token);
		void	parseServerName(ServerConfig &srv, const std::vector<std::string> &token);
		long	parseSizeWithUnits(const std::string &token);
		void	finalize(ServerConfig &srv);
		void	buildNameIndex(void);

		void	syntaxError(const std::string &msg) const;
		void	require(bool toggle, const std::string &msg) const;

		ConfigLoader(void);
		
	public:
		ConfigLoader(const ConfigLoader &other);
		ConfigLoader	&operator=(const ConfigLoader &other);
		~ConfigLoader(void);
		ConfigLoader(const std::string &path);
		
		const std::vector<ServerConfig>	&getServers(void) const;
		const ServerConfig				&getServer(int index) const;
		int								ServerCount(void) const;
		const std::string				&getPath(void) const;

		/* Find server index by its name */
		int findServerIndex(const std::string &name) const;
		/* Refresh parsed data (maybe useful or not) */
		void	reload(const std::string &path);
};

#endif
