CC=gcc
CFLAGS=-Wall -Iinclude

SRC=src/alloc_trace.c src/dimca_visual_allocator.c src/allocator_opt.s
MAIN_SRC=src/main_visual_sim.c

OBJ=$(SRC:.c=.o)
OBJ += src/allocator_opt.o
MAIN_OBJ=$(MAIN_SRC:.c=.o)

PYTHON=python3

run_simulator:
	$(PYTHON) src/simulator.py

run_visualizer:
	$(PYTHON) visualizer.py

visual_trace.o: src/alloc_trace.c
	$(CC) $(CFLAGS) -c $< -o $@

visual_allocator.o: src/dimca_visual_allocator.c
	$(CC) $(CFLAGS) -c $< -o $@

allocator_opt.o: src/allocator_opt.s
	$(CC) $(CFLAGS) -c $< -o $@

main_visual_sim.o: $(MAIN_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

main_advanced: main_visual_sim.o visual_trace.o visual_allocator.o allocator_opt.o
	$(CC) $(CFLAGS) main_visual_sim.o visual_trace.o visual_allocator.o allocator_opt.o -o main_advanced

all: visual_trace.o visual_allocator.o allocator_opt.o

clean:
	rm -f src/*.o main_visual_sim.o main_advanced

