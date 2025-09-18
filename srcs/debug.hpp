/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/17 20:25:42 by yanli             #+#    #+#             */
/*   Updated: 2025/09/18 15:11:30 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	DEBUG_HPP
# define DEBUG_HPP

# define GET_REQUEST "GET / HTTP/1.1\r\nHost: 127.0.0.1:30000\r\nTransfer-Encoding: chunked\r\nConnection: keep-alive\r\nsec-ch-ua: \"Chromium\";v=\"140\", \"Not=A?Brand\";v=\"24\", \"Google Chrome\";v=\"140\"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: \"Linux\"\r\nDNT: 1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en,zh-CN;q=0.9,zh;q=0.8,fr-FR;q=0.7,fr;q=0.6,zh-TW;q=0.5\r\nCookie: cook_left1=cook_right1; cook_left2=cook_right2\r\nContent-Length: 16\r\n\r\nBODYBODYBODYBODY\r\n"
# define POST_REQUEST "POST / HTTP/1.1\r\nHost: 127.0.0.1:30000\r\nConnection: keep-alive\r\nsec-ch-ua: \"Chromium\";v=\"140\", \"Not=A?Brand\";v=\"24\", \"Google Chrome\";v=\"140\"\r\nsec-ch-ua-mobile: ?0\r\nsec-ch-ua-platform: \"Linux\"\r\nDNT: 1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br, zstd\r\nAccept-Language: en,zh-CN;q=0.9,zh;q=0.8,fr-FR;q=0.7,fr;q=0.6,zh-TW;q=0.5\r\nCookie: cook_left1=cook_right1; cook_left2=cook_right2\r\nContent-Length: 16\r\n\r\nBODYBODYBODYBODY\r\n"
#endif
