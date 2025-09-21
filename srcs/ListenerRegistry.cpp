/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenerRegistry.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 12:16:05 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 00:08:06 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenerRegistry.hpp"
#include "EventLoop.hpp"
#include "ConnectionManager.hpp"

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
:_sockets(), _fd_to_server(), _fd_to_config(), _vec_listener() {}

ListenerRegistry::~ListenerRegistry(void){}

void	ListenerRegistry::prepare
(const ServerConfig &server, const Endpoint &endpoint)
{
	SocketKey	key = {endpoint.getHost(), endpoint.getPort()};
	SocketEntry	entry;
	
	std::map<SocketKey,SocketEntry>::iterator	it = _sockets.find(key);
	if (it == _sockets.end())
	{
		entry._listener = Listener(endpoint.getHost(), endpoint.getPort(), server.getServerName());
		entry._default_name = server.getServerName();
		entry._server_cfg = &server;
		_sockets[key] = entry;
	}
	else
		(void)server;
}

int		ListenerRegistry::engage_all(EventLoop &loop, int backlog, ConnectionManager &manager)
{
	int	count = 0;
	_fd_to_server.clear();
	_fd_to_config.clear();
	_vec_listener.clear();

	std::map<SocketKey,SocketEntry>::iterator	it = _sockets.begin();
	while (it != _sockets.end())
	{
		SocketEntry	&se = it->second;
		se._listener.setConnectionManager(&manager);
		se._listener.setServerConfig(se._server_cfg);
		if (se._listener.listen(backlog))
		{
			se._listener.engageLoop(loop);
			/* Record this pair FD<--->server name */
			int	listen_fd = se._listener.getFD();
			_fd_to_server[listen_fd] = se._default_name;
			_fd_to_config[listen_fd] = se._server_cfg;
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
	_fd_to_config.clear();
	_vec_listener.clear();
}
std::string	ListenerRegistry::DetermineServer(int fd) const
{
	std::map<int,std::string>::const_iterator	it = _fd_to_server.find(fd);
	if (it != _fd_to_server.end())
		return (it->second);
	return (std::string());
}

const ServerConfig	*ListenerRegistry::ResolveConfig(int fd) const
{
	std::map<int,const ServerConfig*>::const_iterator	it = _fd_to_config.find(fd);
	if (it != _fd_to_config.end())
		return (it->second);
	return (0);
}
const std::vector<Listener>	&ListenerRegistry::getListeners(void) const
{
	return (_vec_listener);
}
