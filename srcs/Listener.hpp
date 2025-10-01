/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:10:58 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 23:10:20 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENER_HPP
# define LISTENER_HPP

# include "_headers.hpp"
# include "IFdHandler.hpp"
# include "SysError.hpp"
# include "utility.hpp"

class	ServerConfig;
class	EventLoop;
class	ConnectionManager;

/*	Listener wraps a non-blocking socket for one pair of host<--->port;
	It accepts new connections once poll() signals readability
*/
class	Listener: public IFdHandler
{
	private:
		std::string	_host;
		int			_port;
		std::string	_server_name;
		int			_fd;
		bool		_engaged;
		EventLoop	*_loop;
		ConnectionManager	*_conn_mgr;
		const ServerConfig	*_server_cfg;
		std::vector<const ServerConfig*>	_server_configs;
		
	public:
		Listener(const Listener &other);
		Listener	&operator=(const Listener &other);
		Listener(void);
		virtual	~Listener(void);
		Listener(const std::string &host, int port, const std::string &server_name);

		virtual void	onReadable(int fd);
		virtual void	onWritable(int fd);
		virtual void	onError(int fd);
		virtual void	onHangup(int fd);
		virtual void	onTick(int fd);

		/*	Bind and listen in non-blocking way
			Put this FD into EventLoop
		*/
		bool	listen(int fd);
		/* Engage this listener into the event loop*/
		void	engageLoop(EventLoop &loop);
		/* Disengage, close FD, quit from the event loop */
		void	disengageLoop(EventLoop &loop);
		void	setConnectionManager(ConnectionManager *manager);
		void	setServerConfig(const ServerConfig *cfg);
		void	setServerConfigs(const std::vector<const ServerConfig*> &configs);

		int			getFD(void) const;
		const std::string	&getHost(void) const;
		int			getPort(void) const;
		const std::string	&getServerName(void) const;
};

#endif
