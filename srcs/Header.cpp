/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Header.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 23:24:01 by yanli             #+#    #+#             */
/*   Updated: 2025/09/17 20:45:57 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Header.hpp"

Header::Header(void)
:_request_method(0), _get(), _post(), _delete(), _should_reject(false) {}

Header::Header(const Header &other)
:_request_method(other._request_method), _get(other._get), _post(other._post),
_delete(other._delete), _should_reject(other._should_reject) {}

Header	&Header::operator=(const Header &other)
{
	if (this != &other)
	{
		_request_method = other._request_method;
		_get = other._get;
		_post = other._post;
		_delete = other._delete;
		_should_reject = other._should_reject;
	}
	return (*this);
}

Header::~Header(void) {}

Header::Header(std::string s)
:_request_method(0), _get(), _post(), _delete(), _should_reject(false)
{
	if (process(s))
		_should_reject = true;
}

bool	Header::process(const std::string &str)
{
	std::string			s = str;
	std::istringstream	iss(s);
	std::string			method_word;

	method_word.clear();
	if (!(iss>>method_word))
		return (1);
	if (method_word == "GET")
	{
		_request_method = GET_MASK;
		_get = GetRequest(str);
		return (_get.shouldReject());
	}
	else if (method_word == "POST")
	{
		_request_method = POST_MASK;
		_post = PostRequest(str);
		return (_post.shouldReject());
	}
	else if (method_word == "DELETE")
	{
		_request_method = DELETE_MASK;
		_delete = DeleteRequest(str);
		return (_delete.shouldReject());
	}
	return (0);
}

bool	Header::shouldReject(void) const
{
	return (_should_reject);
}
