CC = gcc
CFLAGS = -Wall -g


SHELL_SRCS = shell.c ast.c tokeniser.c imports.h utils.c
SHELL_OBJS = $(SHELL_SRCS:.c=.o)
SHELL_BIN = msh

COMMANDS = ls pwd echo touch rm cp mv cat

NEED_UTILS = ls touch rm cp mv echo pwd cat

.PHONY: all clean

all: $(SHELL_BIN) $(COMMANDS)

$(SHELL_BIN): $(SHELL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


$(COMMANDS): %: %.c
	@if echo "$(NEED_UTILS)" | grep -qw "$@"; then \
		$(CC) $(CFLAGS) $< utils.o -o $@; \
	else \
		$(CC) $(CFLAGS) $< -o $@; \
	fi

clean:
	rm -f *.o $(SHELL_BIN) $(COMMANDS)
