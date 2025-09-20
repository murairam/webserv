/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 14:16:35 by yanli             #+#    #+#             */
/*   Updated: 2025/09/21 00:50:06 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "IFdHandler.hpp"
# include "_headers.hpp"
# include "utility.hpp"

class	EventLoop;

/*	Each Connection object is a client connection that:
	Non-blocking FD integrated with that poll() event loop;
	Input buf on readble;
	Output buf on writable;
*/
class	Connection: public IFdHandler
{
	private:
		int			_fd;
		EventLoop	*_loop;
		std::string	_server_name;
		std::string	_inbuf;
		std::string	_outbuf;
		bool		_engaged;
		bool		_should_close;

		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);
	public:
		Connection(void);
		virtual	~Connection(void);
		Connection(int fd, const std::string &server_name);

		void	engageLoop(EventLoop &loop);
		/* Quits the loop and close the socket */
		void	disengageLoop(void);
		/* Queue dta to send, enable POLLOUT in the loop */
		void	queueWrite(const std::string &data);
		/* Give ownership of input data to caller */
		void	takeInput(std::string &dest);
		/* Mark it so it closes once job done */
		void	requestClose(void);

		int	getFD(void) const;
		const std::string	&getServerName(void) const;
		bool	isEngaged(void) const;
		bool	isClose(void) const;
		
		virtual void	onReadable(int fd);
		virtual void	onWritable(int fd);
		virtual void	onError(int fd);
		virtual void	onHangup(int fd);
		virtual void	onTick(int fd);
};

#endif
