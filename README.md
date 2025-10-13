# Programming Snippets 
(Python, MATLAB/Simulink, C/C++, Arduino, STM32)

Curated collection of reusable, idiomatic, and well-documented snippets. Each snippet is self-contained, with prerequisites, complexity, and minimal test/usage example.

## Goals
- **Quality**: consistent style, linting, automatic formatting.
- **Portability**: avoid unnecessary dependencies.
- **Educational**: concise explanations and links to resources.

## How to navigate

- [`Python/`](Python/): general utilities (decorators, robust I/O, etc.)
- [`MATLAB/`](MATLAB/): signal processing, numerical utilities.
- [`Simulink/`](Simulink/): `.slx` models with README and screenshots.
- [`C/`](C/): lightweight data structures and embedded patterns.
- [`Cpp/`](Cpp/): idiomatic header-only (RAII, traits, etc.)
- [`Arduino/`](Arduino/): rapid prototyping sketches and reusable libraries.
- [`STM32/`](STM32/): HAL/LL patterns, startup code, and board bring-up notes.

<!-- snippet-index:start -->
## Snippet index

### Python
- `File`
  - `RenameVideo.py`
- `GIT`
  - `update_gitignore.py`
- `Serial_COM`
  - `Serial_SaveData.py`
- `Time`
  - `time_count.py`

### Arduino
- `Sensors`
  - `ADXL345`
    - `ADXL345.ino`
- `Utility`
  - `OverflowMillis.ino`
  - `ScanI2C.ino`
<!-- snippet-index:end -->

## Indexing
Run `python tools/build_index.py` to update the snippet index in this README.

## License
[MIT](LICENSE)
