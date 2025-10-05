# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yanli <yanli@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/13 12:31:48 by yanli             #+#    #+#              #
#    Updated: 2025/10/04 16:32:58 by yanli            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= webserv

CXX				= c++

CXXFLAGS		= -Wall -Wextra -Werror -std=c++98 -D_TESTER_VERSION

SRCS_DIR		= ./srcs

SRCS_FILES		= main.cpp Endpoint.cpp LocationConfig.cpp ServerConfig.cpp \
				SysError.cpp \
				Directory.cpp utility.cpp ConfigLoader.cpp \
				CodePage.cpp timestring.cpp EventLoop.cpp \
				IFdHandler.cpp Listener.cpp ListenerRegistry.cpp \
				Connection.cpp ConnectionManager.cpp \
				Response.cpp SignalHandler.cpp SignalFDHandler.cpp \
				HttpRequest.cpp HttpRequestParser.cpp CgiHandler.cpp

SRCS			= $(addprefix $(SRCS_DIR)/,$(SRCS_FILES))

OBJS			= $(SRCS:.cpp=.o)

DEPS			= $(OBJS:.o=.d)

.PHONY: all clean fclean re z cgi

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) -I$(SRCS_DIR) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCS_DIR) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	-rm -f $(DEPS) $(OBJS)

fclean: clean
	-rm -f $(NAME)

re: fclean all

cgi:
	-rm -f 1
	$(CXX) $(CXXFLAGS) -o 1 fuck_cgi_tester.cpp
	-rm -f fuck_cgi_tester.d
	-rm -f fuck_cgi_tester.o

z:
	-$(MAKE) fclean
	$(MAKE) -j$(nproc)
	-$(MAKE) clean

# Useful during evaluation: `strace -ff -e trace=close,socket,bind,socketpair,signal,fcntl,listen,accept,poll,epoll_create,epoll_wait,epoll_ctl,connect,open,read,send,recv -tt ./webserv ./assets/server_cfgs/GET_ONLY.cfg > strace_log.txt 2>&1`
