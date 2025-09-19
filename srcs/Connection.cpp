/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 14:16:51 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 14:49:03 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "EventLoop.hpp"

namespace
{
	bool	set_nonblock_fd(int fd)
	{
		int	flags = ::fcntl(fd, F_GETFL, 0);

		if (flags < 0)
			return (false);
		if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			return (false);
		return (true);
	}
}

Connection::Connection(void)
:_fd(-1), _loop(0), _server_name(), _inbuf(), _outbuf(),
_engaged(false), _should_close(false)
{}

Connection::~Connection(void)
{
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

Connection::Connection
(int fd, const std::string &server_name)
:_fd(fd), _loop(0), _server_name(server_name), _inbuf(),
_outbuf(), _engaged(false), _should_close(false)
{
	(void)set_nonblock_fd(_fd);
}

void	Connection::engageLoop(EventLoop &loop)
{
	int	events = EVENT_READ;
	
	if (_fd < 0)
		return;
	_loop = &loop;
	if (!_outbuf.empty())
		events = events | EVENT_WRITE;
	_loop->add(_fd, events, this);
	_engaged = true;
}

void	Connection::disengageLoop(void)
{
	if (_engaged && _loop)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::queueWrite(const std::string &data)
{
	int	events = EVENT_READ;
	
	if (_fd < 0)
		return ;
	_outbuf.append(data);
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
}
void	Connection::takeInput(std::string &dest)
{
	dest.swap(_inbuf);
}

void	Connection::requestClose(void)
{
	_should_close = true;
}

int	Connection::getFD(void) const
{
	return (_fd);
}

const std::string	&Connection::getServerName(void) const
{
	return (_server_name);
}

bool	Connection::isEngaged(void) const
{
	return (_engaged);
}

bool	Connection::isClose(void) const
{
	return (_fd < 0);
}
void	Connection::onReadable(int fd)
{
	char	buf[8192];
	ssize_t	n = ::recv(_fd, buf, static_cast<int>(8192u), 0);
	(void)fd;
	
	if (n > 0)
		_inbuf.append(buf, static_cast<size_t>(n));
	else if (!n)
		_should_close = true;
	/* n < 0 must be ignored, otherwise this shit blocks and all fuck up*/
}

void	Connection::onWritable(int fd)
{
	ssize_t	n;
	int		events = EVENT_READ;
	(void)fd;

	if (!_outbuf.empty())
	{
		n = ::send(_fd, _outbuf.data(), static_cast<int>(_outbuf.size()), 0);
		if (n > 0)
			_outbuf.erase(0, static_cast<size_t>(n));
	}
	if (_loop && _engaged)
	{
		if (!_outbuf.empty())
			events = events | EVENT_WRITE;
		_loop->set_events(_fd, events);
	}
	if (_should_close && _outbuf.empty())
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}

void	Connection::onError(int fd)
{
	(void)fd;
	
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onHangup(int fd)
{
	(void)fd;
	
	if (_loop && _engaged)
	{
		_loop->remove(_fd);
		_engaged = false;
	}
	if (_fd > -1)
	{
		(void)::close(_fd);
		_fd = -1;
	}
}

void	Connection::onTick(int fd)
{
	(void)fd;
	
	if (_should_close && _outbuf.empty())
	{
		if (_loop && _engaged)
		{
			_loop->remove(_fd);
			_engaged = false;
		}
		if (_fd > -1)
		{
			(void)::close(_fd);
			_fd = -1;
		}
	}
}
