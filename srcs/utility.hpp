/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:07 by yanli             #+#    #+#             */
/*   Updated: 2025/09/24 22:08:50 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILITY_HPP
# define UTILITY_HPP

# include "_headers.hpp"
# include "SysError.hpp"

/* returns the file size (utility.cpp) */
off_t	getFileSize(const std::string &path);

/* tells you if the path is a directory (utility.cpp) */
bool	isDirectory(const std::string &path);

/* 2 helper functions for methods bitmask <---> string conversion (utility.cpp) */
int	MethodTokenToMask(const std::string &method);
std::string	MethodMaskToToken(int method);

/* to set a FD as O_NONBLOCK, one throws (indicating position)
	one doesn't; (utility.cpp)
*/
bool	set_nonblock_fd_nothrow(int fd);
bool	set_nonblock_fd(int fd, std::string position = std::string());

/*	string processing helpers (utility.cpp)
*/
std::string	trim(const std::string &input);
std::string	toLower(const std::string &input);
#endif
