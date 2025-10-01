/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Directory.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 13:23:09 by yanli             #+#    #+#             */
/*   Updated: 2025/09/30 01:12:23 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DIRECTORY_HPP
# define DIRECTORY_HPP

# include "_headers.hpp"

/* Directory wraps opendir, readdir and closedir */
class	Directory
{
	private:
		DIR			*_dir;
		std::string	_path;
		int			_err_code;
		bool		_err_code_set;

	public:	
		Directory(void);
		Directory(std::string path);
		Directory(const Directory &other);
		Directory	&operator=(const Directory &other);
		~Directory(void);

		bool	isOpen(void) const;
		void	setPath(std::string path);
		void	ft_opendir(void);
		void	ft_closedir(void);
		int		getErrCode(void) const;
		bool	isErrCodeSet(void) const;

		std::string	nextEntry(void);
};
#endif
