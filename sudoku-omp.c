#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int n;
    int l;
    int size;
    int max_parallel_depth;
    int found;
    int *solution;
} solver_ctx_t;

static int is_valid(const int *grid, int n, int l, int row, int col, int value) {
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

static int find_empty(const int *grid, int n, int *out_row, int *out_col) {
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

static void print_grid(const int *grid, int n) {
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

static void publish_solution(solver_ctx_t *ctx, const int *grid) {
    int local_found;

    #pragma omp atomic read
    local_found = ctx->found;
    if (local_found) {
        return;
    }

    #pragma omp critical(solution_publish)
    {
        if (!ctx->found) {
            memcpy(ctx->solution, grid, (size_t)ctx->size * sizeof(int));
            ctx->found = 1;
        }
    }
}

static void solve_recursive(solver_ctx_t *ctx, int *grid, int depth) {
    int local_found;
    int row;
    int col;
    int value;
    int candidates_count;
    int candidates[81];

    #pragma omp atomic read
    local_found = ctx->found;
    if (local_found) {
        return;
    }

    if (!find_empty(grid, ctx->n, &row, &col)) {
        publish_solution(ctx, grid);
        return;
    }

    candidates_count = 0;
    for (value = 1; value <= ctx->n; value++) {
        if (is_valid(grid, ctx->n, ctx->l, row, col, value)) {
            candidates[candidates_count++] = value;
        }
    }

    if (candidates_count == 0) {
        return;
    }

    if (depth < ctx->max_parallel_depth && candidates_count > 1) {
        for (value = 0; value < candidates_count; value++) {
            int candidate = candidates[value];
            int *child_grid = (int *)malloc((size_t)ctx->size * sizeof(int));
            if (child_grid == NULL) {
                continue;
            }

            memcpy(child_grid, grid, (size_t)ctx->size * sizeof(int));
            child_grid[row * ctx->n + col] = candidate;

            // CORREÇÃO: child_grid deve ser firstprivate, não shared
            #pragma omp task default(none) firstprivate(depth, child_grid) shared(ctx)
            {
                solve_recursive(ctx, child_grid, depth + 1);
                free(child_grid);
            }
        }
        #pragma omp taskwait
        return;
    }

    for (value = 0; value < candidates_count; value++) {
        int candidate = candidates[value];

        #pragma omp atomic read
        local_found = ctx->found;
        if (local_found) {
            return;
        }

        grid[row * ctx->n + col] = candidate;
        solve_recursive(ctx, grid, depth + 1);
        grid[row * ctx->n + col] = 0;
    }
}

static int solve_sudoku_openmp(int *grid, int n, int l) {
    solver_ctx_t ctx;

    ctx.n = n;
    ctx.l = l;
    ctx.size = n * n;
    ctx.max_parallel_depth = 2;
    ctx.found = 0;
    ctx.solution = (int *)malloc((size_t)ctx.size * sizeof(int));
    if (ctx.solution == NULL) {
        return 0;
    }

    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            solve_recursive(&ctx, grid, 0);
        }
    }

    if (ctx.found) {
        memcpy(grid, ctx.solution, (size_t)ctx.size * sizeof(int));
    }

    free(ctx.solution);
    return ctx.found;
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int l;
    int n;
    int *grid;
    int r;
    int c;
    double exec_time;

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

    exec_time = -omp_get_wtime();
    if (solve_sudoku_openmp(grid, n, l)) {
        exec_time += omp_get_wtime();
        fprintf(stderr, "%.6fs\n", exec_time);
        print_grid(grid, n);
    } else {
        exec_time += omp_get_wtime();
        fprintf(stderr, "%.6fs\n", exec_time);
        printf("Nenhuma solução\n");
    }

    free(grid);
    return 0;
}