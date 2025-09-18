#!/usr/bin/env python3
"""Generate the snippet index inside README.md."""

from __future__ import annotations

import pathlib
from collections import OrderedDict

ROOT = pathlib.Path(__file__).resolve().parents[1]

SECTIONS = (
    ("Python", "Python", ("**/*.py",)),
    ("C", "C", ("**/*.h", "**/*.c")),
    ("Cpp", "Cpp", ("**/*.hpp", "**/*.cpp")),
    ("MATLAB", "MATLAB", ("**/*.m",)),
    ("Simulink", "Simulink", ("**/*.slx",)),
)

README_MARKERS = OrderedDict(
    [
        (
            "## How to navigate",
            {
                "end_header": "## Indexing",
                "index_header": "## Snippet index",
            },
        ),
        (
            "## Come navigare",
            {
                "end_header": "## Indicizzazione",
                "index_header": "## Elenco snippet",
            },
        ),
    ]
)


def list_snippets() -> "OrderedDict[str, list[str]]":
    items: "OrderedDict[str, list[str]]" = OrderedDict()
    for label, folder, patterns in SECTIONS:
        base = ROOT / folder
        collected: list[str] = []
        if base.exists():
            seen: set[str] = set()
            for pattern in patterns:
                for path in sorted(base.glob(pattern)):
                    if path.is_dir():
                        continue
                    rel = path.relative_to(ROOT).as_posix()
                    if rel in seen:
                        continue
                    seen.add(rel)
                    collected.append(rel)
        items[label] = collected
    return items


def _find_header_index(lines: list[str], header: str) -> int | None:
    header = header.strip()
    for idx, line in enumerate(lines):
        if line.strip() == header:
            return idx
    return None


def _render_index(items: "OrderedDict[str, list[str]]", index_header: str) -> list[str]:
    lines = [index_header]
    for label, files in items.items():
        if not files:
            continue
        lines.append("")
        lines.append(f"### {label}")
        for file_path in files:
            lines.append(f"- `{file_path}`")
    lines.append("")
    return lines


def update_readme(items: "OrderedDict[str, list[str]]") -> None:
    readme_path = ROOT / "README.md"
    lines = readme_path.read_text(encoding="utf-8").splitlines()

    config = None
    start_index = None
    for header, cfg in README_MARKERS.items():
        idx = _find_header_index(lines, header)
        if idx is not None:
            start_index = idx
            config = cfg
            break

    if config is None or start_index is None:
        raise RuntimeError("Unable to locate navigation section in README.md")

    end_header = config["end_header"]
    end_index = _find_header_index(lines, end_header)
    if end_index is None:
        end_index = len(lines)
    index_lines = _render_index(items, config["index_header"])

    new_lines = lines[: start_index + 1]
    if new_lines and new_lines[-1].strip():
        new_lines.append("")
    new_lines.extend(index_lines)
    if end_index < len(lines) and index_lines and index_lines[-1].strip():
        new_lines.append("")
    new_lines.extend(lines[end_index:])

    readme_path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    snippets = list_snippets()
    update_readme(snippets)
    print("README updated.")
