CC = gcc
CFLAGS = -O2 -Wall -Wextra -pedantic
OMPFLAGS = -fopenmp

TARGETS = sudoku-serial sudoku-omp

.PHONY: all clean

all: $(TARGETS)

sudoku-serial: sudoku-serial.c
	$(CC) $(CFLAGS) $(OMPFLAGS) sudoku-serial.c -o sudoku-serial

sudoku-omp: sudoku-omp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) sudoku-omp.c -o sudoku-omp

clean:
	rm -f $(TARGETS)