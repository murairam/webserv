/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Resolver.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 11:55:39 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 12:26:39 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Resolver.hpp"

Resolver::Resolver(void)
:_node(), _service(), _family(AF_UNSPEC), _socket_type(SOCK_STREAM), _flags(0) {}

Resolver::Resolver(const std::string &node, const std::string &service, int family, int socket_type, int flags)
:_node(node), _service(service), _family(family), _socket_type(socket_type), _flags(flags) {}

Resolver::Resolver(const Resolver &other)
:_node(other._node), _service(other._service), _family(other._family), _socket_type(other._socket_type), _flags(other._flags) {}

Resolver	&Resolver::operator=(const Resolver &other)
{
	if (this != &other)
	{
		_node = other._node;
		_service = other._service;
		_family = other._family;
		_socket_type = other._socket_type;
		_flags = other._flags;
	}
	return (*this);
}

Resolver::~Resolver(void) {}

/* Collect results as sockaddr_storage data that can be binded with / coneected to */
std::vector< std::pair< ::sockaddr_storage, socklen_t> >
Resolver::resolver(void) const
{
	std::vector< std::pair< ::sockaddr_storage, socklen_t > >	data;
	struct addrinfo	request;
	struct addrinfo	*result;
	struct addrinfo	*curr;
	int				rv;

	std::memset(&request, 0, sizeof(request));
	request.ai_family = _family;
	request.ai_socktype = _socket_type;
	request.ai_flags = _flags;
	rv = ::getaddrinfo(_node.empty() ? 0 : _node.c_str(),
					_service.empty() ? 0 : _service.c_str(),
					&request, &result);
	if (rv)
	{
		std::string	msg("getaddrinfo failed: ");
		msg += ::gai_strerror(rv);
		throw SysError(msg, 0);
	}
	curr = result;
	while (curr)
	{
		std::pair< sockaddr_storage, socklen_t >	t;
		std::memset(&t.first, 0, sizeof(t.first));
		if (curr->ai_addrlen > sizeof(t.first))
		{
#ifdef	_DEBUG
			throw SysError("addrinfo result size too large for sockaddr_storage", 0);			
#else
			curr = curr->ai_next;
			continue;
#endif
		}
		else
		{
			std::memcpy(&t.first, curr->ai_addr, curr->ai_addrlen);
			t.second = static_cast<socklen_t>(curr->ai_addrlen);
			data.push_back(t);
		}
		curr = curr->ai_next;
	}
	::freeaddrinfo(result);
	return (data);
}

void	Resolver::setNode(const std::string &node)
{
	_node = node;
}
void	Resolver::setService(const std::string &service)
{
	_service = service;
}
void	Resolver::setFamily(int family)
{
	_family = family;
}
void	Resolver::setSocketType(int socket_type)
{
	_socket_type = socket_type;
}
void	Resolver::setFlags(int flags)
{
	_flags = flags;
}
