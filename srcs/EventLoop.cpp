/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:57:27 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 13:28:17 by mmiilpal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IFdHandler.hpp"
#include "EventLoop.hpp"

static volatile sig_atomic_t	g_wakeup = -1;

namespace
{
	void	eventloop_signal_handler(int sig)
	{
		if (g_wakeup > -1)
			(void)::write(g_wakeup, "1", 1);
		(void)sig;
	}

	bool	set_nonblock_fd(int fd)
	{
		int	flags = ::fcntl(fd, F_GETFL, 0);

		if (flags < 0)
			return (false);
		if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			return (false);
		return (true);
	}

	void	drain_pipe_once(int fd)
	{
		char	buf[5000];
		(void)::read(fd, buf, sizeof(buf));
	}

	std::time_t	getTime(void)
	{
		return (std::time(0));
	}
}

EventLoop::EventLoop(void)
: _entries(), _scratch_fds(), _scratch_evs(), _wakeup_rd_fd(-1),
_wakeup_wr_fd(-1), _should_stop(false), _timeout(0)
{
	int	pipefd[2] = {-1, -1};

	/* Creation of the self-pipe for wakeup, both ends are non-blocking */
	if (!::pipe(pipefd))
	{
		_wakeup_rd_fd = pipefd[0];
		_wakeup_wr_fd = pipefd[1];
		if (!set_nonblock_fd(_wakeup_rd_fd)
			|| !set_nonblock_fd(_wakeup_wr_fd))
		{
			::close(_wakeup_rd_fd);
			::close(_wakeup_wr_fd);
			_wakeup_rd_fd = -1;
			_wakeup_wr_fd = -1;
		}
	}
	/* If creation of self-pipe succeeded, engage the read end of this pipe */
	if (_wakeup_rd_fd > -1)
		this->add(_wakeup_rd_fd, EVENT_READ, 0);
}

EventLoop::EventLoop(const EventLoop &other)
: _entries(), _scratch_fds(), _scratch_evs(), _wakeup_rd_fd(-1),
_wakeup_wr_fd(-1), _should_stop(false), _timeout(other._timeout)
{
	int	pipefd[2] = {-1, -1};

	if (!::pipe(pipefd))
	{
		_wakeup_rd_fd = pipefd[0];
		_wakeup_wr_fd = pipefd[1];
		if (!set_nonblock_fd(_wakeup_rd_fd)
			|| !set_nonblock_fd(_wakeup_wr_fd))
		{
			if (_wakeup_rd_fd > -1)
				(void)::close(_wakeup_rd_fd);
			if (_wakeup_wr_fd > -1)
				(void)::close(_wakeup_wr_fd);
			_wakeup_rd_fd = -1;
			_wakeup_wr_fd = -1;
		}
		else
			this->add(_wakeup_rd_fd, EVENT_READ, 0);
	}
}

EventLoop	&EventLoop::operator=(const EventLoop &other)
{
	if (this != &other)
	{
		if (_wakeup_rd_fd > -1)
			(void)::close(_wakeup_rd_fd);
		if (_wakeup_wr_fd > -1)
			(void)::close(_wakeup_wr_fd);
		_wakeup_rd_fd = -1;
		_wakeup_wr_fd = -1;

		_entries.clear();
		_scratch_fds.clear();
		_scratch_evs.clear();
		_should_stop = false;
		_timeout = other._timeout;

		int	pipefd[2] = {-1, -1};

		if (!::pipe(pipefd))
		{
			_wakeup_rd_fd = pipefd[0];
			_wakeup_wr_fd = pipefd[1];
			if (!set_nonblock_fd(_wakeup_rd_fd)
				|| !set_nonblock_fd(_wakeup_wr_fd))
			{
				if (_wakeup_rd_fd > -1)
					(void)::close(_wakeup_rd_fd);
				if (_wakeup_wr_fd > -1)
					(void)::close(_wakeup_wr_fd);
				_wakeup_rd_fd = -1;
				_wakeup_wr_fd = -1;
			}
			else
				this->add(_wakeup_rd_fd, EVENT_READ, 0);
		}
	}
	return (*this);
}

EventLoop::~EventLoop(void)
{
	std::map<int,Entry>::iterator	it = _entries.begin();
	while (it != _entries.end())
	{
		int	fd = it->first;
		bool owned = it->second._owned_by_eventloop;
		it++;
		/* close only if owned by event loop or if it's the wakeup pipe */
		if (owned)
			(void)::close(fd);
	}
	_entries.clear();

	if (_wakeup_rd_fd > -1)
		(void)::close(_wakeup_rd_fd);
	if (_wakeup_wr_fd > -1)
		(void)::close(_wakeup_wr_fd);
}

void	EventLoop::add(int fd, int events, IFdHandler *handler, bool take_ownership)
{
	Entry	e;

	e._fd = fd;
	e._events = events;
	e._handler = handler;
	e._last_active = getTime();
	e._owned_by_eventloop = take_ownership;
	this->_entries[fd] = e;
}

void	EventLoop::remove(int fd)
{
	std::map<int,Entry>::iterator	it = this->_entries.find(fd);
	if (it == this->_entries.end())
		return ;
	this->_entries.erase(it);
}

