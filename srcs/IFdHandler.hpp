/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IFdHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 20:52:36 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 00:08:22 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IFDHANDLER_HPP
# define IFDHANDLER_HPP

class	IFdHandler
{
	public:
		IFdHandler(void);
		IFdHandler(const IFdHandler &other);
		IFdHandler	&operator=(const IFdHandler &other);
		virtual	~IFdHandler(void);

		/* POLLIN */
		virtual void	onReadable(int fd) = 0;
		/* POLLOUT */
		virtual void	onWritable(int fd) = 0;
		/* POLLERR */
		virtual void	onError(int fd) = 0;
		/* POLLHUP */
		virtual void	onHangup(int fd) = 0;
		virtual void	onTick(int fd) = 0;
};
#endif
