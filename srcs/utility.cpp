/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:00 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 12:24:56 by yanli            ###   ########.fr       */
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
