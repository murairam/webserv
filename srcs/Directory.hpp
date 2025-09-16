/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directory.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 13:23:09 by yanli             #+#    #+#             */
/*   Updated: 2025/09/16 12:30:39 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTORY_HPP
# define DIRECTORY_HPP

# include "_headers.hpp"
# include "SysError.hpp"

/* Directory wraps opendir, readdir and closedir */
class	Directory
{
	private:
		DIR			*_dir;
		std::string	_path_cached;

	public:	
		Directory(void);
		Directory(std::string path);
		Directory(const Directory &other);
		Directory	&operator=(const Directory &other);
		~Directory(void);

		bool	isOpen(void) const;
		void	ft_opendir(const std::string &path);
		void	ft_closedir(void);

		std::string	nextEntry(void);
};
#endif
