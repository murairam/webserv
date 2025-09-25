# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/13 12:31:48 by yanli             #+#    #+#              #
#    Updated: 2025/09/25 11:37:09 by mmiilpal         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= webserv

CXX				= c++

CXXFLAGS		= -Wall -Wextra -Werror -std=c++98

SRCS_DIR		= ./srcs

SRCS_FILES		= main.cpp Endpoint.cpp LocationConfig.cpp ServerConfig.cpp \
				SysError.cpp FD.cpp Pipe.cpp Resolver.cpp \
				Directory.cpp Process.cpp utility.cpp ConfigLoader.cpp \
				CodePage.cpp timestring.cpp EventLoop.cpp \
				IFdHandler.cpp Listener.cpp ListenerRegistry.cpp \
				Connection.cpp ConnectionManager.cpp \
				Response.cpp SignalHandler.cpp SignalFDHandler.cpp Socket.cpp \
				HttpRequest.cpp HttpRequestParser.cpp

SRCS			= $(addprefix $(SRCS_DIR)/,$(SRCS_FILES))

OBJS			= $(SRCS:.cpp=.o)

DEPS			= $(OBJS:.o=.d)

.PHONY: all clean fclean re z

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) -I$(SRCS_DIR) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCS_DIR) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(DEPS) $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

z:
	$(MAKE) fclean
	$(MAKE) -j$(nproc)
	$(MAKE) clean
