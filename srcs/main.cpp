/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:13:45 by yanli             #+#    #+#             */
/*   Updated: 2025/10/02 16:49:58 by yanli            ###   ########.fr       */
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

/*	---Tignal handler START---
*/
# include "SignalHandler.hpp"
# include "SignalFDHandler.hpp"

namespace
{
	void	shutdown_callback(void *context)
	{
		if (!context)
			return ;
		EventLoop	*sig_loop = static_cast<EventLoop*>(context);
		std::cout<<"\nwebserv received shutdown singal, terminate"<<std::endl;
		sig_loop->stop();
	}
}
/*	---Signal handler END---
*/

int	main(int argc, char **argv, char **envp)
{
	EventLoop			loop;
	ListenerRegistry	registry;
	ConnectionManager	manager;
	ConfigLoader		cfg;
	int					ret = 0;
/*	---Signal handler START---
*/	SignalHandler	signal_handler;
	SignalFDHandler	signal_fd_handler;
	int				signal_fd = -1;
	bool			signal_installed = false;
/*	---Signal handler END---
*/
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
			cfg = ConfigLoader(std::string("./assets/server_cfgs/GET_ONLY.cfg"));
#ifdef	_DEBUG
		cfg.debug();
#endif
		if (cfg.selfcheck())
		{
			std::cerr<<"\n---ConfigLoader reported fatal error, abort"<<std::endl;
			return (2);
		}
/*	---Signal handler START---
*/
		signal_handler.addSignal(SIGINT);
		signal_handler.addSignal(SIGTERM);
		if (!signal_handler.install())
			std::cerr<<"\n---Signal handler installation failed, you will need to manually abort the webserv";
		signal_installed = true;
		signal_fd = signal_handler.getReadFD();
		if (signal_fd < 0)
			std::cerr<<"\n---Signal handler installation failed, you will need to manually abort the webserv";
		signal_fd_handler = SignalFDHandler (&signal_handler, &shutdown_callback, &loop);
		loop.add(signal_fd, EVENT_READ, &signal_fd_handler);
/*	---Signal handler END---
*/
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
				registry.prepare(server, *it2);
				it2++;
			}
			it++;
		}
		if (!registry.engage_all(loop, SOMAXCONN, manager))
			throw std::runtime_error("\n---Unable to open any listening socket");

		loop.set_timeout(300);
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
/*	---Signal handler START---
*/
	if (signal_fd > -1)
		loop.remove(signal_fd);
	if (signal_installed)
		signal_handler.uninstall();
/*	---Signal handler END---
*/
	return (ret);
}
