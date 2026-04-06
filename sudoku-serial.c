#include <stdio.h>
#include <stdlib.h>

static int is_valid(int *grid, int n, int l, int row, int col, int value) {
	int i;
	int box_row_start;
	int box_col_start;

	for (i = 0; i < n; i++) {
		if (grid[row * n + i] == value) {
			return 0;
		}
		if (grid[i * n + col] == value) {
			return 0;
		}
	}

	box_row_start = (row / l) * l;
	box_col_start = (col / l) * l;
	for (i = 0; i < l * l; i++) {
		int r = box_row_start + (i / l);
		int c = box_col_start + (i % l);
		if (grid[r * n + c] == value) {
			return 0;
		}
	}

	return 1;
}

static int find_empty(int *grid, int n, int *out_row, int *out_col) {
	int r;
	int c;

	for (r = 0; r < n; r++) {
		for (c = 0; c < n; c++) {
			if (grid[r * n + c] == 0) {
				*out_row = r;
				*out_col = c;
				return 1;
			}
		}
	}

	return 0;
}

static int solve_sudoku(int *grid, int n, int l) {
	int row;
	int col;
	int value;

	if (!find_empty(grid, n, &row, &col)) {
		return 1;
	}

	for (value = 1; value <= n; value++) {
		if (is_valid(grid, n, l, row, col, value)) {
			grid[row * n + col] = value;
			if (solve_sudoku(grid, n, l)) {
				return 1;
			}
			grid[row * n + col] = 0;
		}
	}

	return 0;
}

static int validate_initial_grid(int *grid, int n, int l) {
	int r;
	int c;

	for (r = 0; r < n; r++) {
		for (c = 0; c < n; c++) {
			int value = grid[r * n + c];
			if (value != 0) {
				grid[r * n + c] = 0;
				if (!is_valid(grid, n, l, r, c, value)) {
					grid[r * n + c] = value;
					return 0;
				}
				grid[r * n + c] = value;
			}
		}
	}

	return 1;
}

static void print_grid(int *grid, int n) {
	int r;
	int c;

	for (r = 0; r < n; r++) {
		for (c = 0; c < n; c++) {
			if (c > 0) {
				printf(" ");
			}
			printf("%d", grid[r * n + c]);
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]) {
	FILE *fp;
	int l;
	int n;
	int *grid;
	int r;
	int c;

	if (argc != 2) {
		fprintf(stderr, "Uso: %s <ficheiro_entrada>\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Erro ao abrir ficheiro: %s\n", argv[1]);
		return 1;
	}

	if (fscanf(fp, "%d", &l) != 1 || l < 2 || l > 9) {
		fprintf(stderr, "Formato de entrada invalido: L deve estar entre 2 e 9.\n");
		fclose(fp);
		return 1;
	}

	n = l * l;
	grid = (int *)malloc((size_t)n * (size_t)n * sizeof(int));
	if (grid == NULL) {
		fprintf(stderr, "Erro de memoria.\n");
		fclose(fp);
		return 1;
	}

	for (r = 0; r < n; r++) {
		for (c = 0; c < n; c++) {
			int value;
			if (fscanf(fp, "%d", &value) != 1) {
				fprintf(stderr, "Formato de entrada invalido: matriz incompleta.\n");
				free(grid);
				fclose(fp);
				return 1;
			}
			if (value < 0 || value > n) {
				fprintf(stderr, "Formato de entrada invalido: valor fora do intervalo [0,%d].\n", n);
				free(grid);
				fclose(fp);
				return 1;
			}
			grid[r * n + c] = value;
		}
	}

	fclose(fp);

	if (!validate_initial_grid(grid, n, l)) {
		printf("Nenhuma solução\n");
		free(grid);
		return 0;
	}

	if (solve_sudoku(grid, n, l)) {
		print_grid(grid, n);
	} else {
		printf("Nenhuma solução\n");
	}

	free(grid);
	return 0;
}
