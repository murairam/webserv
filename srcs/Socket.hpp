/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 12:42:28 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 23:33:32 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "_headers.hpp"
# include "SysError.hpp"
# include "FD.hpp"

class	Socket
{
	private:
		FD	_fd;
	
	public:
		Socket(void);
		Socket(int domain, int type, int protocol);
		Socket(int fd, bool take_ownership);
		Socket(const Socket &other);
		Socket	&operator=(const Socket &other);
		~Socket(void);

		int		getFD(void) const;
		bool	isValidFD(void) const;
		void	resetFD(int fd);
		int		releaseFD(void);
		void	closeFD(void);

		/* Configuration */
		void	setReuseAddr(bool enabled) const;
		void	setReusePort(bool enabled) const;
		void	setNonBlocking(bool enabled);
		/* Bind, Listen, Accept a socket */
		void	bindTo(const sockaddr *sa, socklen_t len) const;
		void	listenOn(int backlog) const;
		Socket	acceptOne(sockaddr_storage &peer_sa, socklen_t &peer_len) const;
		/* Connect to a socket */
		void	connectTo(const sockaddr *sa, socklen_t len) const;
		/* Input/Output */
		ssize_t	sendIO(const void *buf, size_t len, int flags) const;
		ssize_t	recvIO(void *buf, size_t len, int flags) const;
};

#endif
