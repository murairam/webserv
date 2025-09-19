/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 12:42:21 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:05:59 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(void):_fd(-1) {}
Socket::Socket(int domain, int type, int protocol):_fd(-1)
{
	int	fd;

	fd = socket(domain, type, protocol);
	if (fd < 0)
		throw SysError("socket failed", errno);
	_fd.resetFD(fd);
}
Socket::Socket(int fd, bool take_ownership):_fd(-1)
{
	if (take_ownership)
		_fd.resetFD(fd);
}

Socket::Socket(const Socket &other):_fd(other._fd) {}
Socket	&Socket::operator=(const Socket &other)
{
	if (this != &other)
		_fd = other._fd;
	return (*this);
}

Socket::~Socket(void) {}

int		Socket::getFD(void) const
{
	return (_fd.getFD());
}
bool	Socket::isValidFD(void) const
{
	return (_fd.isValidFD());
}

void	Socket::resetFD(int fd)
{
	_fd.resetFD(fd);
}

int		Socket::releaseFD(void)
{
	return (_fd.releaseFD());
}

void	Socket::closeFD(void)
{
	_fd.closeFD();
}

/* Configuration */
void	Socket::setReuseAddr(bool enabled) const
{
	int	x;
	int	rv;

	if (enabled)
		x = 1;
	else
		x = 0;
	rv = setsockopt(_fd.getFD(), SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));
	if (rv < 0)
		throw SysError("setsockopt(SO_REUSEADDR) failed", errno);
}

void	Socket::setReusePort(bool enabled) const
{
	int	x;

	if (enabled)
		x = 1;
	else
		x = 0;
	setsockopt(_fd.getFD(), SOL_SOCKET, SO_REUSEPORT, &x, sizeof(x));
}

void	Socket::setNonBlocking(bool enabled)
{
	_fd.setNonBlockingFD(enabled);
}

/* Bind, Listen, Accept a socket */
void	Socket::bindTo(const sockaddr *sa, socklen_t len) const
{
	if (bind(_fd.getFD(), sa, len) < 0)
		throw SysError("Unable to binding to socket", errno);
}

void	Socket::listenOn(int backlog) const
{
	if (listen(_fd.getFD(), backlog) < 0)
		throw SysError("Unable to listen to socket", errno);
}

Socket	Socket::acceptOne(sockaddr_storage &peer_sa, socklen_t &peer_len) const
{
	int	fd;

	peer_len = sizeof(peer_sa);
	::memset(&peer_sa, 0, sizeof(peer_sa));
	fd = ::accept(_fd.getFD(), reinterpret_cast<struct sockaddr*>(&peer_sa), &peer_len);
	if (fd < 0)
		throw SysError("Unable to accept a socket", errno);
	return (Socket(fd, true));
}

/* Connect to a socket */
void	Socket::connectTo(const sockaddr *sa, socklen_t len) const
{
	if (::connect(_fd.getFD(), sa, len) < 0)
		throw SysError("Unable to connect to socket", errno);
}

/* Input/Output */
ssize_t Socket::sendIO(const void *buf, size_t len, int flags) const
{
    ssize_t n;

#ifdef __APPLE__
    n = ::send(_fd.getFD(), buf, len, flags | MSG_DONTWAIT);
#else
    n = ::send(_fd.getFD(), buf, len, flags);
#endif

    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (0);
        throw SysError("Unable to send to socket", errno);
    }
    return (n);
}

ssize_t Socket::recvIO(void *buf, size_t len, int flags) const
{
    ssize_t n;

#ifdef __APPLE__
    n = ::recv(_fd.getFD(), buf, len, flags | MSG_DONTWAIT);
#else
    n = ::recv(_fd.getFD(), buf, len, flags);
#endif

    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (0);
        throw SysError("Unable to receive via socket", errno);
    }
    return (n);
}
