/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:53:46 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:15:09 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DeleteRequest.hpp"
DeleteRequest::DeleteRequest(void){}
DeleteRequest::DeleteRequest(std::string s){(void)s;}
DeleteRequest::~DeleteRequest(void){}
DeleteRequest::DeleteRequest(const DeleteRequest &other){(void)other;}
DeleteRequest	&DeleteRequest::operator=(const DeleteRequest &other){(void)other;return (*this);}
bool	DeleteRequest::selfcheck(void) const
{
	return (true);
}