void	EventLoop::set_events(int fd, int events)
{
	std::map<int,Entry>::iterator	it = this->_entries.find(fd);
	if (it == this->_entries.end())
		return ;
	it->second._events = events;
	it->second._last_active = getTime();
}

void	EventLoop::set_timeout(unsigned timeout)
{
	this->_timeout = timeout;
}

void	EventLoop::run(void)
{
	while (!_should_stop)
		this->run_once(200);
}

void	EventLoop::stop(void)
{
	_should_stop = true;
	this->notify();
}

void	EventLoop::run_once(unsigned timeout)
{
	std::vector<struct pollfd>	pfds;

	pfds.reserve(this->_entries.size());
	this->_scratch_fds.clear();
	this->_scratch_evs.clear();
	this->_scratch_fds.reserve(this->_entries.size());
	this->_scratch_evs.reserve(this->_entries.size());

	std::map<int,Entry>::const_iterator	it = this->_entries.begin();
	while (it != this->_entries.end())
	{
		const Entry		&e = it->second;
		struct pollfd	p = {e._fd, 0, 0};

		if (e._events & EVENT_READ)
			p.events |= POLLIN;
		if (e._events & EVENT_WRITE)
			p.events |= POLLOUT;

		/* POLLERR and POLLHUP are delivered by poll() via revents */
		pfds.push_back(p);
		_scratch_fds.push_back(e._fd);
		_scratch_evs.push_back(e._events);
		it++;
	}
	(void)::poll(&pfds[0], pfds.size(), timeout);
	std::time_t	curr_time = getTime();

	/* If timeout set, enforce idle timeout so no hang up*/
	if (_timeout)
	{
		std::vector<int>	to_close;
		std::map<int,Entry>::const_iterator	it2 = _entries.begin();
		while (it2 != _entries.end())
		{
			const Entry	&e = it2->second;
			if (e._fd != _wakeup_rd_fd)
			{
				if (curr_time > e._last_active && _timeout < (curr_time - e._last_active))
					to_close.push_back(e._fd);
			}
			it2++;
		}

		std::vector<int>::const_iterator	it3 = to_close.begin();
		while (it3 != to_close.end())
		{
			int	fd = *it3;
			it3++;
			std::map<int,Entry>::iterator	it4 = _entries.find(fd);
			if (it4 != _entries.end())
			{
				IFdHandler	*handler = it4->second._handler;
				if (handler)
					handler->onHangup(fd);
				(void)::close(fd);
				_entries.erase(it4);
			}
		}
	}

	/* make a copy of FDs to avoid iterator invalidation if some FDs are
		engaged/disengaged during callback; also copy their events;
	*/
	std::vector<int>	ready_fd;
	std::vector<int>	ready_ev;
	std::size_t	i = 0;
	while (i < pfds.size())
	{
		if (pfds[i].revents)
		{
			ready_fd.push_back(pfds[i].fd);
			ready_ev.push_back(pfds[i].revents);
		}
		i++;
	}
	/* Dispatch to handlers */
	i = 0;
	while (i < ready_fd.size())
	{
		int	fd = ready_fd[i];
		int	re = ready_ev[i];
		i++;

		/* wakeup pipe read end */
		if (fd == _wakeup_rd_fd)
		{
			drain_pipe_once(_wakeup_rd_fd);
			continue;
		}

		std::map<int,Entry>::iterator	it2 = _entries.find(fd);
		if (it2 == _entries.end())
			continue;
		IFdHandler	*handler = it2->second._handler;
		/* Check error */
		if (((re & POLLERR) || (re & POLLNVAL)) && handler)
			handler->onError(fd);
		/* Check hangup */
		if ((re & POLLHUP) && handler)
			handler->onHangup(fd);
		/* Check readable */
		if (re & POLLIN)
		{
			if (handler)
				handler->onReadable(fd);
			it2 = _entries.find(fd);
			if (it2 != _entries.end())
				it2->second._last_active = curr_time;
		}
		/* Check writable */
		if (re & POLLOUT)
		{
			if (handler)
				handler->onWritable(fd);
			it2 = _entries.find(fd);
			if (it2 != _entries.end())
				it2->second._last_active = curr_time;
		}
	}
	/* Handlers respirate */
	std::map<int,Entry>::iterator	it2 = _entries.begin();
	while (it2 != _entries.end())
	{
		int	fd = it2->first;
		IFdHandler	*handler = it2->second._handler;
		it2++;
		if (handler)
			handler->onTick(fd);
	}
}

void	EventLoop::touch(int fd)
{
	std::map<int,Entry>::iterator	it = _entries.find(fd);
	if (it == _entries.end())
		return ;
	it->second._last_active = getTime();
}

void	EventLoop::notify(void)
{
	if (_wakeup_wr_fd > -1)
		(void)::write(_wakeup_wr_fd, "6", 1);
}

void	EventLoop::set_signal_wakup(int sig)
{
	g_wakeup = _wakeup_wr_fd;
	(void)::signal(sig, eventloop_signal_handler);
}
