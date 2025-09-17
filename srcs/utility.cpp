/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:00 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 19:56:22 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utility.hpp"

off_t	getFileSize(const std::string &path)
{
	struct stat	st;
	int			r;

	r = ::stat(path.c_str(), &st);
	if (r)
		throw SysError("stat failed: " + path, errno);
	return (st.st_size);
}

bool	isDirectory(const std::string &path)
{
	struct stat	st;
	int			r;

	r = ::stat(path.c_str(), &st);
	if (r)
		return (false);
	return (S_ISDIR(st.st_mode) != 0);
}
/*
bool	isHeaderEnd(const std::string &s)
{
	if (s.size() < 1 || s[0] != '\r')
		return (false);
	return (true);
}

bool	isHeaderLineEnd(const std::string &s)
{
	if (s.size() < 1 || s[s.size() - 1] != '\r')
		return (false);
	return (true);
}*/

int	MethodTokenToMask(const std::string &method)
{
	if (method == "GET")
		return (GET_MASK);
	else if (method == "POST")
		return (POST_MASK);
	else if (method == "DELETE")
		return (DELETE_MASK);
	else if (method == "OPTIONS")
		return (OPTIONS_MASK);
	else if (method == "PUT")
		return (PUT_MASK);
	else if (method == "CONNECT")
		return (CONNECT_MASK);
	else if (method == "HEAD")
		return (HEAD_MASK);
	else
		return (-1);
}

std::string	MethodMaskToToken(int method)
{
	if (method == GET_MASK)
		return ("GET");
	else if (method == POST_MASK)
		return ("POST");
	else if (method == DELETE_MASK)
		return ("DELETE");
	else if (method == OPTIONS_MASK)
		return ("OPTIONS");
	else if (method == PUT_MASK)
		return ("PUT");
	else if (method == CONNECT_MASK)
		return ("CONNECT");
	else if (method == HEAD_MASK)
		return ("HEAD");
	else
		return ("INVALID");
}
