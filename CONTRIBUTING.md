# Contributing

Thank you for helping improve this snippet library! Before opening a pull request, walk through the checklist below.

## Quick checklist
- [ ] Place the snippet under the correct top-level folder (`Python/`, `MATLAB/`, `Simulink/`, `C/`, `Cpp/`, `Arduino/`, `STM32/`, `tools/`).
- [ ] Add a short description header covering purpose, complexity (Big-O or qualitative), dependencies, and usage.
- [ ] Include an executable example or minimal `main()`/test harness demonstrating the snippet.
- [ ] Update companion README/docstrings with context and references when useful.
- [ ] Run the language-specific formatters and linters listed below.
- [ ] Run `python tools/build_index.py` when adding or removing snippets so the top-level index stays in sync.
- [ ] Ensure commit messages and the PR title are clear (e.g. `python: add debounce decorator`).

## Workflow
1. Start from the latest `main` branch.
2. Create a focused feature branch (`python/feature-name`, `bugfix/stm32-clock-config`, etc.).
3. Keep pull requests small and scoped to one topic; split unrelated changes into separate PRs.
4. Reference existing issues when applicable and describe testing/validation steps in the PR body.
5. If behaviour changes, update the snippet's README/docstring so users understand the new expectations.

## Snippet expectations
- **Documentation**: begin files with a brief comment block that summarises purpose, complexity, runtime/memory requirements, and a usage example. For Simulink, also include a short README plus a PNG screenshot in the same folder.
- **Dependencies**: list required dependencies/toolboxes in a `Requirements` section; prefer standard library or bundled functionality.
- **Usage / verification**: provide a minimal runnable example (`if __name__ == "__main__":` driver, MATLAB script, Arduino `setup/loop`, STM32 `main.c` with board notes).
- **File layout**: keep C/C++ headers in `include/` where needed, `.ino` sketches named after their folder, and Simulink models saved as `.slx` with any auxiliary scripts under version control.

## Style & tooling
- **Python**: format with `black`, lint with `ruff`, add type hints on non-trivial functions, and avoid unexpected side effects in module scope.
- **C**: run `clang-format`, keep `.c`/`.h` pairs consistent, avoid variable-length arrays, and document ownership around dynamic memory.
- **C++**: follow modern C++17 idioms, avoid `using namespace`, prefer `std::span`/`std::string_view`, and run `clang-format`.
- **MATLAB**: follow MathWorks naming conventions (camelCase functions, UpperCamelCase classes), avoid single-letter variables except indices, and address Code Analyzer warnings.
- **Simulink**: save models in the repository baseline release (R2023b unless noted), avoid custom blocks when a standard block exists, and store tunable parameters in accompanying `.m` scripts.
- **Arduino**: target default board profiles, include README wiring notes, and compile with the CLI (`arduino-cli compile ...`) if available.
- **STM32**: prefer HAL/LL APIs over direct register access unless required, document the MCU part and board, include `SystemClock_Config` details, and build with CubeIDE or `arm-none-eabi-gcc` before submitting.

## Automated checks
Run the relevant commands before pushing:

```
ruff check .
black --check .
clang-format --dry-run -Werror C/*.c C/*.h Cpp/*.hpp
python tools/build_index.py
```

Add any extra language-specific checks you run (e.g. `arduino-cli compile`, MATLAB unit tests) to your PR description and mention the outcome.

Thanks again for contributing!
