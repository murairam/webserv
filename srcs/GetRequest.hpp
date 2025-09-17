/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:22 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:08:17 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETREQUEST_HPP
# define GETREQUEST_HPP

# include "_headers.hpp"
# include "utility.hpp"

class	GetRequest
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

		bool	containInvalidSection(const std::string &s) const;
		bool	containInvalidLineEnd(const std::string &S) const;
		void	process(const std::string &s);

	public:
		GetRequest(void);
		GetRequest(std::string s);
		GetRequest(const GetRequest &other);
		GetRequest	&operator=(const GetRequest &other);
		~GetRequest(void);

		bool	isTargetSet(void) const;
		bool	isQuerySet(void) const;
		bool	isHostSet(void) const;
		bool	isPortSet(void) const;
		bool	isAuthSet(void) const;
		bool	isCookieSet(void) const;
		bool	selfcheck(void) const;
		
		std::string	getTarget(void) const;
		std::string	getQuery(void) const;
		std::string	getHost(void) const;
		int			getPort(void) const;
		std::map<std::string,std::string>	getAuth(void) const;
		std::map<std::string,std::string>	getCookie(void) const;

};
#endif
