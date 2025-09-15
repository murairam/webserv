/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FD.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:51:23 by yanli             #+#    #+#             */
/*   Updated: 2025/09/15 14:38:55 by mmiilpal         ###   ########.fr       */
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
		new_fd = dup(other._fd);
		if (new_fd < 0)
			throw SysError(std::string("dup failed"), errno);
		_fd = new_fd;
	}
}

FD	&FD::operator=(const FD &other)
{
	int	new_fd;

	if (this != &other)
	{
		if (_fd > -1)
			close(_fd);
		_fd = -1;
		if (other._fd > -1)
		{
			new_fd = dup(other._fd);
			if (new_fd < 0)
				throw SysError("dup failed", errno);
			_fd = new_fd;
		}
	}
	return (*this);
}

/* close FD */
FD::~FD(void)
{
	if (_fd > -1)
	close(_fd);
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
		close(_fd);
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
		close(_fd);
	_fd = -1;
}

FD	FD::openRO(const std::string &path)
{
	int	fd = open(path.c_str(), O_RDONLY);

	if (fd < 0)
		throw SysError("open(READ ONLY) failed: " + path, errno);
	return (FD(fd));
}

FD	FD::openWO(const std::string &path, bool create, mode_t mode)
{
	int	flags = O_WRONLY;
	int	fd;

	if (create)
		flags |= O_CREAT | O_TRUNC;
	fd = open(path.c_str(), flags, mode);
	if (fd < 0)
		throw SysError("open(WRITE ONLY) failed: " + path, errno);
	return (FD(fd));
}

FD	FD::openRW(const std::string &path, bool create, mode_t mode)
{
	int	flags = O_RDWR;
	int	fd;

	if (create)
		flags |= O_CREAT;
	fd = open(path.c_str(), flags, mode);
	if (fd < 0)
		throw SysError("open(READ AND WRITE) failed: " + path, errno);
	return (FD(fd));
}

/*
 * Set file descriptor to non-blocking mode.
 *
 * Subject compliance: poll() and fcntl() are allowed and portable on both Linux and macOS.
 * Platform-specific guards are present for future maintainability.
 */
void	FD::setNonBlockingFD(bool enabled)
{
	int	flags = fcntl(_fd, F_GETFL, 0);
	int	rv;
	if (flags < 0)
		throw SysError("fcntl(F_GETFL) failed", errno);
#ifdef __APPLE__
	// macOS: fcntl is the correct way to set non-blocking mode
	if (enabled)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	rv = fcntl(_fd, F_SETFL, flags);
	if (rv < 0)
		throw SysError("fcntl(F_SETFL) failed", errno);
#elif defined(__LINUX__)
	// Linux: fcntl is also correct, but this guard allows for future tweaks
	if (enabled)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	rv = fcntl(_fd, F_SETFL, flags);
	if (rv < 0)
		throw SysError("fcntl(F_SETFL) failed", errno);
#else
	// Other platforms: fallback (could add more guards if needed)
	if (enabled)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	rv = fcntl(_fd, F_SETFL, flags);
	if (rv < 0)
		throw SysError("fcntl(F_SETFL) failed", errno);
#endif
}

bool	FD::isNonBlockingFD(void) const
{
	int	flags = fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
		throw SysError("fcntl(F_GETFL) failed", errno);
	return ((flags & O_NONBLOCK) != 0);
}
