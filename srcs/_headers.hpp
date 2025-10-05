/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _headers.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yanli <yanli@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 23:18:53 by yanli             #+#    #+#             */
/*   Updated: 2025/10/04 15:45:57 by yanli            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HEADERS_HPP
# define _HEADERS_HPP

# ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE
# endif

#ifdef __APPLE__
# include <sys/event.h>  /* for kqueue if we decide to use it for macOS */
# include <poll.h>
#endif

# if defined(__LINUX__) || defined(__linux__)
#  include <poll.h>
# endif

# include <stdexcept>
# include <vector>
# include <iostream>
# include <iomanip>
# include <ctime>
# include <sstream>
# include <map>
# include <string>
# include <algorithm>
# include <cctype>
# include <utility>
# include <fstream>
# include <cstring>
# include <cstdlib>
# include <cerrno>
# include <cstdio>
# include <limits>

# include <sys/types.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <signal.h>
# include <unistd.h>
# include <dirent.h>

# include "msg.hpp"

#endif
