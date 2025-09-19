/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 14:17:24 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 15:22:25 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionManager.hpp"
#include "Connection.hpp"

ConnectionManager::ConnectionManager(void)
:_conns(){}
ConnectionManager::~ConnectionManager(void)
{
	drop_all();
}
Connection	*ConnectionManager::establish(int client_fd, const std::string &server_name, EventLoop &loop)
{

	try
	{
		if (client_fd < 0)
			return (0);
		Connection	*c = new Connection(client_fd, server_name);
		c->engageLoop(loop);
		_conns[client_fd] = c;
		return (c);
	}
	catch (const std::exception &e)
	{
		(void)::close(client_fd);
		return (0);
	}
	catch (...)
	{
		(void)::close(client_fd);
		return (0);
	}
}

void	ConnectionManager::drop(int fd)
{
	std::map<int,Connection*>::iterator	it = _conns.find(fd);
	if (it == _conns.end())
		return;
	if (it->second)
		delete it->second;
	_conns.erase(it);
}

void	ConnectionManager::drop_all(void)
{
	std::map<int,Connection*>::iterator	it = _conns.begin();
	while (it != _conns.end())
	{
		Connection	*c = it->second;
		std::map<int,Connection*>::iterator	tempit = it;
		it++;
		if (c)
			delete c;
		_conns.erase(tempit);
	}
}

Connection	*ConnectionManager::getConn(int fd) const
{
	std::map<int,Connection*>::const_iterator	it = _conns.find(fd);
	if (it != _conns.end())
		return (it->second);
	return (0);
}

size_t	ConnectionManager::getMapSize(void) const
{
	return (_conns.size());
}
