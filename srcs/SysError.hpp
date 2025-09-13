/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SysError.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:27:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 00:11:37 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SYSERROR_HPP
# define SYSERROR_HPP

# include "_headers.hpp"

class	SysError: public std::exception
{
	private:
		int			_errno;
		std::string	_msg;
		
	public:
		SysError(void);
		SysError(const char *msg, int errno_value);
		SysError(const std::string &msg, int errno_value);
		SysError(const SysError &other);
		SysError	&operator=(const SysError &other);
		virtual	~SysError(void) throw();

		int			getErrno(void) const;
		const char	*what(void) const throw();
};

#endif
