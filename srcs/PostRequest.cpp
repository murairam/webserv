/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:54:48 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 17:16:20 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostRequest.hpp"
bool	PostRequest::selfcheck(void) {return (true);}
PostRequest::PostRequest(void){}
PostRequest::~PostRequest(void){}
PostRequest::PostRequest(std::string s){(void)s;}
PostRequest::PostRequest(const PostRequest &other){(void)other;}
PostRequest	&PostRequest::operator=(const PostRequest &other){(void)other;return (*this);}
