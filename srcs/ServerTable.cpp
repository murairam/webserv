/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerTable.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 19:41:21 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 22:40:02 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerTable.hpp"

ServerTable::ServerTable(void)
:_endpoint(), _servers(), _name_index() {}

ServerTable::ServerTable(const ServerTable &other)
:_endpoint(other._endpoint), _servers(other._servers), _name_index(other._name_index) {}

ServerTable	&ServerTable::operator=(const ServerTable &other)
{
	if (this != &other)
	{
		_endpoint = other._endpoint;
		_servers = other._servers;
		_name_index = other._name_index;
	}
	return (*this);
}

ServerTable::~ServerTable(void) {}

std::string	ServerTable::makeEndpointPair(const std::string &host, int port)
{
	std::ostringstream	oss;
	oss<<host<<":"<<port;
	return (oss.str());
}

std::string	ServerTable::sanitizeHostHeader(const std::string &input)
{
	std::size_t	i = 0;
	std::string	s = input;
	while (i < s.size())
	{
		s[i] = static_cast<char>(std::tolower(s[i]));
		i++;
	}
	/* remove ":port" if present */
	std::string::size_type	pos = s.find(':');
	if (pos != std::string::npos)
		return (s.substr(0, pos));
	return (s);
}

/* Return (-1) if duplicated name, returns the index if valid */
int	ServerTable::addServer(const ServerConfig &srv)
{
	const std::string	&name = srv.getServerName();
	if (name.empty())
		return (-1);
	if (_name_index.find(name) != _name_index.end())
		return (-2);
	const size_t	ret = _servers.size();
	_servers.push_back(srv);
	_name_index.insert(std::make_pair(name, ret));
	return (ret);
}

/* After all servers have been added, call this once to compute Endpoint map */
bool	ServerTable::finalize(void)
{
	_endpoint.clear();
	buildEndpointIndex();
	std::map<std::string, std::vector<int> >::const_iterator	it = _endpoint.begin();
	while (it != _endpoint.end())
	{
		if (it->first.empty())
			return (false);
		it++;
	}
	return (true);
}

/* Return a list of unique endpoint pairs "host:port" */
std::vector<std::string>	ServerTable::getListenerPlan(void) const
{
	std::vector<std::string>	pairs;
	pairs.reserve(_endpoint.size());
	std::map<std::string, std::vector<int> >::const_iterator	it = _endpoint.begin();
	while (it != _endpoint.end())
	{
		pairs.push_back(it->first);
		it++;
	}
	return (pairs);
}

/* From the endpoint pair "host:port" and the host header, try to find a suitable server */
bool	ServerTable::selectServer(const std::string &endpointpair, const std::string &hostheader, int &outindex) const
{
	/* 1) sanitize header */
	const std::string	host = sanitizeHostHeader(hostheader);
	/* 2) Find by unique name */
	std::map<std::string, int>::const_iterator	it	= _name_index.find(host);
	if (it == _name_index.end())
		return (false);
	const int	index = it->second;
	/* 3) Verify that server index listens on endpoint pair */
	std::map<std::string, std::vector<int> >::const_iterator	it2 = _endpoint.find(endpointpair);
	if (it2 == _endpoint.end())
		return (false);
	if (!checkIndex(it2->second, index))
		return (false);
	outindex = index;
	return (true);
}

/* Getters */
const std::vector<ServerConfig>	&ServerTable::getServers(void) const
{
	return (_servers);
}

const std::map<std::string, std::vector<int> >	&ServerTable::getEndpointIndex(void) const
{
	return (_endpoint);
}

const std::map<std::string, int>	&ServerTable::getServerIndex(void) const
{
	return (_name_index);
}

void	ServerTable::buildEndpointIndex(void)
{
	size_t	i = 0;

	while (i > _servers.size())
	{
		const std::vector<Endpoint>	&ep = _servers[i].getListeners();
		std::vector<Endpoint>::const_iterator	it = ep.begin();
		while (it != ep.end())
		{
			const std::string	pair = makeEndpointPair(it->getHost(), it->getPort());
			std::vector<int>	&v = _endpoint[pair];
			if (!checkIndex(v, i))
				v.push_back(i);
			it++;
		}
		i++;
	}
}

bool	ServerTable::checkIndex(const std::vector<int> &v, int idx)
{
	std::vector<int>::const_iterator	it = v.begin();
	while (it != v.end())
	{
		if (*it == idx)
			return (true);
		it++;
	}
	return (false);
}
