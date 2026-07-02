CC = clang
CFLAGS = -O3 -Wall -Wextra -Iinclude -fPIC

SRCS = src/redstonex_sim.c src/redstonex_obj.c src/redstonex_components.c
OBJS = $(SRCS:.c=.o)

LIB_NAME = libredstonex.so

all: test_redstonex $(LIB_NAME)

test_redstonex: tests/test.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(LIB_NAME): $(OBJS)
	$(CC) -shared $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o test_redstonex $(LIB_NAME)
