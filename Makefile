# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mmiilpal <mmiilpal@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/13 12:31:48 by yanli             #+#    #+#              #
#    Updated: 2025/09/19 14:21:04 by mmiilpal         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			= webserv

CXX				= c++

# Detect platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	PLATFORM_FLAGS = -D__APPLE__
	# Add macOS specific linker flags if needed
	LDFLAGS =
else ifeq ($(UNAME_S),Linux)
	PLATFORM_FLAGS = -D__LINUX__
	LDFLAGS =
else
	PLATFORM_FLAGS =
	LDFLAGS =
endif

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 $(PLATFORM_FLAGS)

SRCS_DIR		= ./srcs

SRCS_FILES		= main.cpp Endpoint.cpp LocationConfig.cpp ServerConfig.cpp \
				SysError.cpp FD.cpp Pipe.cpp Resolver.cpp Socket.cpp \
				Directory.cpp Process.cpp utility.cpp ConfigLoader.cpp \
				Header.cpp GetRequest.cpp DeleteRequest.cpp PostRequest.cpp \
				CodePage.cpp timestring.cpp EventLoop.cpp IFdHandler.cpp \
				Listener.cpp Response.cpp

SRCS			= $(addprefix $(SRCS_DIR)/,$(SRCS_FILES))

OBJDIR = objs

OBJS = $(addprefix $(OBJDIR)/, $(SRCS_FILES:.cpp=.o))

DEPS = $(OBJS:.o=.d)

.PHONY: all clean fclean re z

all: $(OBJDIR) $(NAME)

$(NAME): $(OBJS)
	$(CXX) -I$(SRCS_DIR) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCS_DIR)/%.cpp
				$(CXX) $(CXXFLAGS) -I$(SRCS_DIR) -MMD -MP -c $< -o $@

$(OBJDIR):
		mkdir -p $(OBJDIR)

-include $(DEPS)

clean:
	rm -rf $(DEPS) $(OBJS) $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

z:
	$(MAKE) fclean
	$(MAKE) -j$(nproc)
	$(MAKE) clean
