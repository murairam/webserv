/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 21:05:45 by yanli             #+#    #+#             */
/*   Updated: 2025/10/01 13:50:05 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTLOOP_HPP
# define EVENTLOOP_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "utility.hpp"

class	IFdHandler;

class	EventLoop
{
	private:
		struct Entry
		{
			int			_fd;
			int			_events;
			IFdHandler	*_handler;
			std::time_t	_last_active;
			bool		_is_listener;

		};

		std::map<int,Entry>	_entries;
		std::vector<int>	_scratch_fds;
		std::vector<int>	_scratch_evs;
		int					_wakeup_rd_fd;
		int					_wakeup_wr_fd;
		bool				_should_stop;
		unsigned			_timeout;
		
	public:
		EventLoop(void);
		~EventLoop(void);
		EventLoop(const EventLoop &other);
		EventLoop	&operator=(const EventLoop &other);

		/* Engage a FD (socket type) with a IFdHandler */
		void	add(int fd, int events, IFdHandler *handler, bool is_listener = false);

		/*	Engage a local FD (file local file creation, deletion, modification
		*/
		void	add(int fd, int events, std::string action, bool is_listener = false);
		/* disengage a FD */
		void	remove(int fd);
		/* change events for an engaged FD */
		void	set_events(int fd, int events);
		/* set the maximum waiting time to avoid hangup */
		void	set_timeout(unsigned timeout);
		/* start the loop */
		void	run(void);
		/* run only once (single poll-dispatch cycle) with a timeout */
		void	run_once(unsigned timeout);
		/* stop the loop when safely to do so */
		void	stop(void);
		/* wake the loop up from poll() */
		void	notify(void);
		/* use a custom signal handler that writes to the wakeup pipe */
		void	set_signal_wakup(int sig);
		/* mark a FD as active */
		void	touch(int fd);
};

#endif
