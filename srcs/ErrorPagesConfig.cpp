/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPagesConfig.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:30:45 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 14:00:50 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPagesConfig.hpp"

ErrorPagesConfig::ErrorPagesConfig(void)
{
	_data.insert(std::map<int, std::string>::value_type(400, ERROR_PAGE_400));
	_data.insert(std::map<int, std::string>::value_type(401, ERROR_PAGE_401));
	_data.insert(std::map<int, std::string>::value_type(403, ERROR_PAGE_403));
	_data.insert(std::map<int, std::string>::value_type(404, ERROR_PAGE_404));
	_data.insert(std::map<int, std::string>::value_type(408, ERROR_PAGE_408));
	_data.insert(std::map<int, std::string>::value_type(429, ERROR_PAGE_429));
	_data.insert(std::map<int, std::string>::value_type(500, ERROR_PAGE_500));
	_data.insert(std::map<int, std::string>::value_type(502, ERROR_PAGE_502));
	_data.insert(std::map<int, std::string>::value_type(503, ERROR_PAGE_503));
	_data.insert(std::map<int, std::string>::value_type(504, ERROR_PAGE_504));
}

ErrorPagesConfig::ErrorPagesConfig(const ErrorPagesConfig &other)
:_data(other._data) {}

ErrorPagesConfig	&ErrorPagesConfig::operator=(const ErrorPagesConfig &other)
{
	if (this != &other)
		_data = other._data;
	return (*this);
}

ErrorPagesConfig::~ErrorPagesConfig(void) {}

/*	If that specific error code is not found, return 404 be default
*/
const std::string	&ErrorPagesConfig::getErrorPage(int n) const
{
	std::map<int, std::string>::const_iterator	it = _data.find(n);
	if (it == _data.end())
		return (_data.find(404)->second);
	return (it->second);
}
