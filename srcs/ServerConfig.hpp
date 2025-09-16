/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 19:37:29 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 16:22:09 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "_headers.hpp"
# include "LocationConfig.hpp"
# include "Endpoint.hpp"

class	ServerConfig
{
	private:
		std::string					_server_name;
		std::vector<Endpoint>		_listeners;
		long						_client_max_body_size;	/* default is -1, -1 = unlimited */
		std::map<int,std::string>	_error_pages;
		std::vector<std::string>	_index_fallback;
		std::vector<LocationConfig>	_locations;
	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig &other);
		ServerConfig	&operator=(const ServerConfig &other);
		~ServerConfig(void);

		const std::string	&getServerName(void) const;
		const std::vector<Endpoint>	&getListeners(void) const;
		const std::vector<LocationConfig>	&getLocations(void) const;
		const LocationConfig	*matchLocation(const std::string &path) const;
		long	getBodyLimit(const LocationConfig *loc) const;
		const std::string	&getErrorPage(int code) const;

		void	setServerName(std::string name);
		void	addEndpoint(std::string data);
		void	setBodySize(long n, char c);
		void	addErrorPage(int err_code, std::string err_page);
		void	addLocation(LocationConfig lc);

#ifdef	_DEBUG
		void	debug(void) const;
#endif
};

#endif
