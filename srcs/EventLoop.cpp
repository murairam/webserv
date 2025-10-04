/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:57:27 by yanli             #+#    #+#             */
/*   Updated: 2025/10/03 22:16:47 by yanli            ###   ########.fr       */
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
		{
			ssize_t	ignored = ::write(g_wakeup, "1", 1);
			(void)ignored;
		}
		(void)sig;
	}

void	drain_pipe(int fd)
	{
		char	buf[5000];
		while (1)
		{
			ssize_t	n = ::read(fd, buf, sizeof(buf));
			if (n > 0)
				continue;
			if (!n)
				break;
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN
#if defined(EWOULDBLOCK) && EAGAIN != EWOULDBLOCK
					|| errno == EWOULDBLOCK
#endif
			)
				break;
			break;
		}
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
		if (!set_nonblock_fd(_wakeup_rd_fd, std::string("EventLoop:54"))
			|| !set_nonblock_fd(_wakeup_wr_fd, std::string("EventLoop:55")))
		{
			(void)::close(_wakeup_rd_fd);
			(void)::close(_wakeup_wr_fd);
			_wakeup_rd_fd = -1;
			_wakeup_wr_fd = -1;
		}
	}
	/* If creation of self-pipe succeeded, engage the read end of this pipe */
	if (_wakeup_rd_fd > -1)
		this->add(_wakeup_rd_fd, EVENT_READ, 0, false);
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
		if (!set_nonblock_fd(_wakeup_rd_fd, std::string("EventLoop:78"))
			|| !set_nonblock_fd(_wakeup_wr_fd, std::string("EventLoop:79")))
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
		it++;
		if (fd != _wakeup_rd_fd)
			(void)::close(fd);
	}
	_entries.clear();
	
	if (_wakeup_rd_fd > -1)
		(void)::close(_wakeup_rd_fd);
	if (_wakeup_wr_fd > -1)
		(void)::close(_wakeup_wr_fd);
}

void	EventLoop::add(int fd, int events, IFdHandler *handler, bool is_listener)
{
	Entry	e;

	e._fd = fd;
	e._events = events;
	e._handler = handler;
	e._last_active = getTime();
	e._is_listener = is_listener;
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

void	EventLoop::set_timeout(int timeout)
{
	this->_timeout = timeout;
}

void	EventLoop::run(void)
{
	while (!_should_stop)
		this->run_once(_timeout);
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
	(void)::poll(pfds.size() ? &pfds[0] : reinterpret_cast<struct pollfd*>(0), pfds.size(), timeout);
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
				if (e._is_listener)
				{
					it2++;
					continue;
				}
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
				if (it4->second._handler)
					it4->second._handler->onHangup(fd);
				it4 = _entries.find(fd);
				if (it4 != _entries.end())
				{
					(void)::close(fd);
					_entries.erase(it4);
				}
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
			drain_pipe(_wakeup_rd_fd);
			continue;
		}

		std::map<int,Entry>::iterator	it2 = _entries.find(fd);
		if (it2 == _entries.end())
			continue;
		IFdHandler	*handler = it2->second._handler;
		bool	has_error = ((re & POLLERR) || (re & POLLNVAL));
		bool	has_hangup = (re & POLLHUP);
		/* Check error */
		if (has_error && handler)
			handler->onError(fd);
		/* Check hangup */
		if (has_hangup && handler)
			handler->onHangup(fd);
		if (has_error || has_hangup)
			continue;
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
	std::vector<int>	tick_fds;
	tick_fds.reserve(_entries.size());
	std::map<int,Entry>::iterator	it2 = _entries.begin();
	while (it2 != _entries.end())
	{
		tick_fds.push_back(it2->first);
		it2++;
	}
	std::vector<int>::const_iterator	tick_it = tick_fds.begin();
	while (tick_it != tick_fds.end())
	{
		int	fd = *tick_it;
		tick_it++;
		std::map<int,Entry>::iterator	e_it = _entries.find(fd);
		if (e_it != _entries.end() && e_it->second._handler)
			e_it->second._handler->onTick(fd);
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
	{
		ssize_t	ignored = ::write(_wakeup_wr_fd, "6", 1);
		(void)ignored;
	}
}

void	EventLoop::set_signal_wakup(int sig)
{
	g_wakeup = _wakeup_wr_fd;
	(void)::signal(sig, eventloop_signal_handler);
}
