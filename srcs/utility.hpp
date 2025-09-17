/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:07 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:33:51 by yanli            ###   ########.fr       */
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

/* tells you if it is `\r` */
bool	isHeaderEnd(const std::string &s);
bool	isHeaderLineEnd(const std::string &s);

/* 2 helper functions for methods bitmask <---> string conversion */
int	MethodTokenToMask(const std::string &method);
std::string	MethodMaskToToken(int method);

#endif
