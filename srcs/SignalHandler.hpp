/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 14:25:29 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 23:07:23 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALHANDLER_HPP
# define SIGNALHANDLER_HPP

# include <vector>
# include <signal.h>
# include <unistd.h>
# include <fcntl.h>
# include <cerrno>
# include <string>
# include "utility.hpp"

class	SignalHandler
{
	private:
		static volatile sig_atomic_t	_shutdown_flag;
		static int						_pipefd[2];

		std::vector<int>				_signals;
		std::vector<int>				_ignored_signals;
		bool							_installed;
#ifdef	_USE_SIGACTION
		static void						_siginfo_handler(int sig, siginfo_t *info, void *context);
		static void						_siginfo_ignored_handler(int sig, siginfo_t *info, void *context);
#else
		static void						_handler(int sig);
		static void						_ignored_handler(int sig);
#endif
		bool							_hook_all(void);
		void							_unhook_all(void);
		bool							_install_handlers(void);
	public:
		SignalHandler(void);
		~SignalHandler(void);
		SignalHandler(const SignalHandler &other);
		SignalHandler	&operator=(const SignalHandler &other);

		void	addSignal(int sig);
		void	ignoreSignal(int sig);
		bool	install(void);
		void	uninstall(void);
		bool	isInstalled(void) const;
		int		getReadFD(void) const;
		bool	shouldShutdown(void) const;
		bool	checkStatus(void);
		void	unsetShutdown(void);
};

#endif
