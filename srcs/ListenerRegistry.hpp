/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenerRegistry.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 12:16:29 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 12:42:42 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENERREGISTRY_HPP
# define LISTENERREGISTRY_HPP

# include "_headers.hpp"
# include "Listener.hpp"

class	EventLoop;
class	ConnectionManager;

/*	ListenerRegistry contains all Listener objects;
	It decides which server handles a new connection
	based on the Listener FD;
*/
class	ListenerRegistry
{
	private:
		struct SocketKey
		{
			std::string	_host;
			int			_port;
			bool	operator<(const SocketKey &s) const;
		};
		struct SocketEntry
		{
			Listener	_listener;
			std::string	_default_name;
		};
		
		std::map<SocketKey,SocketEntry>	_sockets;
		std::map<int,std::string>		_fd_to_server;
		std::vector<Listener>			_vec_listener;

		ListenerRegistry(const ListenerRegistry &other);
		ListenerRegistry	&operator=(const ListenerRegistry &other);
	public:
		ListenerRegistry(void);
		~ListenerRegistry(void);

		/*	Bind a server to a pair of host<--->port,
			The first server comes becomes the default server for that pair;
		*/
		void	prepare(const std::string	&server_name, const std::string &host, int port);
		/*	Engage all Listeners to that EventLoop;
			Return the number of successfully opened sockets;
		*/
		int		engage_all(EventLoop &loop, int backlog, ConnectionManager &manager);
		void	disengage_all(EventLoop &loop);

		/*		Determine which server should handle this FD;
		*/
		std::string	DetermineServer(int fd) const;
		const std::vector<Listener>	&getListeners(void) const;
};

#endif
