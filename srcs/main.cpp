/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 13:11:13 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "_headers.hpp"
# include "ConfigLoader.hpp"
# include "SysError.hpp"
# include "EventLoop.hpp"
# include "ListenerRegistry.hpp"
# include "ConnectionManager.hpp"
# include "ServerConfig.hpp"
# include "Endpoint.hpp"

int	main(int argc, char **argv, char **envp)
{
	EventLoop			loop;
	ListenerRegistry	registry;
	ConnectionManager	manager;
	ConfigLoader		cfg;
	int					ret = 0;
	
	if (!envp || !*envp || !**envp)
	{
		std::cerr<<ERROR_MSG_INVALID_ENVP<<std::endl;
		return (1);
	}
	try
	{
		
		std::cout<<"Launching webserv"<<std::endl;
		if (argc > 1)
			cfg = ConfigLoader(argv[1]);
		else
			cfg = ConfigLoader();
#ifdef	_DEBUG
		cfg.debug();
#endif
		if (cfg.selfcheck())
		{
			std::cerr<<"\n---ConfigLoader reported fatal error, abort"<<std::endl;
			return (2);
		}
		const std::map<int,ServerConfig>	&servers = cfg.getServers();
		if (servers.empty())
			throw std::runtime_error("\n---No server configuration available");

		std::map<int,ServerConfig>::const_iterator	it = servers.begin();
		while (it != servers.end())
		{
			const ServerConfig	&server = it->second;
			const std::vector<Endpoint>	&endpoints = server.getListeners();
			if (endpoints.empty())
			{
				std::cout<<"Server '"<<server.getServerName()<<"' has no listeners"<<std::endl;
				it++;
				continue;
			}
			std::vector<Endpoint>::const_iterator	it2 = endpoints.begin();
			while (it2 != endpoints.end())
			{
				registry.prepare(server.getServerName(), it2->getHost(), it2->getPort());
				it2++;
			}
			it++;
		}
		if (!registry.engage_all(loop, 128, manager))
			throw std::runtime_error("\n---Unable to open any listening socket");

		loop.set_timeout(400);
		loop.run();
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
		ret = 3;
	}
	catch (...)
	{
		std::cerr<<"\n---Non-standard exception caught"<<std::endl;
		ret = 4;
	}
	manager.drop_all();
	registry.disengage_all(loop);
	return (ret);
}

/* this one is to test Get/Post/Delete Request parsing */
/*
int	main(int argc, char **argv, char **envp)
{
	try
	{
		(void)argc;
		(void)argv;
		(void)envp;
		std::istringstream	input(GET_REQUEST);
		std::istringstream	iss(GET_REQUEST);
		std::string	str(iss.str());
		std::cout<<str<<std::endl;
		Header	test(input);
		if (test.shouldReject())
			return (1);
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
	}
	catch (...)
	{
		std::cerr<<"Non-standard exception caught"<<std::endl;
	}
	return (0);
}*/
