CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread

# Targets
BASIC_EXE = dimca_basic_sim
GENERAL_EXE = dimca_general_sim

# Source files
BASIC_SRC = src/main_basic.c src/dimca_basic.c
GENERAL_SRC = src/main_general.c src/dimca_general.c

# Default target: build both
all: $(BASIC_EXE) $(GENERAL_EXE)

# Build basic executable
$(BASIC_EXE): $(BASIC_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Build general executable (with pthread linking)
$(GENERAL_EXE): $(GENERAL_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Run basic executable
run_basic: $(BASIC_EXE)
	./$(BASIC_EXE)

# Run general executable
run_general: $(GENERAL_EXE)
	./$(GENERAL_EXE)

# Clean executables
clean:
	rm -f $(BASIC_EXE) $(GENERAL_EXE)

.PHONY: all run_basic run_general clean
