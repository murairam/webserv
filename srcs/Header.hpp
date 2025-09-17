/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:24:15 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:07:45 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADER_HPP
# define HEADER_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "GetRequest.hpp"
# include "PostRequest.hpp"
# include "DeleteRequest.hpp"

class	Header
{
	private:
		int			_request_method;
		GetRequest	_get;
		PostRequest	_post;
		DeleteRequest	_delete;
		bool		_should_reject;

		int	process(const std::string &str);

	public:
		Header(void);
		~Header(void);
		Header(const Header &other);
		Header	&operator=(const Header &other);
		Header(std::string s);
};

#endif
