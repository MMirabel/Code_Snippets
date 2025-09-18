# Contributing

Thank you! Before opening a PR:

1. **Snippet structure**
   - A self-contained file (or .h/.c pair).
   - At the top: brief doc with purpose, complexity, requirements, usage example.
2. **Style**
   - Python: `ruff` + `black`.
   - C/C++: `clang-format`.
   - MATLAB: MathWorks conventions; no single-letter variables.
3. **Test/Usage**
   - Include an executable example block (or minimal `main()`).
4. **Naming**
   - snake_case for Python/MATLAB, kebab-case for files, PascalCase for C++ types.
5. **Commit**
   - Clear, concise messages. Example: `python: add debounce decorator`.

Run locally:

ruff check && black --check .
clang-format --dry-run -Werror c/.c c/.h cpp/*.hpp