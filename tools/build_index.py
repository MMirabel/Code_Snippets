# /*********************************************************************************************
# PROJECT:          Code Snippets
# COMPONENT:        Tools
# FILE:             build_index.py
# AUTHOR:           Ing. Mirko Mirabella
#                   m.mirabella@neptunengineering.com
#                   www.neptunengineering.com
# REVISION:         v. 1.3
# DATE:             14/06/2024
# **********************************************************************************************/
"""Generate snippet indexes inside the repository READMEs."""

from __future__ import annotations

import pathlib
from collections import OrderedDict
from dataclasses import dataclass, field

ROOT = pathlib.Path(__file__).resolve().parents[1]

INDEX_MARKER_START = "<!-- snippet-index:start -->"
INDEX_MARKER_END = "<!-- snippet-index:end -->"


@dataclass
class TreeNode:
    """Represent a node inside the snippet tree."""

    children: dict[str, "TreeNode"] = field(default_factory=dict)
    files: set[str] = field(default_factory=set)


@dataclass(frozen=True)
class Section:
    label: str
    folder: str
    patterns: tuple[str, ...]
    readme_header: str | None = None
    readme_end_header: str = "## How to contribute"
    readme_index_header: str = "## Snippet index"


SECTIONS: tuple[Section, ...] = (
    Section("Python", "Python", ("**/*.py",), readme_header="# Python Snippets"),
    Section("C", "C", ("**/*.h", "**/*.c"), readme_header="# C Snippets"),
    Section("Cpp", "Cpp", ("**/*.hpp", "**/*.cpp"), readme_header="# C++ Snippets"),
    Section("MATLAB", "MATLAB", ("**/*.m",), readme_header="# MATLAB Snippets"),
    Section("Simulink", "Simulink", ("**/*.slx",), readme_header="# Simulink Snippets"),
    Section("Arduino", "Arduino", ("**/*.ino", "**/*.h", "**/*.cpp"), readme_header="# Arduino Snippets"),
    Section("STM32", "STM32", ("**/*.c", "**/*.h", "**/*.cpp"), readme_header="# STM32 Snippets"),
)

SECTION_BY_LABEL: dict[str, Section] = {section.label: section for section in SECTIONS}


def _insert_path(node: TreeNode, parts: list[str]) -> None:
    """Insert a path split into parts inside the tree."""

    if not parts:
        return
    head, *tail = parts
    if not tail:
        node.files.add(head)
        return
    child = node.children.setdefault(head, TreeNode())
    _insert_path(child, tail)


def _build_tree_structure(files: list[str], base_folder: str) -> TreeNode:
    """Build a hierarchical tree out of the collected file paths."""

    tree = TreeNode()
    base_parts = list(pathlib.PurePosixPath(base_folder).parts)
    for file_path in files:
        parts = list(pathlib.PurePosixPath(file_path).parts)
        if base_parts and parts[: len(base_parts)] == base_parts:
            parts = parts[len(base_parts) :]
        _insert_path(tree, parts)
    return tree


def _render_tree_lines(node: TreeNode, level: int) -> list[str]:
    """Render the hierarchical tree into Markdown bullet lines."""

    lines: list[str] = []
    indent = "  " * level
    for directory in sorted(node.children):
        lines.append(f"{indent}- `{directory}`")
        lines.extend(_render_tree_lines(node.children[directory], level + 1))
    for file_name in sorted(node.files):
        lines.append(f"{indent}- `{file_name}`")
    return lines

README_MARKERS = (
    {
        "start_header": "## How to navigate",
        "end_header": "## Indexing",
        "index_header": "## Snippet index",
    },
    {
        "start_header": "## Come navigare",
        "end_header": "## Indicizzazione",
        "index_header": "## Elenco snippet",
    },
)


def list_snippets() -> "OrderedDict[str, list[str]]":
    items: "OrderedDict[str, list[str]]" = OrderedDict()
    for section in SECTIONS:
        base = ROOT / section.folder
        collected: list[str] = []
        if base.exists():
            seen: set[str] = set()
            for pattern in section.patterns:
                for path in sorted(base.glob(pattern)):
                    if path.is_dir():
                        continue
                    rel = path.relative_to(ROOT).as_posix()
                    if rel in seen:
                        continue
                    seen.add(rel)
                    collected.append(rel)
        items[section.label] = collected
    return items


