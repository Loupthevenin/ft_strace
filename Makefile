# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ltheveni <ltheveni@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/06 13:57:18 by ltheveni          #+#    #+#              #
#    Updated: 2025/08/04 11:34:25 by ltheveni         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Colors
_GREY	= \033[30m
_RED	= \033[31m
_GREEN	= \033[32m
_YELLOW	= \033[33m
_BLUE	= \033[34m
_PURPLE	= \033[35m
_CYAN	= \033[36m
_WHITE	= \033[37m
_END	= \033[0m

NAME = ft_strace
SRC_DIR = srcs/
OBJ_DIR = objs/

SYS_SCRIPT = generate_syscall_table.sh
OUT_C = srcs/syscall_names.c
OUT_H = includes/syscall_names.h

# Config
SHELL = /bin/bash
CC = gcc

INCLUDE = -I includes
CFLAGS = -Wall -Werror -Wextra
RM = rm -rf
DEBUG_FLAGS = -g3

# C program
SRCS = $(SRC_DIR)ft_strace.c \
	   $(SRC_DIR)utils.c \
	   $(SRC_DIR)tracer.c \
	   $(SRC_DIR)syscall_names.c
OBJS = $(OBJ_DIR)ft_strace.o \
	   $(OBJ_DIR)utils.o \
	   $(OBJ_DIR)tracer.o \
	   $(OBJ_DIR)syscall_names.o

# Recipe
all: $(NAME)

$(OUT_C) $(OUT_H): $(SYS_SCRIPT)
	./$(SYS_SCRIPT)

objs/syscall_names.o: $(OUT_C) $(OUT_H)

$(NAME): $(OBJS)
	@printf "$(_END)\nCompiled source files\n"
	@$(CC) $(CFLAGS) $(INCLUDE) $(OBJS) -o $@
	@printf "$(_GREEN)Finish compiling $(NAME)!\n"
	@printf "Try \"./$(NAME)\" to use$(_END)\n"

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	@printf "$(_GREEN)█$(_END)"

clean:
	@printf "$(_YELLOW)Removing object files ...$(_END)\n"
	@$(RM) $(OBJ_DIR)

fclean:
	@printf "$(_RED)Removing object files and program ...$(_END)\n"
	@$(RM) $(NAME) $(OBJ_DIR) $(OUT_C) $(OUT_H)

re: fclean all

debug: CFLAGS += -fsanitize=address $(DEBUG_FLAGS)
debug: re
	@printf "$(_BLUE)Debug build done$(_END)\n"

leak: CFLAGS += $(DEBUG_FLAGS)
leak: re
	@printf "$(_BLUE)Leak check build done$(_END)\n"

.PHONY: all clean fclean re libft debug leak
