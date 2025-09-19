/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directory.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 13:23:02 by yanli             #+#    #+#             */
/*   Updated: 2025/09/19 20:55:03 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Directory.hpp"

Directory::Directory(void)
: _dir(0), _path(""), _err_code(0), _err_code_set(false) {}

Directory::Directory(std::string path)
: _dir(0), _path(path), _err_code(0), _err_code_set(false)
{}

Directory::Directory(const Directory &other)
:_dir(0), _path(other._path), _err_code(other._err_code), _err_code_set(other._err_code_set)
{}

Directory	&Directory::operator=(const Directory &other)
{
	if (this != &other)
	{
		if (_dir)
			(void)::closedir(_dir);
		_dir = 0;
		_path = other._path;
		_err_code = other._err_code;
		_err_code_set = other._err_code_set;
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

void	Directory::ft_opendir(void)
{
	if (this->isOpen())
		return ;
	_err_code = 0;
	_err_code_set = false;
	_dir = ::opendir(_path.c_str());
	if (!_dir)
	{
		_err_code = errno;
		_err_code_set = true;
		throw SysError("opendir failed on: " + _path + ", ", _err_code);
	}
}

void	Directory::setPath(std::string path)
{
	_path = path;
}

void	Directory::ft_closedir(void)
{
	if (!_dir)
		return;
	if (::closedir(_dir) == -1)
	{
		_err_code = errno;
		_err_code_set = true;
		throw SysError("closedir failed on: " + _path + ", ", _err_code);
	}
	_dir = 0;
}

std::string	Directory::nextEntry(void)
{
	struct dirent	*entry;

	if (!_dir)
		throw SysError("readdir on closed DIR*", EBADF);
	errno = 0;
	while (1)
	{
		_err_code = 0;
		_err_code_set = false;
		entry = ::readdir(_dir);
		_err_code = errno;
		if (!entry)
		{
			if (_err_code)
			{
				_err_code_set = true;
				throw SysError("readdir failed", _err_code);
				return (std::string());
			}
		}
		const char	*name = entry->d_name;
		if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
			continue;
		return (std::string(name));
	}
}

int	Directory::getErrCode(void) const
{
	return (_err_code);
}

bool	Directory::isErrCodeSet(void) const
{
	return (_err_code_set);
}
