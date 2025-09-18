#!/usr/bin/env python3
import re, pathlib, textwrap

ROOT = pathlib.Path(__file__).resolve().parents[1]
SECTIONS = {
    "python": ["*.py"],
    "c": ["*.h", "*.c"],
    "cpp": ["*.hpp"],
    "matlab": ["*.m"],
    "simulink": ["models/*.slx"],
}

def list_snippets():
    items = {}
    for section, patterns in SECTIONS.items():
        files = []
        for pat in patterns:
            files += sorted((ROOT / section).glob(pat))
        items[section] = [f.relative_to(ROOT).as_posix() for f in files]
    return items

def update_readme(items):
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    marker = r"(?s)(## Come navigare.*?)(?:\n## Indicizzazione|$)"
    # Non sovrascriviamo descrizione; solo aggiungiamo liste per cartella (idempotente).
    block = ["## Elenco snippet"]
    for k, files in items.items():
        if not files:
            continue
        block.append(f"\n### {k}\n")
        for f in files:
            block.append(f"- `{f}`")
    new = re.sub(marker, r"\1\n\n" + "\n".join(block) + "\n\n## Indicizzazione", readme)
    (ROOT / "README.md").write_text(new, encoding="utf-8")

if __name__ == "__main__":
    items = list_snippets()
    update_readme(items)
    print("README aggiornato.")
