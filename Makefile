CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
SRCS = $(wildcard src/*.c src/*/*.c src/*/*/*.c)
BUILD_DIR = build
BIN_DIR = bin
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/lob_sim

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
