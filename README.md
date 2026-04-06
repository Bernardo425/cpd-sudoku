# Sudoku Solver em C

Projeto com duas implementações do algoritmo Sudoku:

- [sudoku-serial.c](sudoku-serial.c): versão serial com backtracking.
- [sudoku-omp.c](sudoku-omp.c): versão paralela com OpenMP tasks.

O repositório também inclui:

- [sudoku_input.txt](sudoku_input.txt): ficheiro de entrada no formato pedido pelo trabalho.
- [RELATORIO.md](RELATORIO.md): relatório com medições e análise de desempenho.
- [Makefile](Makefile): compilação das duas versões.

## Requisitos

- GCC ou compatível.
- Suporte a OpenMP.
- `make`.

## Formato de entrada

O programa lê um ficheiro de texto com:

1. Uma primeira linha com `L`, onde `n = L * L`.
2. `n` linhas seguintes, cada uma com `n` inteiros separados por espaço.
3. Os valores devem estar no intervalo `[0, n]`.
4. O valor `0` representa uma célula vazia.

Exemplo para `L = 3`:

```text
3
5 3 0 0 7 0 0 0 0
6 0 0 1 9 5 0 0 0
0 9 8 0 0 0 0 6 0
8 0 0 0 6 0 0 0 3
4 0 0 8 0 3 0 0 1
7 0 0 0 2 0 0 0 6
0 6 0 0 0 0 2 8 0
0 0 0 4 1 9 0 0 5
0 0 0 0 8 0 0 7 9
```

## Formato de saída

O programa escreve a solução em `stdout`, no mesmo formato da matriz de entrada, mas sem zeros.

Se não existir solução, o programa escreve:

```text
Nenhuma solução
```

O tempo de execução é escrito em `stderr`, usando `omp_get_wtime()`.

## Compilação

Compilar tudo:

```bash
make
```

Compilar apenas uma versão específica:

```bash
make sudoku-serial
make sudoku-omp
```

Limpar binários gerados:

```bash
make clean
```

## Execução

Executar a versão serial:

```bash
./sudoku-serial sudoku_input.txt
```

Executar a versão OpenMP com número de threads definido pelo ambiente:

```bash
OMP_NUM_THREADS=4 ./sudoku-omp sudoku_input.txt
```

## Observações sobre desempenho

A versão OpenMP paraleliza a árvore de busca do backtracking com tasks, mas a escalabilidade depende fortemente da instância de entrada. Para o exemplo atual, o overhead de paralelização pode superar o ganho por concorrência, sobretudo em matrizes pequenas.

Mais detalhes experimentais estão em [RELATORIO.md](RELATORIO.md).