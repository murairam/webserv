/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Process.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 14:42:57 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 15:25:19 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROCESS_HPP
# define PROCESS_HPP

# include "_headers.hpp"
# include "SysError.hpp"

/* Process handles fork, execve, waitpid and kill */
class	Process
{
	private:
		pid_t	_pid;

	public:
		Process(void);
		Process(const Process &other);
		Process	&operator=(const Process &other);
		~Process(void);

		pid_t	getpid(void) const;
		/* Fork it, returns pid to parent, set pid = 0 to child */
		pid_t	ft_fork(void);
		/* custom execve */
		static void	ft_execve(const std::string &path, char *const argv[], char *const envp[]);
		/* custom wait, waitpid */
		int	ft_wait(int option);
		static int	ft_waitpid(pid_t pid, int option);
		/* kill with signal */
		void	killsig(int sig) const;
		static void	signalset(int sig, void (*handler)(int));
};

#endif
