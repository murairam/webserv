/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:37:33 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:47:06 by yanli            ###   ########.fr       */
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
		std::vector<std::string>	_server_names;
		long						_client_max_body_size;
		ErrorPagesConfig			_error_pages;
		std::vector<LocationConfig>	_locations;
		std::vector<bool>			_is_default_on_listener;
		std::vector<std::string>	_index_fallback;
		int							_autoindex_fallback;
		int							_raw_order;

	public:	
		ServerConfig(void);
		ServerConfig(const ServerConfig &other);
		ServerConfig	&operator=(const ServerConfig &other);
		~ServerConfig(void);

		const std::vector<Endpoint>			&getListeners(void) const;
		const std::vector<std::string>		&getServerNames(void) const;
		long								getClientMaxBodySize(void) const;
		const ErrorPagesConfig				&getErrorPages(void) const;
		const std::vector<LocationConfig>	&getLocations(void) const;
		const std::vector<bool>				&getIsDefaultOnListener(void) const;
		const std::vector<std::string>		&getIndexFallback(void) const;
		int									getAutoindexFallback(void) const;
		int									getRawOrder(void) const;
};

#endif
