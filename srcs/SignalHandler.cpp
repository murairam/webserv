/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 14:58:10 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 23:06:42 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SignalHandler.hpp"

volatile sig_atomic_t	SignalHandler::_shutdown_flag = 0;
int						SignalHandler::_pipefd[2] = {-1, -1};

void	SignalHandler::_handler(int sig)
{
	_shutdown_flag = 1;
	if (_pipefd[1] > -1)
	{
		ssize_t	ignored = ::write(_pipefd[1], "1", 1);
		(void)ignored;
	}
	(void)sig;
}

bool	SignalHandler::_hook_all(void)
{
	size_t	i = 0;
	while (i < _signals.size())
	{
#ifdef	USE_SIGACTION
		struct sigaction	sa;
		sa.sa_flags = SA_SIGINFO | SA_RESTART;
		sa.sa_handler = &SignalHandler::_handler;
		if (::sigemptyset(&sa.sa_mask) || ::sigaction(_signals[i], &sa, 0))
			return (false);
#else
		if (::signal(_signals[i], &SignalHandler::_handler) == SIG_ERR)
			return (false);
#endif
		i++;
	}
	return (true);
}

void	SignalHandler::_unhook_all(void)
{
	size_t	i = 0;
	while (i < _signals.size())
	{
#ifdef	USE_SIGACTION
		struct sigaction	sa;
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		(void)::sigemptyset(&sa.sa_mask);
		(void)::sigaction(_signals[i], &sa, 0);
#else
		(void)::signal(_signals[i], SIG_DFL);
#endif
		i++;
	}
}

bool	SignalHandler::_install_handlers(void)
{
	if (_pipefd[0] != -1 || _pipefd[1] != -1)
		return (false);
	if (::pipe(_pipefd))
		return (false);
	if (!set_nonblock_fd(_pipefd[0], std::string("SignalHandler.cpp:73"))
		|| !set_nonblock_fd(_pipefd[1], std::string("SignalHandler.cpp:74"))
		|| !_hook_all())
	{
		(void)::close(_pipefd[0]);
		(void)::close(_pipefd[1]);
		_pipefd[0] = -1;
		_pipefd[1] = -1;
		return (false);
	}
	return (true);
}

SignalHandler::SignalHandler(void)
:_signals(), _installed(false) {}

SignalHandler::~SignalHandler(void)
{
	uninstall();
}

SignalHandler::SignalHandler(const SignalHandler &other)
:_signals(other._signals), _installed(false) {}

SignalHandler	&SignalHandler::operator=(const SignalHandler &other)
{
	if (this != &other)
	{
		if (_installed)
			uninstall();
		_signals = other._signals;
		_installed = false;
	}
	return (*this);
}

void	SignalHandler::addSignal(int sig)
{
	if (_installed)
		return ;
	_signals.push_back(sig);
}

bool	SignalHandler::install(void)
{
	if (_installed)
		return (true);
	_shutdown_flag = 0;
	if (!_install_handlers())
		return (false);
	_installed = true;
	return (true);
}

void	SignalHandler::uninstall(void)
{
	if (!_installed)
		return ;
	_unhook_all();
	if (_pipefd[0] > -1)
		(void)::close(_pipefd[0]);
	_pipefd[0] = -1;
	if (_pipefd[1] > -1)
		(void)::close(_pipefd[1]);
	_pipefd[1] = -1;
	_installed = false;
}

bool	SignalHandler::isInstalled(void) const
{
	return (_installed);
}

int	SignalHandler::getReadFD(void) const
{
	return (_pipefd[0]);
}

bool	SignalHandler::checkStatus(void)
{
	char	buf[3000];

	if (_pipefd[0] > -1)
	{
		while (1)
		{
			ssize_t	n = ::read(_pipefd[0], buf, sizeof(buf));
			if (n > 0)
				continue;
			if (!n)
				break;
			if (errno == EINTR)
				continue;
			break;
		}
	}
	return (_shutdown_flag != 0);
}

bool	SignalHandler::shouldShutdown(void) const
{
	return (_shutdown_flag != 0);
}

void	SignalHandler::unsetShutdown(void)
{
	_shutdown_flag = 0;
}
