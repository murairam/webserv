/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:37 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:10:55 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTREQUEST_HPP
# define POSTREQUEST_HPP

# include "_headers.hpp"

class	PostRequest
{
	private:

	public:
		bool	selfcheck(void);
		PostRequest(void);
		~PostRequest(void);
		PostRequest(std::string s);
		PostRequest(const PostRequest &other);
		PostRequest	&operator=(const PostRequest &other);
};

#endif
