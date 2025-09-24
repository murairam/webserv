/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:24:15 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 19:53:34 by yanli            ###   ########.fr       */
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
		bool		_reject_400;
		bool		_reject_405;

		int	process(std::istream &s);

	public:
		Header(void);
		~Header(void);
		Header(const Header &other);
		Header	&operator=(const Header &other);
		Header(std::istream &s);

		bool	shouldReject(void) const;
		int		rejectCode(void) const;
};

#endif
