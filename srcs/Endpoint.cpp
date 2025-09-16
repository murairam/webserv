/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Endpoint.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:14:04 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 08:39:13 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Endpoint.hpp"

Endpoint::Endpoint(void):_host(), _port(0) {}

Endpoint::Endpoint(std::string host, int port)
:_host(host), _port(port) {}

Endpoint::Endpoint(const Endpoint &other)
:_host(other._host), _port(other._port) {}

Endpoint &Endpoint::operator=(const Endpoint &other)
{
	if (this != &other)
	{
		_host = other._host;
		_port = other._port;
	}
	return (*this);
}

Endpoint::~Endpoint(void) {}

const std::string	&Endpoint::getHost(void) const
{
	return (_host);
}

int	Endpoint::getPort(void) const
{
	return (_port);
}

void	Endpoint::setHost(std::string host)
{
	_host = host;
}
void	Endpoint::setPort(int port)
{
	_port = port;
}

#ifdef	_DEBUG
void	Endpoint::debug(void) const
{
	std::cout<<"Endpoint debug info: host: "<<_host<<", port: "<<_port<<std::endl;
}
#endif
