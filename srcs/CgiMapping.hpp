/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiMapping.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 14:03:29 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 18:48:15 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIMAPPING_HPP
# define CGIMAPPING_HPP

# include <string>
# include <map>
# include <stdexcept>

class	CgiMapping
{
	private:
		std::map<std::string, std::string>	_data;

	public:
		CgiMapping(void);
		CgiMapping(const CgiMapping &other);
		CgiMapping	&operator=(const CgiMapping &other);
		~CgiMapping(void);

		const std::string	&getCgiProcessor(std::string name) const;
};

#endif
