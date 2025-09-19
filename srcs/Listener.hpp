/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:10:58 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 00:53:29 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LISTENER_HPP
# define LISTENER_HPP

# include "_headers.hpp"
# include "IFdHandler.hpp"

class	EventLoop;

class	Listener: public IFdHandler
{
	private:
		std::string	_host;
		int			_port;
		std::string	_server_name;
		int			_fd;
		bool		_engaged;

		Listener(const Listener &other);
		Listener	&operator=(const Listener &other);
		
	public:
		Listener(void);

		virtual	~Listener(void);
		Listener(const std::string &host, int port, const std::string &server_name);

		virtual void	onReadable(int fd);
		virtual void	onWritable(int fd);
		virtual void	onError(int fd);
		virtual void	onHangup(int fd);
		virtual void	onTick(int fd);

		/*	Bind and listen in non-blocking way
			Put this fd into EventLoop
		*/
		bool	ft_open(int fd);
		/* Engage this listener into the event loop*/
		void	engageLoop(EventLoop &loop);
		/* Disengage, close FD, quit from the event loop */
		void	disengageLoop(EventLoop &loop);

		int			getFD(void) const;
		std::string	getHost(void) const;
		int			getPort(void) const;
		std::string	getServerName(void) const;
};

#endif
