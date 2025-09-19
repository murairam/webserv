/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenerRegistry.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 12:16:05 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:08:25 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenerRegistry.hpp"
#include "EventLoop.hpp"

/* compare which host (then port) came earlier */
bool	ListenerRegistry::SocketKey::operator<(const SocketKey &s) const
{
	if (_host < s._host)
		return (true);
	if (_host > s._host)
		return (false);
	return (_port < s._port);
}
ListenerRegistry::ListenerRegistry(void)
:_sockets(), _fd_to_server(), _vec_listener() {}

ListenerRegistry::~ListenerRegistry(void){}

void	ListenerRegistry::prepare
(const std::string	&server_name, const std::string &host, int port)
{
	SocketKey	key = {host, port};
	SocketEntry	entry;
	
	std::map<SocketKey,SocketEntry>::iterator	it = _sockets.find(key);
	if (it == _sockets.end())
	{
		entry._listener = Listener(host, port, server_name);
		entry._default_name = server_name;
		_sockets[key] = entry;
	}
	else
		(void)server_name;
}

int		ListenerRegistry::engage_all(EventLoop &loop, int fd)
{
	int	count = 0;
	_fd_to_server.clear();
	_vec_listener.clear();
	
	std::map<SocketKey,SocketEntry>::iterator	it = _sockets.begin();
	while (it != _sockets.end())
	{
		SocketEntry	&se = it->second;
		if (se._listener.listen(fd))
		{
			se._listener.engageLoop(loop);
			/* Record this pair FD<--->server name */
			int	listen_fd = se._listener.getFD();
			_fd_to_server[listen_fd] = se._default_name;
			_vec_listener.push_back(se._listener);
			count++;
		}
		it++;
	}
	return (count);
}
void	ListenerRegistry::disengage_all(EventLoop &loop)
{
	std::map<SocketKey,SocketEntry>::iterator	it = _sockets.begin();
	while (it != _sockets.end())
	{
		SocketEntry	&se = it->second;
		se._listener.disengageLoop(loop);
		it++;
	}
	_fd_to_server.clear();
	_vec_listener.clear();
}
std::string	ListenerRegistry::DetermineServer(int fd) const
{
	std::map<int,std::string>::const_iterator	it = _fd_to_server.find(fd);
	if (it != _fd_to_server.end())
		return (it->second);
	return (std::string());
}
const std::vector<Listener>	&ListenerRegistry::getListeners(void) const
{
	return (_vec_listener);
}
