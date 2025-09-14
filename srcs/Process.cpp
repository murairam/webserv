/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 14:42:47 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 15:32:15 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Process.hpp"

Process::Process(void): _pid(-1) {}
Process::Process(const Process &other): _pid(other._pid) {}
Process	&Process::operator=(const Process &other)
{
	if (this != &other)
		_pid = other._pid;
	return (*this);
}

Process::~Process(void) {}

pid_t	Process::getpid(void) const
{
	return (_pid);
}

/* Fork it, returns pid to parent, set pid = 0 to child */
pid_t	Process::ft_fork(void)
{
	pid_t	temp;

	temp = fork();
	if (temp < 0)
		throw SysError("fork failed", errno);
	_pid = temp;
	return (temp);
}

/* custom execve */
void	Process::ft_execve(const std::string &path, char *const argv[], char *const envp[])
{
	if (execve(path.c_str(), argv, envp))
		throw SysError("execve failed: " + path, errno);
}

/* custom wait, waitpid */
int	Process::ft_wait(int option)
{
	int		status;
	pid_t	pid;

	if (_pid <= 0)
		throw SysError("wait on invalid pid", ECHILD);
	pid = waitpid(_pid, &status, option);
	if (pid < 0)
		throw SysError("waitpid failed", errno);
	return (status);
}

int	Process::ft_waitpid(pid_t pid, int option)
{
	int		status;
	pid_t	r;

	r = waitpid(pid, &status, option);
	if (r < 0)
		throw SysError("waitpid failed", errno);
	return (status);
}

/* kill with signal */
void	Process::killsig(int sig) const
{
	if (_pid <= 0)
		throw SysError("kill on invalid pid", ESRCH);
	if (kill(_pid, sig))
		throw SysError("kill failed", errno);
}

void	Process::signalset(int sig, void (*handler)(int))
{
	void	(*r)(int);
	
	r = signal(sig, handler);
	if (r == SIG_ERR)
		throw SysError("signal failed", errno);
}
