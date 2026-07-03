# Copyright (C) 2026 Czy_4201b
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
