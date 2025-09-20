/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalFDHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/20 15:19:57 by yanli             #+#    #+#             */
/*   Updated: 2025/09/20 15:39:09 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SignalFDHandler.hpp"

SignalFDHandler::SignalFDHandler(void)
:IFdHandler(), _sh(0), _cb(0), _context(0) {}

SignalFDHandler::~SignalFDHandler(void) {}

SignalFDHandler::SignalFDHandler(const SignalFDHandler &other)
:IFdHandler(), _sh(other._sh), _cb(other._cb), _context(other._context) {}

SignalFDHandler	&SignalFDHandler::operator=(const SignalFDHandler &other)
{
	if (this != &other)
	{
		_sh = other._sh;
		_cb = other._cb;
		_context = other._context;
	}
	return (*this);
}

SignalFDHandler::SignalFDHandler(SignalHandler *sh, ShutdownCallback cb, void *context)
:IFdHandler(), _sh(sh), _cb(cb), _context(context) {}

void	SignalFDHandler::onReadable(int fd)
{
	(void)fd;
	if (_sh && _sh->checkStatus() && _cb)
		_cb(_context);
}

void	SignalFDHandler::onWritable(int fd)
{
	(void)fd;
}

void	SignalFDHandler::onError(int fd)
{
	(void)fd;
}

void	SignalFDHandler::onHangup(int fd)
{
	(void)fd;
}

void	SignalFDHandler::onTick(int fd)
{
	(void)fd;
}
