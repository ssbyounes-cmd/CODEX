NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -pthread


SRCS        = coders/main.c \
              coders/init_sim.c \
              coders/start_sim.c \
              coders/parse.c \
              coders/routine.c \
              coders/actions.c \
              coders/monitor.c \
              coders/cleanup.c \
              coders/utils.c

OBJS        = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c coders/codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re