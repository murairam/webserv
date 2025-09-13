/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:37:33 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 17:57:09 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <vector>
# include <string>
# include <cstddef>
# include "LocationConfig.hpp"
# include "ErrorPagesConfig.hpp"
# include "Endpoint.hpp"

class	ServerConfig
{
	private:
		std::vector<Endpoint>		_listeners;
		std::string					_server_name;
		long						_client_max_body_size;
		ErrorPagesConfig			_error_pages;
		std::vector<LocationConfig>	_locations;
		std::vector<std::string>	_index_fallback;
		int							_autoindex_fallback;

	public:	
		ServerConfig(void);
		ServerConfig(const ServerConfig &other);
		ServerConfig	&operator=(const ServerConfig &other);
		~ServerConfig(void);

		const std::vector<Endpoint>			&getListeners(void) const;
		const std::string					&getServerName(void) const;
		long								getClientMaxBodySize(void) const;
		const ErrorPagesConfig				&getErrorPages(void) const;
		const std::vector<LocationConfig>	&getLocations(void) const;
		const std::vector<std::string>		&getIndexFallback(void) const;
		int									getAutoindexFallback(void) const;
};

#endif
