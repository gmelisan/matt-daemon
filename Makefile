NAME = Matt_daemon

CC = @clang++
CPPFLAGS = -Wall -Wextra #-Werror

OBJ = $(addprefix ./src/, \
main.o Tintin_reporter.o Server.o \
)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ)
	@printf "\033[0;32mFile $@ was successfully created.\033[0m\n"

$(OBJ): \
./include/Tintin_reporter.h \
./include/Server.h \
./include/Matt_daemon.h

%.o: %.cpp
	$(CC) $(CPPFLAGS) -I./include -I$(LIBFT)/include -c -o $@ $<
	@printf "\033[1;34mCompiling ($(CPPFLAGS)) \033[0;36m$<\033[0m\n"

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all
