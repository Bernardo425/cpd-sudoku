sudoku-serial: sudoku-serial.o
	gcc sudoku-serial.o -o sudoku-serial

sudoku-serial.o: sudoku-serial.c
	gcc -c sudoku-serial.c

clean:
	rm -f *.o main