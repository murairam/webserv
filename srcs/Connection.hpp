/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/21 20:08:04 by yanli             #+#    #+#             */
/*   Updated: 2025/10/04 15:57:49 by yanli            ###   ########.fr       */
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
# include "EventLoop.hpp"
# include "Response.hpp"
# include "timestring.hpp"
# include "CgiHandler.hpp"

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
		std::string	_inbuf;              // Make sure this exists
		std::string	_outbuf;
		bool		_engaged;
		bool		_should_close;       // Make sure this exists
		const ServerConfig	*_server;
		std::vector<const ServerConfig*>	_available_servers;
		int			_method;
		CgiHandler	*_cgi;
		bool		_request_metadata_ready;
		bool		_request_chunked;
		long		_request_content_length;
		size_t		_request_body_limit;
		bool		_has_request_body_limit;
		size_t		_request_header_end;
		size_t		_chunk_total_bytes;
		size_t		_chunk_scan_offset;
		bool		_pending_cgi;

		// NEW PARSER INTEGRATION
		void	dispatcher(void);    // Make sure this is declared
		bool	handleRequestWithNewParser(void);  // Make sure this is declared
		void	handleParsedRequest(const HttpRequest &request);

		// METHOD-SPECIFIC HANDLERS
		void	handleGetRequest(const HttpRequest &request, const LocationConfig *loc);
		void	handlePostRequest(const HttpRequest& request, const LocationConfig *loc, const std::string &method);
		void	handleDeleteRequest(const HttpRequest &request, const LocationConfig *loc);

		// HELPER METHODS
		std::string	buildFilePath(const LocationConfig *loc, const std::string &target);
		bool	serveFile(const std::string &file_path, int &err_code);
		void	sendErrorResponse(int code);
		bool	selectServerForRequest(const HttpRequest &request);
		void	sendSimpleResponse(int code, std::string content_type = std::string(), std::string body = std::string());
		void	sendRedirectResponse(int code, const std::string &location);
		std::string	resolveRedirectLocation(const std::string &target, const HttpRequest &request) const;
		void	sendDirectoryListing(const std::string &dir_path, const std::string &uri);
		void	handleCgiRequest(const HttpRequest &request, const LocationConfig *loc, const std::string &cgi_program);
		void	checkCgi(void);

		// UTILITY METHODS
		std::string	getContentType(const std::string &file_path) const;
		std::string	getFileExtension(const std::string &path) const;
		std::string	intToString(int value) const;
		bool		enforceRequestBodyLimit(void);
		bool		parseRequestMetadata(void);
		void		resetRequestState(void);
		const ServerConfig	*selectDefaultServer(void) const;
		bool		checkChunkedBodyLimit(void);

		Connection(void);
		Connection(const Connection &other);
		Connection	&operator=(const Connection &other);

	public:
		virtual	~Connection(void);
		Connection(int fd, const std::string &server_name, const ServerConfig *server, const std::vector<const ServerConfig*> &servers);
		/*	This one ensures all write/read/recv/send would be 
			precedented by a poll;
		*/
		Connection(int fd, std::string action, std::string path, int &err_code, std::string filename = std::string());

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

		static bool	handleMultipart(const HttpRequest &request, std::string &filename, std::size_t &content_offset, std::size_t &content_length);
		static bool	uploadFile(const HttpRequest &request, const LocationConfig *loc, std::string &response_body, int &status_code, const std::string &method);
		void	markCgiReady(void);
};

#endif
