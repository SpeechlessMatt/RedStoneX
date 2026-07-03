CC = clang
CFLAGS = -O3 -Wall -Wextra -Iinclude -fPIC

SRCS = src/redstonex_sim.c src/redstonex_obj.c src/redstonex_components.c
OBJS = $(SRCS:.c=.o)

LIB_NAME = libredstonex.so
BUILD_DIR = build

# 默认目标：创建构建目录，编译库，运行测试
all: $(BUILD_DIR) test $(LIB_NAME)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/torch_relay_test: tests/torch_relay_test.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/comparator_test: tests/comparator_test.c $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

test: $(BUILD_DIR)/torch_relay_test $(BUILD_DIR)/comparator_test
	@./$(BUILD_DIR)/torch_relay_test
	@./$(BUILD_DIR)/comparator_test

$(LIB_NAME): $(OBJS)
	$(CC) -shared $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf src/*.o $(BUILD_DIR) $(LIB_NAME)

.PHONY: all test clean
