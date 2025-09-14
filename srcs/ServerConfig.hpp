/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 19:37:29 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 20:24:42 by yanli            ###   ########.fr       */
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
		int							_autoindex;	/* -1 if unset, 0 if disabled, 1 if enabled*/
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
};

#endif
