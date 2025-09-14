/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resolver.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 11:55:47 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 12:31:53 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESOLVER_HPP
# define RESOLVER_HPP

# include "_headers.hpp"
# include "SysError.hpp"

/*	Resolver : getaddrinfo() with string input */
class	Resolver
{
	private:
		std::string		_node;
		std::string		_service;
		int				_family;
		int				_socket_type;
		int				_flags;

	public:
		Resolver(void);
		Resolver(const std::string &node, const std::string &service, int family, int socket_type, int flags);
		Resolver(const Resolver &other);
		Resolver	&operator=(const Resolver &other);
		~Resolver(void);

		/* Collect results as sockaddr_storage data that can be binded with / coneected to */
		std::vector< std::pair< ::sockaddr_storage, socklen_t> >	resolver(void) const;

		void	setNode(const std::string &node);
		void	setService(const std::string &service);
		void	setFamily(int family);
		void	setSocketType(int socket_type);
		void	setFlags(int flags);
};

#endif
