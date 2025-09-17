/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:53:52 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:12:16 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	DELETEREQUEST_HPP
# define DELETEREQUEST_HPP

# include "_headers.hpp"

class	DeleteRequest
{
	private:
	
	public:
		DeleteRequest(void);
		DeleteRequest(std::string s);
		~DeleteRequest(void);
		DeleteRequest(const DeleteRequest &other);
		DeleteRequest	&operator=(const DeleteRequest &other);
		bool	selfcheck(void) const;
};
#endif
