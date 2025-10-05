/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 15:34:00 by yanli             #+#    #+#             */
/*   Updated: 2025/10/04 12:23:56 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utility.hpp"

off_t	getFileSize(const std::string &path)
{
	struct stat	st;
	off_t		ret;

	try
	{
		if (::stat(path.c_str(), &st) < 0)
			throw SysError("\n---stat failed: " + path, errno);
		ret = st.st_size;
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
		ret = -1;
	}
	catch (...)
	{
		std::cerr<<"\n---Non-standard exception caught"<<std::endl;
		ret = -1;
	}
	return (ret);
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
		return (std::string("GET"));
	else if (method == POST_MASK)
		return (std::string("POST"));
	else if (method == DELETE_MASK)
		return (std::string("DELETE"));
	else if (method == OPTIONS_MASK)
		return (std::string("OPTIONS"));
	else if (method == PUT_MASK)
		return (std::string("PUT"));
	else if (method == CONNECT_MASK)
		return (std::string("CONNECT"));
	else if (method == HEAD_MASK)
		return (std::string("HEAD"));
	else
		return (std::string("INVALID"));
}

bool	set_nonblock_fd_nothrow(int fd)
{
	int	flags = ::fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return (false);
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return (false);
	return (true);
}

bool	set_cloexec_fd_nothrow(int fd)
{
	int	flags = ::fcntl(fd, F_GETFD, 0);
	if (flags < 0)
		return (false);
	if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		return (false);
	return (true);
}

bool	set_nonblock_fd(int fd, std::string position)
{
	bool	ret = true;

	try
	{
		int	flags = ::fcntl(fd, F_GETFL, 0);
		if (flags < 0)
			throw SysError("\n---fcntl(fd, F_GETFL, 0) failed (" + position + ")", errno);
		if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			throw SysError("\n---fcntl(fd, F_SETFL, flags | O_NONBLOCK, 0) failed (" + position + ")", errno);
	}
	catch (const std::exception &e)
	{
		std::cerr<<e.what()<<std::endl;
		ret = false;
	}
	catch (...)
	{
		std::cerr<<"\n---Non-standard exception caught"<<std::endl;
		ret = false;
	}
	return (ret);
}

/* trim removes space and tab */
std::string	trim(const std::string &input)
{
	std::string::size_type	first = input.find_first_not_of(" \t");
	if (first == std::string::npos)
		return (std::string());
	std::string::size_type	last = input.find_last_not_of(" \t");
	return (input.substr(first, last - first + 1));
}

std::string	toLower(const std::string &input)
{
	std::string	copy = input;
	std::string::iterator	it = copy.begin();
	while (it != copy.end())
	{
		*it = static_cast<char>(std::tolower(static_cast<unsigned char>(*it)));
		it++;
	}
	return (copy);
}

/* stripSlash removes slash, it's useful for path */
std::string	stripSlash(const std::string &str)
{
	if (str.size() < 2)
		return (str);
	std::string	ret(str);
	while (ret.size() > 1 && ret[ret.size() - 1] == '/')
		ret.erase(ret.size() - 1);
	return (ret.empty() ? std::string("/") : ret);
}

std::string	joinPath(const std::string &dir, const std::string &filename)
{
	if (dir.empty())
		return (filename);;
	if (dir[dir.size() - 1] == '/')
		return (dir + filename);
	return (dir + std::string("/") + filename);
}

/* This one is used while processing upload request */
bool	sanitizeFilename(const std::string &raw, std::string &name)
{
	name.clear();
	if (raw.empty())
	{
		name = raw;
		return (false);
	}
	std::string	base(raw);
	size_t		i = base.find_last_of("/\\");
	if (i != std::string::npos)
		base = base.substr(i + 1);
	std::string	ret;
	size_t	k = 0;
	while (k < base.size())
	{
		unsigned char	c = static_cast<unsigned char>(base[k]);
		if (std::isalnum(c) || c == '.' || c == '_' || c == '-')
			name.push_back(static_cast<char>(c));
		else if (c == ' ')
			name.push_back('_');
		k++;
	}
	if (name.empty())
		return (false);
	if (name.find("..") != std::string::npos)
	{
		name.clear();
		return (false);
	}
	return (true);
}

bool	extractFilename(const std::string &path, const std::string &locationPrefix, std::string &filename)
{
	filename.clear();
	if (path.empty())
		return (false);
	std::string	cleanPrefix = stripSlash(locationPrefix.empty() ? std::string("/") : locationPrefix);
	std::string	currentPath(path);
	if (cleanPrefix != "/")
	{
		if (currentPath.size() < cleanPrefix.size() || currentPath.compare(0, cleanPrefix.size(), cleanPrefix))
		{
			filename.clear();
			return (false);
		}
		currentPath.erase(0, cleanPrefix.size());
		if (!currentPath.empty() && currentPath[0] == '/')
			currentPath.erase(0, 1);
	}
	else if (!currentPath.empty() && currentPath[0] == '/')
		currentPath.erase(0, 1);
	if (currentPath.empty())
	{
		filename.clear();
		return (false);
	}
	size_t	i = currentPath.find_last_of('/');
	if (i != std::string::npos)	
		currentPath = currentPath.substr(i + 1);
	filename = currentPath;
	return (true);
}

/*	This one is used by matchLocationPath, the longest match
	would be chosen;
*/
bool	matchPath(const std::string &path, const std::string &prefix)
{
	if (prefix.empty())
		return (false);
	if (prefix == "/")
		return (true);
	if (path.size() < prefix.size())
		return (false);
	if (path.compare(0, prefix.size(), prefix))
		return (false);
	if (path.size() == prefix.size())
		return (true);
	return (path[prefix.size()] == '/');
}
