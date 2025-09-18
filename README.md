# Snippets di Programmazione (Python, MATLAB/Simulink, C/C++)

Raccolta curata di snippet riutilizzabili, idiomatici e documentati. Ogni snippet è autosufficiente, con prerequisiti, complessità e test/usage minimo.

## Obiettivi
- **Qualità**: stile consistente, lint, formattazione automatica.
- **Portabilità**: evitare dipendenze superflue.
- **Didattica**: spiegazioni concise e link a risorse.

## Come navigare
- [`python/`](python/): utility generiche (decorator, I/O robusto, ecc.)
- [`matlab/`](matlab/): elaborazione segnali, utilità numeriche.
- [`simulink/`](simulink/): modelli `.slx` con README e screenshot.
- [`c/`](c/): strutture dati leggere e pattern embedded.
- [`cpp/`](cpp/): header-only idiomatici (RAII, traits, ecc.)

## Indicizzazione
Esegui `python tools/build_index.py` per aggiornare l'indice degli snippet in questo README.

## Indice degli Snippet
*Generato automaticamente con `python tools/build_index.py`*
**Totale snippet: 5**
### PYTHON
- [`Crea backup se file esiste e backup è richiesto`](python/file_utils.py) - Utility per I/O file robusto con gestione errori e backup. Dipendenze: pathlib, shutil (standard library) Complessità: O(n) dove n è (*O(n) dove n è la dimensione del file*) [Req: pathlib, shutil (standard library)]
- [`Potrebbe fallire occasionalmente`](python/decorators.py) - Decoratori Python utili per timing, retry e caching. Dipendenze: standard library Complessità: O(1) per applicazione, varia per esecuzione (*O(1) per applicazione, varia per esecuzione*) [Req: standard library]
### MATLAB
- [`% Signal Processing Utilities`](matlab/signal_processing.m) - % Signal Processing Utilities (*varia per funzione (specificata in ogni caso)*) [Req: Signal Processing Toolbox]
### C
- [`Data Structures`](c/data_structures.c) - define STACK_MAX_SIZE 128 (*O(1) per tutte le operazioni base*)
### CPP
- [`Smart Ptr Utils`](cpp/smart_ptr_utils.hpp) - ifndef SMART_PTR_UTILS_HPP (*O(1) per creazione/distruzione*)

## Licenza
[MIT](LICENSE)