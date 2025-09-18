# Contributi

Grazie! Prima di aprire una PR:

1. **Struttura snippet**
   - Un file (o coppia .h/.c) autosufficiente.
   - In testa: breve doc con scopo, complessità, requisiti, esempio d’uso.
2. **Stile**
   - Python: `ruff` + `black`.
   - C/C++: `clang-format`.
   - MATLAB: convenzioni MathWorks; niente variabili 1-lettera.
3. **Test/Usage**
   - Inserire un blocco d’esempio eseguibile (o `main()` minimale).
4. **Naming**
   - snake_case per Python/MATLAB, kebab-case per file, PascalCase per tipi C++.
5. **Commit**
   - Messaggi chiari, brevi. Esempio: `python: add debounce decorator`.

Esegui localmente:

ruff check && black --check .
clang-format --dry-run -Werror c/.c c/.h cpp/*.hpp