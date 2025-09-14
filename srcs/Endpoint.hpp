/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Endpoint.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:14:15 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 19:32:14 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENDPOINT_HPP
# define ENDPOINT_HPP

# include <string>

/*	Endpoint records listen host:port configuration (like 0.0.0.0:443)
*/
class	Endpoint
{
	private:
		std::string		_host;
		int				_port;
		bool			_is_ipv6;

	
	public:
		Endpoint(void);
		Endpoint(const std::string &host, int port, bool is_ipv6);
		Endpoint(const Endpoint &other);
		Endpoint &operator=(const Endpoint &other);
		~Endpoint(void);

		const std::string	&getHost(void) const;
		int					getPort(void) const;
		bool				getIPV6status(void) const;
};

#endif
