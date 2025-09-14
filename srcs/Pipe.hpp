/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pipe.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 11:45:57 by yanli             #+#    #+#             */
/*   Updated: 2025/09/14 11:48:53 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PIPE_HPP
# define PIPE_HPP

# include "_headers.hpp"
# include "FD.hpp"

class	Pipe
{
	private:
		FD		_r;
		FD		_w;

	public:
		Pipe(void);
		Pipe(const Pipe &other);
		Pipe	&operator=(const Pipe &other);
		~Pipe(void);

		FD	&readEnd(void);
		FD	&writeEnd(void);
		const FD	&readEnd(void) const;
		const FD	&writeEnd(void) const;
};

#endif