def _find_header_index(lines: list[str], header: str) -> int | None:
    target = header.strip()
    for idx, line in enumerate(lines):
        if line.strip() == target:
            return idx
    return None


def _strip_trailing_blank_lines(lines: list[str]) -> list[str]:
    result = list(lines)
    while result and not result[-1].strip():
        result.pop()
    return result


def _ensure_leading_blank_line(lines: list[str]) -> list[str]:
    if not lines:
        return [""]
    if lines[0].strip():
        return [""] + lines
    return lines


def _remove_existing_index(lines: list[str]) -> list[str]:
    result: list[str] = []
    skip = False
    for line in lines:
        stripped = line.strip()
        if stripped == INDEX_MARKER_START:
            skip = True
            while result and not result[-1].strip():
                result.pop()
            continue
        if stripped == INDEX_MARKER_END:
            skip = False
            continue
        if not skip:
            result.append(line)
    return result


def _render_global_index(items: "OrderedDict[str, list[str]]", index_header: str) -> list[str]:
    lines = [INDEX_MARKER_START, index_header]
    content: list[str] = []
    for label, files in items.items():
        if not files:
            continue
        if content:
            content.append("")
        content.append(f"### {label}")
        section = SECTION_BY_LABEL.get(label)
        base_folder = section.folder if section else ""
        tree = _build_tree_structure(files, base_folder)
        content.extend(_render_tree_lines(tree, 0))
    if content:
        lines.append("")
        lines.extend(content)
    else:
        lines.extend(["", "_No snippets found yet._"])
    lines.append(INDEX_MARKER_END)
    lines.append("")
    return lines


def _render_local_index(files: list[str], index_header: str, folder_path: pathlib.Path) -> list[str]:
    lines = [INDEX_MARKER_START, index_header, ""]
    if files:
        for file_path in files:
            relative = (ROOT / file_path).relative_to(folder_path).as_posix()
            lines.append(f"- `{relative}`")
    else:
        lines.append("_No snippets found yet._")
    lines.append(INDEX_MARKER_END)
    lines.append("")
    return lines


def _replace_section(
    lines: list[str],
    start_header: str,
    end_header: str,
    index_lines: list[str],
) -> list[str] | None:
    start_idx = _find_header_index(lines, start_header)
    if start_idx is None:
        return None

    end_idx = _find_header_index(lines, end_header)
    if end_idx is None:
        end_idx = len(lines)

    section = lines[start_idx + 1 : end_idx]
    section = _remove_existing_index(section)
    section = _strip_trailing_blank_lines(section)
    section = _ensure_leading_blank_line(section)

    new_section = list(section)
    if index_lines:
        if new_section and new_section[-1].strip():
            new_section.append("")
        new_section.extend(index_lines)

    return lines[: start_idx + 1] + new_section + lines[end_idx:]


def update_root_readme(items: "OrderedDict[str, list[str]]") -> None:
    readme_path = ROOT / "README.md"
    lines = readme_path.read_text(encoding="utf-8").splitlines()

    for marker in README_MARKERS:
        index_lines = _render_global_index(items, marker["index_header"])
        updated = _replace_section(
            lines,
            marker["start_header"],
            marker["end_header"],
            index_lines,
        )
        if updated is not None:
            readme_path.write_text("\n".join(updated) + "\n", encoding="utf-8")
            return

    raise RuntimeError("Unable to locate navigation section in README.md")


def update_section_readmes(items: "OrderedDict[str, list[str]]") -> None:
    for section in SECTIONS:
        if section.readme_header is None:
            continue
        readme_path = ROOT / section.folder / "README.md"
        if not readme_path.exists():
            continue
        lines = readme_path.read_text(encoding="utf-8").splitlines()
        files = items.get(section.label, [])
        index_lines = _render_local_index(files, section.readme_index_header, readme_path.parent)
        updated = _replace_section(
            lines,
            section.readme_header,
            section.readme_end_header,
            index_lines,
        )
        if updated is not None:
            readme_path.write_text("\n".join(updated) + "\n", encoding="utf-8")


def main() -> None:
    snippets = list_snippets()
    update_root_readme(snippets)
    update_section_readmes(snippets)
    print("README files updated.")


if __name__ == "__main__":
    main()
