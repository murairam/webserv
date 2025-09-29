/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 00:57:27 by yanli             #+#    #+#             */
/*   Updated: 2025/09/29 16:41:09 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IFdHandler.hpp"
#include "EventLoop.hpp"

static volatile sig_atomic_t	g_wakeup = -1;

#ifdef	_USE_EPOLL
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


	unsigned int	to_epoll_events(int events)
	{
		unsigned int	mask = EPOLLET | EPOLLERR | EPOLLHUP;
#if defined(EPOLLRDHUP)
		mask |= EPOLLRDHUP;
#endif

		if (events & EVENT_READ)
			mask |= EPOLLIN;
		if (events & EVENT_WRITE)
			mask |= EPOLLOUT;
		return (mask);
	}
	std::time_t	getTime(void)
	{
		return (std::time(0));
	}
}

EventLoop::EventLoop(void)
: _entries(), _scratch_fds(), _scratch_evs(), 
_epoll_fd(-1), _epoll_events(), _wakeup_rd_fd(-1),
_wakeup_wr_fd(-1), _should_stop(false), _timeout(0)
{
	int	pipefd[2] = {-1, -1};

	_epoll_fd = ::epoll_create(256);
	if (_epoll_fd < 0)
		throw SysError("EventLoop: epoll_create failed", errno);
	_epoll_events.reserve(64);

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
	{
		try
		{
			this->add(_wakeup_rd_fd, EVENT_READ, 0, false);
		}
		catch (...)
		{
			(void)::close(_wakeup_rd_fd);
			(void)::close(_wakeup_wr_fd);
			_wakeup_rd_fd = -1;
			_wakeup_wr_fd = -1;
			throw;
		}
	}
}

EventLoop::EventLoop(const EventLoop &other)
: _entries(), _scratch_fds(), _scratch_evs(),
_epoll_fd(-1), _epoll_events(), _wakeup_rd_fd(-1),
_wakeup_wr_fd(-1), _should_stop(false), _timeout(other._timeout)
{
	int	pipefd[2] = {-1, -1};

	_epoll_fd = ::epoll_create(256);
	if (_epoll_fd < 0)
		throw SysError("EventLoop: epoll_create", errno);
	_epoll_events.reserve(64);

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
		{
			try
			{
				this->add(_wakeup_rd_fd, EVENT_READ, 0, false);
			}
			catch (...)
			{
				(void)::close(_wakeup_rd_fd);
				(void)::close(_wakeup_wr_fd);
				_wakeup_rd_fd = -1;
				_wakeup_wr_fd = -1;
				throw;
			}
		}
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

		if (_epoll_fd > -1)
			(void)::close(_epoll_fd);
		_epoll_fd = -1;

		_entries.clear();
		_scratch_fds.clear();
		_scratch_evs.clear();
		_epoll_events.clear();
		_should_stop = false;
		_timeout = other._timeout;
		
		int	pipefd[2] = {-1, -1};

		_epoll_fd = ::epoll_create(256);
		if (_epoll_fd < 0)
			throw SysError("EventLoop: epoll_create", errno);
		_epoll_events.reserve(64);
		
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
			{
				try
				{
					this->add(_wakeup_rd_fd, EVENT_READ, 0, false);
				}
				catch (...)
				{
					(void)::close(_wakeup_rd_fd);
					(void)::close(_wakeup_wr_fd);
					_wakeup_rd_fd = -1;
					_wakeup_wr_fd = -1;
					throw;
				}
			}
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
	if (_epoll_fd > -1)
		(void)::close(_epoll_fd);
}

void	EventLoop::add(int fd, int events, IFdHandler *handler, bool is_listener)
{
	Entry				e;
	struct epoll_event	ev;

	std::memset(&ev, 0, sizeof(ev));
	ev.data.fd = fd;
	ev.events = to_epoll_events(events);
	if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		if (errno == EEXIST)
		{
			if (::epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0)
				throw SysError("EventLoop: epoll_ctl", errno);
		}
		else
			throw SysError("EventLoop: epoll_ctl", errno);
	}

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
	if (_epoll_fd > -1)
	{
		struct epoll_event	ev;
		std::memset(&ev, 0, sizeof(ev));
		(void)::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &ev);
	}
	this->_entries.erase(it);
}

void	EventLoop::set_events(int fd, int events)
{
	std::map<int,Entry>::iterator	it = this->_entries.find(fd);
	if (it == this->_entries.end())
		return ;
	it->second._events = events;
	it->second._last_active = getTime();
	if (_epoll_fd > -1)
	{
		struct epoll_event	ev;
		std::memset(&ev, 0, sizeof(ev));
		ev.data.fd = fd;
		ev.events = to_epoll_events(events);
		if (::epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0)
		{
			if (errno == ENOENT)
			{
				if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) < 0)
					throw SysError("EventLoop: epoll_ctl", errno);
			}
		else
			throw SysError("EventLoop: epoll_ctl", errno);
		}
	}
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
	if (_epoll_fd < 0)
		return;
	size_t	max_events = _entries.size();
	if (!max_events)
		max_events = 1;
	if (_epoll_events.size() < max_events)
		_epoll_events.resize(max_events);
	int	wait_timeout = static_cast<int>(timeout);
	if (wait_timeout < 0)
		wait_timeout = 0;
	int	nfds;
	while (1)
	{
		nfds = ::epoll_wait(_epoll_fd, &_epoll_events[0], static_cast<int>(_epoll_events.size()), wait_timeout);
		if (nfds >= 0)
			break;
		if (errno == EINTR)
		{
			nfds = 0;
			break;
		}
		throw SysError("EventLoop: epoll_wait", errno);
	}
	std::time_t	curr_time = getTime();
	if (_timeout)
	{
		std::vector<int>	to_close;
		std::map<int,Entry>::const_iterator	it = _entries.begin();
		while (it != _entries.end())
		{
			const Entry	&e = it->second;
			if (e._fd != _wakeup_rd_fd)
			{
				if (e._is_listener)
				{
					it++;
					continue;
				}
				if (curr_time > e._last_active && _timeout < (curr_time - e._last_active))
					to_close.push_back(e._fd);
			}
			it++;
		}
		std::vector<int>::const_iterator	close_it = to_close.begin();
		while (close_it != to_close.end())
		{
			int	fd = *close_it;
			close_it++;
			std::map<int,Entry>::iterator	entry = _entries.find(fd);
			if (entry != _entries.end())
			{
				IFdHandler	*handler = entry->second._handler;
				if (handler)
					handler->onHangup(fd);
				if (_epoll_fd > -1)
				{
					struct epoll_event	ev;
					std::memset(&ev, 0, sizeof(ev));
					(void)::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &ev);
				}
				(void)::close(fd);
				_entries.erase(entry);
			}
		}
	}
	_scratch_fds.clear();
	_scratch_evs.clear();
	_scratch_fds.reserve(static_cast<size_t>(nfds));
	_scratch_evs.reserve(static_cast<size_t>(nfds));
	int	i = 0;
	while (i < nfds)
	{
		_scratch_fds.push_back(_epoll_events[i].data.fd);
		_scratch_evs.push_back(static_cast<int>(_epoll_events[i].events));
		i++;
	}
	size_t	k = 0;
	while (k < _scratch_fds.size())
	{
		int	fd = _scratch_fds[k];
		int	re = _scratch_evs[k];
		k++;
		if (fd == _wakeup_rd_fd)
		{
			drain_pipe(_wakeup_rd_fd);
			continue;
		}
		std::map<int,Entry>::iterator	it = _entries.find(fd);
		if (it == _entries.end())
			continue;
		IFdHandler	*handler = it->second._handler;
		if ((re & EPOLLERR) && handler)
			handler->onError(fd);
		bool	has_hangup = ((re & EPOLLHUP) != 0);
#if defined(EPOLLRDHUP)
		if (re & EPOLLRDHUP)
			has_hangup = true;
#endif
		if (has_hangup && handler)
			handler->onHangup(fd);
		if (re & EPOLLIN)
		{
			if (handler)
				handler->onReadable(fd);
			it = _entries.find(fd);
			if (it != _entries.end())
				it->second._last_active = curr_time;
		}
		if (re & EPOLLOUT)
		{
			if (handler)
				handler->onWritable(fd);
			it = _entries.find(fd);
			if (it != _entries.end())
				it->second._last_active = curr_time;
		}
	}
	std::map<int,Entry>::iterator	it = _entries.begin();
	while (it != _entries.end())
	{
		int	fd = it->first;
		IFdHandler	*handler = it->second._handler;
		it++;
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

#else
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

void	EventLoop::set_timeout(unsigned timeout)
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
			drain_pipe(_wakeup_rd_fd);
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
		if (it2->second._handler)
			it2->second._handler->onTick(fd);
		it2++;
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
#endif
