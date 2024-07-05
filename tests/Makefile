CC := gcc
CFLAGS := 																																		\
	-O2 -g -DDEBUG=1 																														\
	-Wall -Werror -Wextra 																											\
	-fsanitize=undefined -fno-sanitize-recover=undefined


.PHONY: all
all:
	@$(CC) $(CFLAGS) $(LDFLAGS) -o test-runner *.c
	@./test-runner
