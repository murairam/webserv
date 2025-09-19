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

# ifndef HTTP_CODE_MSG
#  define HTTP_CODE_MSG
#  define HTTP100 "100 Continue"
#  define HTTP200 "200 OK"
#  define HTTP201 "201 Created"
#  define HTTP202 "202 Accepted"
#  define HTTP301 "301 Move Permanently"
#  define HTTP302 "302 Found"
#  define HTTP308 "308 Permanent Redirect"
#  define HTTP400 "400 Bad Request"
#  define HTTP401 "401 Unauthorized"
#  define HTTP403 "403 Forbidden"
#  define HTTP404 "404 Not Found"
#  define HTTP405 "405 Method Not Allowed"
#  define HTTP406 "406 Not Acceptable"
#  define HTTP408 "408 Request Timeout"
#  define HTTP409 "409 Conflict"
#  define HTTP411 "411 Length Required"
#  define HTTP412 "412 Precondition Failed"
#  define HTTP413 "413 Payload Too Large"
#  define HTTP431 "431 Request Header Fields Too Large"
#  define HTTP501 "501 Not Implemented"
#  define HTTP505 "505 HTTP Version Not Supported"
# endif /* HTTP_CODE_MSG */

# ifndef ERROR_MSG
#  define ERROR_MSG
#  define ERROR_MSG_INVALID_ENVP "WHAT'S WRONG WITH YOU ? PLAYING THAT ENV -I TRICK WITH YOUR DADDY ?"
# endif /* ERROR_MSG */

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
# endif /* ERROR_PAGE */

#endif /* MSG_HPP */
