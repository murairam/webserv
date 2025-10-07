/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:07 by yanli             #+#    #+#             */
/*   Updated: 2025/10/04 00:48:18 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILITY_HPP
# define UTILITY_HPP

# include "_headers.hpp"
# include "SysError.hpp"

/* returns the file size */
off_t	getFileSize(const std::string &path);

/* tells you if the path is a directory */
bool	isDirectory(const std::string &path);

/*
 tells you if it is `\r` 
bool	isHeaderEnd(const std::string &s);
bool	isHeaderLineEnd(const std::string &s);*/

/* 2 helper functions for methods bitmask <---> string conversion */
int	MethodTokenToMask(const std::string &method);
std::string	MethodMaskToToken(int method);

/* to set a FD as O_NONBLOCK, one throws (indicating position)
	one doesn't
*/
bool	set_nonblock_fd_nothrow(int fd);
bool	set_nonblock_fd(int fd, std::string position = std::string());
bool	set_cloexec_fd_nothrow(int fd);

/*	string processing helpers
*/
std::string	trim(const std::string &input);
std::string	toLower(const std::string &input);
std::string	stripSlash(const std::string &str);
std::string	joinPath(const std::string &dir, const std::string &filename);
bool	sanitizeFilename(const std::string &raw, std::string &name);
bool	extractFilename(const std::string &path, const std::string &locationPrefix, std::string &filename);
bool	matchPath(const std::string	&path, const std::string &prefix);
std::string	expandPath(const std::string &path);
bool	set_cloexec_fd(int fd, std::string position = std::string());
std::string	getRealpath(void);
std::string	findNameByInode(const std::string &parentPath, ino_t childIno);
#endif
