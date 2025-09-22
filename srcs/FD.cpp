/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FD.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:51:23 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 13:02:27 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FD.hpp"

/* FD initialized to -1 */
FD::FD(void):_fd(-1) {}
/* Acquir FD ownership */
FD::FD(int fd):_fd(fd) {}
/* dup if FD is valid */
FD::FD(const FD &other):_fd(-1)
{
	int	new_fd;
	if (other._fd > -1)
	{
		new_fd = ::dup(other._fd);
		if (new_fd < 0)
			throw SysError("\n---dup failed", errno);
		_fd = new_fd;
	}
}

FD	&FD::operator=(const FD &other)
{
	int	new_fd;
	
	if (this != &other)
	{
		if (_fd > -1)
			::close(_fd);
		_fd = -1;
		if (other._fd > -1)
		{
			new_fd = ::dup(other._fd);
			if (new_fd < 0)
				throw SysError("\n---dup failed", errno);
			_fd = new_fd;
		}
	}
	return (*this);
}

/* close FD */
FD::~FD(void)
{
	if (_fd > -1)
	::close(_fd);
}

bool	FD::isValidFD(void) const
{
	return (_fd > -1);
}

int	FD::getFD(void) const
{
	return (_fd);
}

/* resetFD closes the old FD and take a new FD */
void	FD::resetFD(int fd)
{
	if (_fd > -1)
		::close(_fd);
	_fd = fd;
}

/* abandon FD ownership, return that FD */
int		FD::releaseFD(void)
{
	int	new_fd;

	new_fd = _fd;
	_fd = -1;
	return (new_fd);
}

/* close that FD and set it to -1 */
void	FD::closeFD(void)
{
	if (_fd > -1)
		::close(_fd);
	_fd = -1;
}

FD	FD::openRO(const std::string &path)
{
	int	fd = ::open(path.c_str(), O_RDONLY | O_NOFOLLOW);

	if (fd < 0)
		throw SysError("\n---open(O_RDONLY | O_NOFOLLOW) failed: " + path, errno);
	return (FD(fd));
}

FD	FD::openWO(const std::string &path, bool create, mode_t mode)
{
	int	flags = O_WRONLY | O_NOFOLLOW;
	int	fd;

	if (create)
		flags |= O_CREAT | O_TRUNC;
	fd = ::open(path.c_str(), flags, mode);
	if (fd < 0)
		throw SysError("\n---open(O_WRONLY | O_NOFOLLOW | O_CREAT | O_TRUNC) failed: " + path, errno);
	return (FD(fd));
}

FD	FD::openRW(const std::string &path, bool create, mode_t mode)
{
	int	flags = O_RDWR | O_NOFOLLOW;
	int	fd;
	
	if (create)
		flags |= O_CREAT;
	fd = ::open(path.c_str(), flags, mode);
	if (fd < 0)
		throw SysError("\n---open(O_RDWR | O_NOFOLLOW | O_CREAT) failed: " + path, errno);
	return (FD(fd));
}

/* Check non-blocking flag */
void	FD::setNonBlockingFD(bool enabled)
{
	int	flags = ::fcntl(_fd, F_GETFL, 0);
	int	rv;
	
	if (flags < 0)
		throw SysError("\n---fcntl(F_GETFL) failed", errno);
	if (enabled)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	rv = ::fcntl(_fd, F_SETFL, flags);
	if (rv < 0)
		throw SysError("\n---fcntl(F_SETFL) failed", errno);
}

bool	FD::isNonBlockingFD(void) const
{
	int	flags = ::fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
		throw SysError("\n---fcntl(F_GETFL) failed", errno);
	return ((flags & O_NONBLOCK) != 0);
}
