/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timestring.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 16:17:18 by yanli             #+#    #+#             */
/*   Updated: 2025/09/30 13:56:40 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TIMESTRING_HPP
# define TIMESTRING_HPP

# include "_headers.hpp"

/* Generate a string representating the current time in HTTP-sytle */
std::string	getTimeString(void);
std::string	getUniqueTimeString(void);

/* Compare 2 strings of time in HTTP-style */
int	compareTimeString(const std::string &t1, const std::string &t2);

#endif
