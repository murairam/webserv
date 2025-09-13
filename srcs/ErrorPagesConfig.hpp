/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPagesConfig.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 13:30:55 by yanli             #+#    #+#             */
/*   Updated: 2025/09/13 13:57:00 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGESCONFIG_HPP
# define ERRORPAGESCONFIG_HPP

# ifndef ERROR_PAGE
#  define ERROR_PAGE
#  define ERROR_PAGE_400 "assets/error_pages/400.html"
#  define ERROR_PAGE_401 "assets/error_pages/401.html"
#  define ERROR_PAGE_403 "assets/error_pages/403.html"
#  define ERROR_PAGE_404 "assets/error_pages/404.html"
#  define ERROR_PAGE_408 "assets/error_pages/408.html"
#  define ERROR_PAGE_429 "assets/error_pages/429.html"
#  define ERROR_PAGE_500 "assets/error_pages/500.html"
#  define ERROR_PAGE_502 "assets/error_pages/502.html"
#  define ERROR_PAGE_503 "assets/error_pages/503.html"
#  define ERROR_PAGE_504 "assets/error_pages/504.html"
# endif

# include <string>
# include <map>


class	ErrorPagesConfig
{
	private:
		std::map<int, std::string>	_data;
	
	public:
		ErrorPagesConfig(void);
		ErrorPagesConfig(const ErrorPagesConfig &other);
		ErrorPagesConfig	&operator=(const ErrorPagesConfig &other);
		~ErrorPagesConfig(void);
		
		const std::string	&getErrorPage(int n) const;
};

#endif
