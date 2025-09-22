/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalFDHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 15:19:36 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 15:27:38 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALFDHANDLER_HPP
# define SIGNALFDHANDLER_HPP

# include "_headers.hpp"
# include "SignalHandler.hpp"
# include "IFdHandler.hpp"

class	SignalFDHandler:public IFdHandler
{
	public:
		typedef void	(*ShutdownCallback)(void *_context);

	private:
		SignalHandler		*_sh;
		ShutdownCallback	_cb;
		void				*_context;
	
	public:
		SignalFDHandler(void);
		~SignalFDHandler(void);
		SignalFDHandler(const SignalFDHandler &other);
		SignalFDHandler	&operator=(const SignalFDHandler &other);
		SignalFDHandler(SignalHandler *sh, ShutdownCallback cb, void *context);

		void	onReadable(int fd);
		void	onWritable(int fd);
		void	onError(int fd);
		void	onHangup(int fd);
		void	onTick(int fd);
};

#endif
