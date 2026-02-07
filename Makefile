CC = gcc
CFLAGS = -std=c11 -O3 -Wall -Wextra -Iinclude
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

DEBUG_TARGET = $(BIN_DIR)/lob_sim_debug
DEBUG_CFLAGS = -std=c11 -O0 -g -Wall -Wextra -Iinclude
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(DEBUG_CFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

bench-run: $(BENCH_TARGET)
	./$(BENCH_TARGET)

format:
	clang-format -i src/**/*.c include/**/*.h

BENCH_TARGET = $(BIN_DIR)/lob_sim_bench
BENCH_SRCS = $(SRCS) src/bench/latency.c
BENCH_OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(BENCH_SRCS))

bench: $(BENCH_TARGET)

$(BENCH_TARGET): $(BENCH_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -O3 -DBENCHMARK -o $@ $^
