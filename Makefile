# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: yanli <yanli@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/13 12:31:48 by yanli             #+#    #+#              #
#    Updated: 2025/09/16 09:21:56 by yanli            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= webserv

CXX				= c++

CXXFLAGS		= -Wall -Wextra -std=c++98 -pipe \
				-mtune=native -march=native \
				-Og -fno-inline -ggdb3 -fno-omit-frame-pointer \
				-fno-optimize-sibling-calls \
				-Wmissing-declarations \
				-fvar-tracking-assignments -Wuninitialized -Wnon-virtual-dtor \
				-Woverloaded-virtual \
				-Wshadow -Wnull-dereference -Wpointer-arith \
				-Wundef -Wredundant-decls \
				-Wno-duplicated-branches -Wduplicated-cond \
				-D_DEBUG \
				-fsanitize=address,undefined,leak,vptr,float-divide-by-zero,bounds,bounds-strict \
				-fno-sanitize-recover=all \
				-fsanitize-address-use-after-scope 

LDFLAGS			= -fsanitize=address,undefined,leak,vptr,float-divide-by-zero,bounds,bounds-strict \
				-fno-sanitize-recover=all \
				-fsanitize-address-use-after-scope 

SRCS_DIR		= ./srcs

SRCS_FILES		= main.cpp Endpoint.cpp LocationConfig.cpp ServerConfig.cpp \
				SysError.cpp FD.cpp Pipe.cpp Resolver.cpp Socket.cpp \
				Directory.cpp Process.cpp utility.cpp ConfigLoader.cpp

SRCS			= $(addprefix $(SRCS_DIR)/,$(SRCS_FILES))

OBJS			= $(SRCS:.cpp=.o)

DEPS			= $(OBJS:.o=.d)

.PHONY: all clean fclean re z

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) -I$(SRCS_DIR) $(OBJS) -o $(NAME)

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
