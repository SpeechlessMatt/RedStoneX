CC = clang
CFLAGS = -O3 -Wall -Iinclude

SRCS = src/redstone_sim.c src/redstone_obj.c src/redstone_components.c
OBJS = $(SRCS:.c=.o)

all: test_redstone

test_redstone: tests/test.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o test_redstone
