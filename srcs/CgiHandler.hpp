/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 12:58:51 by yanli             #+#    #+#             */
/*   Updated: 2025/10/02 13:43:42 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "_headers.hpp"
# include "IFdHandler.hpp"

class	EventLoop;
class	HttpRequest;
class	LocationConfig;

class	CgiHandler: public IFdHandler
{
	private:
		int			_stdin_pipe[2];
		int			_stdout_pipe[2];
		pid_t		_pid;
		std::string	_output;
		std::string	_body;
		size_t		_body_sent;
		bool		_headers_parsed;
		bool		_done;
		int			_status;
		std::map<std::string,std::string>	_headers;
		std::string	_response_body;
		EventLoop	*_loop;
		time_t		_start;
		std::string	_cgi_path;
		std::string	_script;
		std::string	_workdir;
		std::map<std::string,std::string>	_env;

		void	closePipes(void);
		char	**buildEnv(void) const;
		void	freeEnv(char **envp) const;
		bool	parseHeaders(void);

		CgiHandler(void);
		CgiHandler(const CgiHandler &other);
		CgiHandler	&operator=(const CgiHandler &other);
	
	public:
		CgiHandler(const HttpRequest &req, const std::string &cgi, const std::string &script, const LocationConfig *loc);
		virtual ~CgiHandler(void);

		bool	execute(EventLoop &loop);
		bool	isDone(void) const;
		bool	isTimeout(void) const;
		std::string	getResponse(void) const;
		void	removeFromEventLoop(void);

		virtual void	onReadable(int fd);
		virtual void	onWritable(int fd);
		virtual void	onError(int fd);
		virtual void	onHangup(int fd);
		virtual void	onTick(int fd);
};

#endif
