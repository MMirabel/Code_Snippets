# C Snippets

Collezione di strutture dati leggere e pattern per sistemi embedded.

## Contenuti

- [`data_structures.c`](data_structures.c) - Stack, queue, linked list implementazioni leggere
- [`memory_utils.c`](memory_utils.c) - Gestione memoria sicura, pool allocator, operazioni string sicure

## Utilizzo

Ogni file contiene implementazioni complete e testabili. Compilare con:

```bash
gcc -std=c99 -Wall -Wextra -O2 file.c -o output
```

## Prerequisiti

- Compilatore C99 compatibile (GCC, Clang)
- Nessuna dipendenza esterna
- Testato su sistemi embedded ARM Cortex-M