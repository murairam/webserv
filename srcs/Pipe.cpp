/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pipe.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 11:45:50 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 11:53:47 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Pipe.hpp"

Pipe::Pipe(void):_r(-1), _w(-1)
{
	int		pipefd[2];

	if (pipe(pipefd) < 0)
		throw SysError (std::string("pipe failed"), errno);
	_r.resetFD(pipefd[0]);
	_w.resetFD(pipefd[1]);
}

Pipe::Pipe(const Pipe &other)
:_r(other._r), _w(other._w) {}

Pipe	&Pipe::operator=(const Pipe &other)
{
	if (this != &other)
	{
		_r = other._r;
		_w = other._w;
	}
	return (*this);
}

Pipe::~Pipe(void) {}

FD	&Pipe::readEnd(void)
{
	return (_r);
}

FD	&Pipe::writeEnd(void)
{
	return (_w);
}

const FD	&Pipe::readEnd(void) const
{
	return (_r);
}

const FD	&Pipe::writeEnd(void) const
{
	return (_w);
}
