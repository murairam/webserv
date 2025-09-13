/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Endpoint.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:14:04 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 13:28:08 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Endpoint.hpp"

Endpoint::Endpoint(std::string host, unsigned port, bool is_ipv6)
:_host(host), _port(port), _is_ipv6(is_ipv6) {}

Endpoint::Endpoint(const Endpoint &other)
:_host(other._host), _port(other._port), _is_ipv6(other._is_ipv6) {}

Endpoint &Endpoint::operator=(const Endpoint &other)
{
	if (this != &other)
	{
		_host = other._host;
		_port = other._port;
		_is_ipv6 = other._is_ipv6;
	}
	return (*this);
}

Endpoint::~Endpoint(void) {}

const std::string	&Endpoint::getHost(void) const
{
	return (_host);
}

unsigned	Endpoint::getPort(void) const
{
	return (_port);
}

bool	Endpoint::getIPV6status(void) const
{
	return (_is_ipv6);
}
