/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:11:14 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:14:02 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Listener.hpp"

Listener::Listener(void)
:_host(), _port(), _server_name(), _fd(-1), _engaged(false)
{}

Listener::~Listener(void) {}

Listener::Listener
(const std::string &host, int port, const std::string &server_name)
:_host(host), _port(port), _server_name(server_name),
_fd(-1), _engaged(false)
{}

void	Listener::onReadable(int fd) { (void)fd; }
void	Listener::onWritable(int fd) { (void)fd; }
void	Listener::onError(int fd) { (void)fd; }
void	Listener::onHangup(int fd) { (void)fd; }
void	Listener::onTick(int fd) { (void)fd; }
bool	Listener::ft_open(int fd) { (void)fd; return false; }
void	Listener::engageLoop(EventLoop &loop) { (void)loop; }
void	Listener::disengageLoop(EventLoop &loop) { (void)loop; }

int	Listener::getFD(void) const
{
	return (_fd);
}

std::string	Listener::getHost(void) const
{
	return (_host);
}

int	Listener::getPort(void) const
{
	return (_port);
}

std::string	Listener::getServerName(void) const
{
	return (_server_name);
}
