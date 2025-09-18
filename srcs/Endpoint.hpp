/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Endpoint.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:14:15 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 08:58:12 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENDPOINT_HPP
# define ENDPOINT_HPP

# include "_headers.hpp"

/*	Endpoint records listen host:port configuration (like 0.0.0.0:443)
*/
class	Endpoint
{
	private:
		std::string		_host;
		int				_port;
	
	public:
		Endpoint(void);
		Endpoint(std::string host, int port);
		Endpoint(const Endpoint &other);
		Endpoint &operator=(const Endpoint &other);
		~Endpoint(void);

		const std::string	&getHost(void) const;
		int					getPort(void) const;
		void				setHost(std::string host);
		void				setPort(int port);

#ifdef	_DEBUG
		void	debug(void) const;
#endif
};

#endif
