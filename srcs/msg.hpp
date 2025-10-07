/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   msg.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/14 11:41:03 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 21:18:38 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MSG_HPP
# define MSG_HPP

# ifndef METHODS_BITMASK
#  define METHODS_BITMASK
enum methods_bitmask
{
	GET_MASK		= (1 << 0),
	POST_MASK		= (1 << 1),
	DELETE_MASK		= (1 << 2),
	OPTIONS_MASK	= (1 << 3),
	PUT_MASK		= (1 << 4),
	CONNECT_MASK	= (1 << 5),
	HEAD_MASK		= (1 << 6)
};
# endif /* METHODS_BITMASK */

# ifndef EVENT_BITMASK
#  define EVENT_BITMASK
enum event_bitmask
{
	EVENT_NONE = 0,
	EVENT_READ = (1 << 0),
	EVENT_WRITE = (1 << 1),
	EVENT_ERR = (1 << 2)
};
# endif /* EVENT_BITMASK */

# ifndef ERROR_MSG
#  define ERROR_MSG
#  define ERROR_MSG_INVALID_ENVP "WHAT'S WRONG WITH YOU ? PLAYING THAT ENV -I TRICK WITH YOUR DADDY ?"
# endif /* ERROR_MSG */

#endif /* MSG_HPP */
