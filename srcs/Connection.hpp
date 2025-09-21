/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 20:08:04 by yanli             #+#    #+#             */
/*   Updated: 2025/09/21 22:08:52 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "IFdHandler.hpp"
# include "_headers.hpp"
# include "ServerConfig.hpp"
# include "GetRequest.hpp"
# include "PostRequest.hpp"
# include "DeleteRequest.hpp"

class	EventLoop;

/*	Each Connection object is a client connection that:
	Non-blocking FD integrated with that poll() event loop;
	Input buf on readble;
	Output buf on writable;
*/
class	Connection: public IFdHandler
{
	private:
		struct Request
		{
			std::string	_target;
			bool		_target_set;
			std::string	_query;
			bool		_query_set;
			std::string	_host;
			bool		_host_set;
			int			_port;
			bool		_port_set;
			std::map<std::string,std::string>	_auth;
			bool		_auth_set;
			std::map<std::string,std::string>	_cookie;
			bool		_cookie_set;
			bool		_should_reject;
			bool		_persistent;
			bool		_chunked;
			long		_body_length;
			bool		_body_length_set;
			int			_err_code;
			bool		_err_code_set;
			std::string	_body;
			bool		_body_set;
		};

		int			_fd;
		EventLoop	*_loop;
		const std::string	&_server_name;
		std::string	_inbuf;
		std::string	_outbuf;
		bool		_engaged;
		bool		_should_close;
		const ServerConfig	*_server;
		int			_method;
		Request		r;

		void	parseGET(std::istream &s);
		void	parsePOST(std::istream &s);
		void	parseDELETE(std::istream &s);
		void	dispatcher(void);
		void	resetRequest(void);

		Connection(void);
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);

	public:
		virtual	~Connection(void);
		Connection(int fd, const std::string &server_name, const ServerConfig *server);

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
