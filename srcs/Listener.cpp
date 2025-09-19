/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:11:14 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:13:22 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Listener.hpp"
#include "EventLoop.hpp"

namespace
{
	bool	set_nonblock_fd(int fd)
	{
		int	flags = ::fcntl(fd, F_GETFL, 0);

		if (flags < 0)
			return (false);
		if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			return (false);
		return (true);
	}

	bool	parse_ipv4(const std::string &host, struct in_addr *ret)
	{
		unsigned long	a = 0;
		unsigned long	b = 0;
		unsigned long	c = 0;
		unsigned long	d = 0;
		unsigned long	r = 0;
		size_t			i = 0;
		int				part = 0;
		int				letter = 0;
		
		if (host.empty() || host == "0.0.0.0")
		{
			ret->s_addr = htonl(INADDR_ANY);
			return (true);
		}
		if (host == "localhost" || host == "127.0.0.1")
		{
			ret->s_addr = htonl(INADDR_LOOPBACK);
			return (true);
		}
		while (i < host.size())
		{
			letter = host[i];
			if (letter >= '0' && letter <= '9')
			{
				r = r * 10 + static_cast<unsigned long>((letter - '0'));
				if (r > 255UL)
					return (false);
			}
			else if (letter == '.')
			{
				if (!part)
					a = r;
				else if (part == 1)
					b = r;
				else if (part == 2)
					c = r;
				else
					return (false);
				r = 0;
				part++;
			}
			else
				return (false);
			i++;
		}
		if (part != 3)
			return (false);
		d = r;
		if (d > 255UL)
			return (false);
		ret->s_addr = htonl((a<<24) | (b<<16) | (c<<8) | d);
		return (true);
	}
}

Listener::Listener(void)
:IFdHandler(), _host(), _port(0), _server_name(), _fd(-1), _engaged(false)
{}

Listener::~Listener(void)
{
	if (_fd > -1)
		(void)::close(_fd);
	_fd = -1;
	_engaged = false;
}

Listener::Listener
(const std::string &host, int port, const std::string &server_name)
:IFdHandler(), _host(host), _port(port), _server_name(server_name),
_fd(-1), _engaged(false)
{}

bool	Listener::listen(int fd)
{
	struct in_addr		ip;
	int					s = ::socket(AF_INET, SOCK_STREAM, 0);
	int					set = 1;
	struct sockaddr_in	addr;

	if (s < 0)
		return (false);
	(void)::setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set));
	
	if (!set_nonblock_fd(s))
	{
		(void)::close(s);
		return (false);
	}
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	if (!parse_ipv4(_host, &ip))
	{
		(void)::close(s);
		return (false);
	}
	addr.sin_addr = ip;
	if (::bind(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
	{
		(void)::close(s);
		return (false);
	}
	if (::listen(s, fd) < 0)
	{
		(void)::close(s);
		return (false);
	}
	_fd = s;
	return (true);
}

void	Listener::onReadable(int fd)
{
	(void)fd;

	while (1)
	{
		int					client_fd;
		struct sockaddr_in	peer;
		socklen_t			len;

		std::memset(&peer, 0, sizeof(peer));
		len = static_cast<socklen_t>(sizeof(peer));
		client_fd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&peer), &len);
		if (client_fd < 0)
			break;
		if (set_nonblock_fd(client_fd))
		{
			(void)::close(client_fd);
			continue;
		}
		/* This is where Connection should set up */
		(void)::close(client_fd);
	}
}

/* Listener never use POLLOUT */
void	Listener::onWritable(int fd)
{
	(void)fd;
}

/* Ignore errors */
void	Listener::onError(int fd)
{
	(void)fd;
}

/* Consider it never hangs with non-block */
void	Listener::onHangup(int fd)
{
	(void)fd;
}

/* This one is irrelevent for a listener */
void	Listener::onTick(int fd)
{
	(void)fd;
}

int	Listener::getFD(void) const
{
	return (_fd);
}

const std::string	&Listener::getHost(void) const
{
	return (_host);
}

int	Listener::getPort(void) const
{
	return (_port);
}

const std::string	&Listener::getServerName(void) const
{
	return (_server_name);
}

void	Listener::engageLoop(EventLoop &loop)
{
	if (_fd < 0 || _engaged)
		return ;
	loop.add(_fd, EVENT_READ, this);
	_engaged = true;
}

void	Listener::disengageLoop(EventLoop &loop)
{
	if (_engaged)
	{
		loop.remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}
Listener::Listener(const Listener &other)
:IFdHandler(other), _host(other._host), _port(other._port), _server_name(other._server_name),
_fd(-1), _engaged(false) {}

Listener	&Listener::operator=(const Listener &other)
{
	if (this != &other)
	{
		if (_fd > -1)
			(void)::close(_fd);
		_host = other._host;
		_port = other._port;
		_server_name = other._server_name;
		_fd = -1;
		_engaged = false;
	}
	return (*this);
}
