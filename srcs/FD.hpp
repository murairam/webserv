/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FD.hpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:42:47 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 23:57:10 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FD_HPP
# define FD_HPP

# include "_headers.hpp"
# include "SysError.hpp"

class	FD
{
	private:
		int		_fd;

	public:
		/* FD initialized to -1 */
		FD(void);
		/* Acquir FD ownership */
		FD(int fd);
		/* dup if FD is valid */
		FD(const FD &other);
		FD	&operator=(const FD &other);
		/* close FD */
		~FD(void);

		bool	isValidFD(void) const;
		int		getFD(void) const;
		/* resetFD closes the old FD and take a new FD */
		void	resetFD(int fd);
		/* abandon FD ownership, return that FD */
		int		releaseFD(void);
		/* close that FD and set it to -1 */
		void	closeFD(void);

		static FD	openRO(const std::string &path);
		static FD	openWO(const std::string &path, bool create, mode_t mode);
		static FD	openRW(const std::string &path, bool create, mode_t mode);
		
		/* Check non-blocking flag */
		void	setNonBlockingFD(bool enabled);
		bool	isNonBlockingFD(void) const;
};

#endif
