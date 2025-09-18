/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directory.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 13:23:02 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 16:33:25 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Directory.hpp"

Directory::Directory(void): _dir(0), _path_cached() {}

Directory::Directory(std::string path): _dir(0), _path_cached()
{
	ft_opendir(path);
}

Directory::Directory(const Directory &other)
:_dir(0), _path_cached(other._path_cached)
{
	if (other._dir)
	{
		_dir = ::opendir(_path_cached.c_str());
		if (!_dir)
			throw SysError("opendir failed: " + _path_cached, errno);
	}
}

Directory	&Directory::operator=(const Directory &other)
{
	if (this != &other)
	{
		if (_dir)
			::closedir(_dir);
		_dir = 0;
		_path_cached = other._path_cached;
		if (other._dir)
		{
			_dir = ::opendir(_path_cached.c_str());
			if (!_dir)
				throw SysError("opendir failed: " + _path_cached, errno);
		}
	}
	return (*this);
}

Directory::~Directory(void)
{
	if (_dir)
		::closedir(_dir);
}
bool	Directory::isOpen(void) const
{
	return (_dir != 0);
}

void	Directory::ft_opendir(const std::string &path)
{
	if (_dir)
		::closedir(_dir);
	_dir = ::opendir(path.c_str());
	if (!_dir)
		throw SysError("opendir failed: " + path, errno);
	_path_cached = path;
}

void	Directory::ft_closedir(void)
{
	if (_dir)
		::closedir(_dir);
	_dir = 0;
}

std::string	Directory::nextEntry(void)
{
	struct dirent	*entry;

	if (_dir)
		throw SysError("readdir on closed DIR*", EBADF);
	errno = 0;
	entry = ::readdir(_dir);
	if (!entry)
	{
		if (errno)
			throw SysError("readdir failed", errno);
		return (std::string());
	}
	if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
		return (nextEntry());
	return (std::string(entry->d_name));
}
