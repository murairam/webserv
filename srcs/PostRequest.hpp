/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:37 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 23:24:17 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTREQUEST_HPP
# define POSTREQUEST_HPP

# include "_headers.hpp"

class	PostRequest
{
	private:
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
		
		void	process(const std::string &s);
		void	setErrCode(void);

	public:
		PostRequest(void);
		~PostRequest(void);
		PostRequest(std::string s);
		PostRequest(const PostRequest &other);
		PostRequest	&operator=(const PostRequest &other);
		bool	isTargetSet(void) const;
		bool	isQuerySet(void) const;
		bool	isHostSet(void) const;
		bool	isPortSet(void) const;
		bool	isAuthSet(void) const;
		bool	isCookieSet(void) const;
		bool	isPersistent(void) const;
		bool	isChunked(void) const;
		bool	isBodySet(void) const;
		bool	isBodyLengthSet(void) const;
		bool	isErrCodeSet(void) const;
		bool	shouldReject(void) const;
		
		std::string	getTarget(void) const;
		std::string	getQuery(void) const;
		std::string	getHost(void) const;
		int			getPort(void) const;
		long		getBodyLength(void) const;
		int			getErrCode(void) const;
		std::string	getBody(void) const;
		std::map<std::string,std::string>	getAuth(void) const;
		std::map<std::string,std::string>	getCookie(void) const;
};

#endif
