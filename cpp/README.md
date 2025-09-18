# C++ Snippets

Collezione di idiomi C++ moderni, header-only e pattern RAII.

## Contenuti

- [`smart_ptr_utils.hpp`](smart_ptr_utils.hpp) - Utility per smart pointers e RAII
- [`template_metaprogramming.hpp`](template_metaprogramming.hpp) - Type traits e template tricks
- [`concurrent_utils.hpp`](concurrent_utils.hpp) - Pattern per programmazione concorrente

## Utilizzo

Header-only libraries. Include direttamente nei tuoi progetti:

```cpp
#include "smart_ptr_utils.hpp"
```

Compilare con:
```bash
g++ -std=c++17 -Wall -Wextra -O2 example.cpp -o output
```

## Prerequisiti

- Compilatore C++17 o superiore
- Standard library
- Thread support per concurrent_utils.hpp