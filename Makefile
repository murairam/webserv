# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yanli <yanli@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/13 12:31:48 by yanli             #+#    #+#              #
#    Updated: 2025/09/14 00:26:59 by yanli            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= webserv

CXX				= c++

CXXFLAGS		= -Wall -Wextra -Werror -std=c++98 

SRCS_DIR		= ./srcs

SRCS			= $(addprefix $(SRCS_DIR)/, main.cpp, Endpoint.cpp, ErrorPagesconfig.cpp, \
					LocationConfig.cpp, ServerConfig.cpp, CgiMapping.cpp, SysError.cpp \
					ServerTable.cpp, GlobalConfig.cpp, FD.cpp)

OBJS			= $(SRCS:.cpp=.o)

DEPS			= $(OBJS:.o=.d)

.PHONY: all clean fclean re z

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) -I$(SRCS_DIR) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(SRCS_DIR) -MMD -MP -c $< -c $@

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
