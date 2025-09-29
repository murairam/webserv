/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 20:08:04 by yanli             #+#    #+#             */
/*   Updated: 2025/09/29 18:03:41 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "IFdHandler.hpp"
# include "_headers.hpp"
# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include "HttpRequestParser.hpp"
# include "CodePage.hpp"

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
		const std::string	&_server_name;
		std::string	_inbuf;              // Make sure this exists
		std::string	_outbuf;
		bool		_engaged;
		bool		_should_close;       // Make sure this exists
		const ServerConfig	*_server;
		int			_method;

		// NEW PARSER INTEGRATION
		void        dispatcher(void);    // Make sure this is declared
		bool        handleRequestWithNewParser(void);  // Make sure this is declared
		void        handleParsedRequest(const HttpRequest& request);

		// METHOD-SPECIFIC HANDLERS
		void        handleGetRequest(const HttpRequest& request, const LocationConfig* loc);
		void        handlePostRequest(const HttpRequest& request, const LocationConfig* loc);
		void        handleDeleteRequest(const HttpRequest& request, const LocationConfig* loc);

		// HELPER METHODS
		std::string buildFilePath(const LocationConfig *loc, const std::string &target);
		bool        serveFile(const std::string &file_path);
		void        sendErrorResponse(int code);
		void        sendSimpleResponse(int code, const std::string& content_type, const std::string& body);
		void        sendRedirectResponse(int code, const std::string& location);
		void        sendDirectoryListing(const std::string& dir_path, const std::string& uri);
		void        handleFileUpload(const HttpRequest& request, const LocationConfig* loc);
		void        handleCgiRequest(const HttpRequest& request, const LocationConfig* loc, const std::string& cgi_program);

		// UTILITY METHODS
		std::string getContentType(const std::string &file_path) const;
		std::string getFileExtension(const std::string& path) const;
		std::string getReasonPhrase(int code) const;
		std::string intToString(int value) const;

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
