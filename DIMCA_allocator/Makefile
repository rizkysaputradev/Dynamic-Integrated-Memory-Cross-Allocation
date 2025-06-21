CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
ASM_DIR = asm

# === Basic Allocator ===
BASIC_SRCS = $(SRC_DIR)/dimca_allocator.c $(SRC_DIR)/main.c
BASIC_TARGET = dimca_alloc

# === Advanced Allocator ===
ADVANCED_SRCS = $(SRC_DIR)/dimca_advanced_allocator.c $(SRC_DIR)/main_advanced.c
ADVANCED_TARGET = dimca_advanced

# === Benchmarks ===
TEST_BASIC = $(TEST_DIR)/benchmark.c
TEST_ADVANCED = $(TEST_DIR)/benchmark_advanced.c

BASIC_TEST_TARGET = dimca_benchmark
ADVANCED_TEST_TARGET = dimca_advanced_benchmark

# === Assembly Support ===
ASM_OBJS = $(ASM_DIR)/allocator_opt.o

# === Build All ===
all: $(BASIC_TARGET) $(ADVANCED_TARGET)

# === Build Rules ===
$(BASIC_TARGET): $(BASIC_SRCS) $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ADVANCED_TARGET): $(ADVANCED_SRCS) $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BASIC_TEST_TARGET): $(SRC_DIR)/dimca_allocator.c $(TEST_BASIC) $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(ADVANCED_TEST_TARGET): $(SRC_DIR)/dimca_advanced_allocator.c $(TEST_ADVANCED) $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# === Assembly Object Rule ===
$(ASM_DIR)/%.o: $(ASM_DIR)/%.s
	$(CC) -c $< -o $@

# === Run Rules ===
run: $(BASIC_TARGET)
	./$(BASIC_TARGET)

run-advanced: $(ADVANCED_TARGET)
	./$(ADVANCED_TARGET)

run-test: $(BASIC_TEST_TARGET)
	./$(BASIC_TEST_TARGET)

run-advanced-test: $(ADVANCED_TEST_TARGET)
	./$(ADVANCED_TEST_TARGET)

# === Clean ===
clean:
	rm -f $(BASIC_TARGET) $(ADVANCED_TARGET) $(BASIC_TEST_TARGET) $(ADVANCED_TEST_TARGET) $(ASM_OBJS)