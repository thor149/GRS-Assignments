# Makefile for GRS Programming Assignment 01
# Roll Number: MT25077
#
# This Makefile compiles both process-based (Program A) and thread-based (Program B) programs
# along with the worker functions.
#
# Usage:
#   make              - Build all programs
#   make all          - Build all programs
#   make program_a    - Build process-based program only
#   make program_b    - Build thread-based program only
#   make clean        - Remove all compiled binaries and temporary files
#   make run_part_c   - Build programs and run Part C measurement script
#   make run_part_d   - Build programs and run Part D measurement script
#   make plots        - Generate plots from Part D CSV data

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -std=c11
PTHREAD_FLAGS = -lpthread
MATH_FLAGS = -lm

# Target executables
TARGETS = program_a program_b

# Source files
PROGRAM_A_SRC = MT25077_Part_A_Program_A.c MT25077_Part_B_workers.c
PROGRAM_B_SRC = MT25077_Part_B_Program_B.c MT25077_Part_B_workers.c
WORKER_HEADER = MT25077_Part_B_workers.h

# Default target: build all programs
all: $(TARGETS)
	@echo ""
	@echo "==================================================================="
	@echo "Build completed successfully!"
	@echo "==================================================================="
	@echo "Programs built:"
	@echo "  - program_a (process-based using fork)"
	@echo "  - program_b (thread-based using pthread)"
	@echo ""
	@echo "Usage:"
	@echo "  ./program_a <cpu|mem|io> [num_processes]"
	@echo "  ./program_b <cpu|mem|io> [num_threads]"
	@echo ""
	@echo "Examples:"
	@echo "  ./program_a cpu           - Run with 2 processes, CPU worker"
	@echo "  ./program_b mem 4         - Run with 4 threads, Memory worker"
	@echo "==================================================================="

# Build Program A (process-based)
program_a: $(PROGRAM_A_SRC) $(WORKER_HEADER)
	@echo "Compiling Program A (process-based)..."
	$(CC) $(CFLAGS) -o program_a $(PROGRAM_A_SRC) $(MATH_FLAGS)
	@echo "Program A compiled successfully."

# Build Program B (thread-based)
program_b: $(PROGRAM_B_SRC) $(WORKER_HEADER)
	@echo "Compiling Program B (thread-based)..."
	$(CC) $(CFLAGS) -o program_b $(PROGRAM_B_SRC) $(PTHREAD_FLAGS) $(MATH_FLAGS)
	@echo "Program B compiled successfully."

# Clean: remove all compiled files and temporary files
clean:
	@echo "Cleaning up..."
	rm -f $(TARGETS)
	rm -f *.o
	rm -f /tmp/io_test_*.dat
	rm -f core
	@echo "Cleanup complete."

# Run Part C measurement script
run_part_c: all
	@echo ""
	@echo "==================================================================="
	@echo "Running Part C measurement script..."
	@echo "==================================================================="
	bash MT25077_Part_C_shell.sh

# Run Part D measurement script
run_part_d: all
	@echo ""
	@echo "==================================================================="
	@echo "Running Part D measurement script..."
	@echo "==================================================================="
	bash MT25077_Part_D_shell.sh

# Generate plots from Part D CSV data
plots: MT25077_Part_D_CSV.csv
	@echo ""
	@echo "==================================================================="
	@echo "Generating plots from Part D data..."
	@echo "==================================================================="
	python3 MT25077_Part_D_plot.py
	@echo "Plots generated successfully."

# Test: build and run quick tests
test: all
	@echo ""
	@echo "==================================================================="
	@echo "Running quick tests..."
	@echo "==================================================================="
	@echo "Testing Program A with CPU worker (2 processes)..."
	./program_a cpu
	@echo ""
	@echo "Testing Program B with CPU worker (2 threads)..."
	./program_b cpu
	@echo ""
	@echo "==================================================================="
	@echo "Tests completed!"
	@echo "==================================================================="

# Help target
help:
	@echo "==================================================================="
	@echo "Makefile for GRS Programming Assignment 01"
	@echo "Roll Number: MT25077"
	@echo "==================================================================="
	@echo "Available targets:"
	@echo "  make            - Build all programs (default)"
	@echo "  make all        - Build all programs"
	@echo "  make program_a  - Build process-based program only"
	@echo "  make program_b  - Build thread-based program only"
	@echo "  make clean      - Remove all compiled files"
	@echo "  make test       - Build and run quick tests"
	@echo "  make run_part_c - Build and run Part C measurements"
	@echo "  make run_part_d - Build and run Part D measurements"
	@echo "  make plots      - Generate plots from Part D data"
	@echo "  make help       - Show this help message"
	@echo "==================================================================="

# Phony targets (not actual files)
.PHONY: all clean run_part_c run_part_d plots test help
