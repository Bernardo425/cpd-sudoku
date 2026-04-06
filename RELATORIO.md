# Relatório da versão OpenMP do Sudoku

  

## 1. Contexto experimental

  

As medições abaixo foram feitas sobre a instância atual em [sudoku_input.txt](sudoku_input.txt), no formato do trabalho com `L = 3`, logo `n = 9`.

  

A medição do tempo foi feita com `omp_get_wtime()`, e o tempo total do algoritmo é impresso em `stderr`, conforme implementado em [sudoku-omp.c](sudoku-omp.c#L270) e [sudoku-serial.c](sudoku-serial.c#L169).

  

## 2. Tempos experimentais, speedup e eficiência

  

Os tempos foram obtidos executando a versão OpenMP com `OMP_NUM_THREADS = 1, 2, 4, 8`.

  

| Threads | Tempo (s) | Speedup vs. 1 thread | Eficiência |
|--------:|----------:|---------------------:|-----------:|
| 1       | 0.000182  | 1.000                | 1.000      |
| 2       | 0.000253  | 0.719                | 0.360      |
| 4       | 0.000328  | 0.555                | 0.139      |
| 8       | 0.000457  | 0.398                | 0.050      |

  

Definições usadas:

  

- `speedup(p) = T1 / Tp`

- `eficiência(p) = speedup(p) / p`

  

Onde `T1` é o tempo com 1 thread e `Tp` o tempo com `p` threads.

  

Leitura prática dos resultados:

  

- O desempenho piorou à medida que o número de threads aumentou.

- A melhoria esperada por paralelismo não apareceu porque o problema é muito pequeno e o custo de paralelização domina.

- As diferenças absolutas estão na ordem de microssegundos, portanto existe também ruído de medição.

  

## 3. Decomposição utilizada

  

A decomposição adotada foi por árvore de busca do backtracking:

  

- A rotina recursiva escolhe a próxima célula vazia com [find_empty](sudoku-omp.c#L127).

- Para essa célula, são gerados os candidatos válidos com [is_valid](sudoku-omp.c#L133).

- Quando a profundidade é menor que o limite configurado, cada candidato gera uma tarefa OpenMP independente em [sudoku-omp.c](sudoku-omp.c#L143).

  

Na prática, a paralelização acontece apenas nas primeiras camadas da árvore (`max_parallel_depth = 2` em [sudoku-omp.c#L180-L187]), e o restante da resolução segue de forma recursiva e sequencial dentro de cada tarefa.

  

## 4. Sincronização e preocupações

  

As principais preocupações de sincronização foram:

  

- Evitar condições de corrida ao registrar a primeira solução encontrada.

- Evitar que várias tarefas continuem a trabalhar desnecessariamente depois que uma solução já existe.

- Garantir que a solução copiada para o resultado final não seja corrompida por acessos concorrentes.

  

Isso foi tratado da seguinte forma:

  

- O flag `found` é consultado com `#pragma omp atomic read` em [sudoku-omp.c](sudoku-omp.c#L121) e [sudoku-omp.c](sudoku-omp.c#L168).

- A cópia da solução para `ctx->solution` é protegida por `#pragma omp critical(solution_publish)` em [sudoku-omp.c](sudoku-omp.c#L104).

- O `#pragma omp taskwait` em [sudoku-omp.c](sudoku-omp.c#L161) garante que as tarefas criadas naquele nível terminem antes de avançar.

  

Esses mecanismos resolvem a correção, mas também introduzem overhead, especialmente quando o problema é pequeno.

  

## 5. Balanceamento de carga

  

O balanceamento de carga foi realizado de forma dinâmica:

  

- As subárvores de busca são distribuídas como tarefas OpenMP.

- Cada tarefa explora um ramo diferente da árvore de backtracking.

- Como a quantidade de trabalho de cada ramo é imprevisível, o agendamento dinâmico ajuda a distribuir a carga entre threads.

  

Ao mesmo tempo, este tipo de problema tem granulação irregular:

  

- Alguns ramos terminam cedo.

- Outros aprofundam bastante a recursão.

- Há tarefas que fazem muito pouco trabalho, e o overhead de criação supera o benefício.

  

Por isso, embora a decomposição seja correta, ela não garante escalabilidade boa para instâncias pequenas.

  

## 6. Discussão dos resultados

  

Os resultados observados não corresponderam à expectativa de aceleração com mais threads.

  

As razões mais prováveis são:

  

- O problema atual é pequeno demais para amortizar o custo de criação e agendamento das tasks.

- Há cópias completas da grelha com `malloc` + `memcpy` por tarefa em [sudoku-omp.c](sudoku-omp.c#L146).

- O acesso ao flag global `found` e a secção crítica de publicação da solução criam contenção.

- O limite de paralelização (`max_parallel_depth = 2`) restringe o número de ramos úteis disponíveis para várias threads.

  

Em resumo, o código está correto e funcional, mas a implementação OpenMP usada aqui é mais adequada como base de paralelização do que como versão já otimizada para desempenho. Para instâncias maiores, ou com heurísticas melhores, a tendência é que o paralelismo passe a compensar melhor.

  

## 7. Conclusão

  

A versão OpenMP cumpre os requisitos funcionais do trabalho, usa `OMP_NUM_THREADS` sem sobrescrever o valor do ambiente e mede o tempo com `omp_get_wtime()`.

  

No conjunto de medições feito nesta entrega, o aumento do número de threads não reduziu o tempo de execução; pelo contrário, aumentou ligeiramente. Isso indica overhead de paralelização superior ao ganho de concorrência para a instância atual.

  

Se for necessário melhorar a escalabilidade, os próximos passos mais relevantes são:

  

- reduzir cópias de memória por tarefa;

- aplicar a heurística MRV para escolher a próxima célula;

- usar cancelamento mais agressivo após encontrar solução;

- aumentar a granularidade mínima das tarefas.