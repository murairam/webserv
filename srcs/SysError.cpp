/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SysError.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:32:14 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 12:45:36 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SysError.hpp"

SysError::SysError(void):_errno(0), _msg("") 
{
	if (_errno)
		_msg = std::string(std::strerror(_errno));
}

SysError::SysError(const std::string &msg, int errno_value)
:_errno(errno_value), _msg(msg)
{
	if (_errno)
	{
		_msg += ": ";
		_msg += std::strerror(_errno);
	}
}

SysError::SysError(const SysError &other)
:_errno(other._errno), _msg(other._msg) {}

SysError::SysError(const char *msg, int errno_value)
:_errno(errno_value), _msg(msg)
{
	if (_errno)
	{
		_msg += ": ";
		_msg += std::strerror(_errno);
	}
}

SysError	&SysError::operator=(const SysError &other)
{
	if (this != &other)
	{
		_msg = other._msg;
		_errno = other._errno;
	}
	return (*this);
}

SysError::~SysError(void) throw() {}

int	SysError::getErrno(void) const
{
	return (_errno);
}

const char	*SysError::what(void) const throw()
{
	return (_msg.c_str());
}
