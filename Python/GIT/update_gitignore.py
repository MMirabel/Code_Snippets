# /*********************************************************************************************
# PROJECT:          Code Snippets
# COMPONENT:        Python/GIT
# FILE:             update_gitignore.py
# AUTHOR:           Ing. Mirko Mirabella
#                   m.mirabella@neptunengineering.com
#                   www.neptunengineering.com
# REVISION:         v. 1.0
# DATE:             13/10/2025
# **********************************************************************************************/
"""Utility to keep the repository .gitignore file up to date.

The module exposes reusable helpers that append a path to the ignore list,
remove it from Git tracking and optionally create a descriptive commit. It is
designed for interactive command-line usage while remaining importable.

Run the code into a Git repository to add a path to .gitignore:"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Iterable


def ensure_git_repository(repo_root: Path) -> Path:
    """Ensure that the provided directory contains a Git repository."""
    git_dir = repo_root / ".git"
    if not git_dir.exists():
        raise RuntimeError("Errore: questa cartella non è un repository Git.")
    return repo_root


def normalise_ignore_path(path_to_ignore: str, repo_root: Path) -> str:
    """Return the POSIX relative path for the ignore target."""
    raw_path = Path(path_to_ignore).expanduser()
    try:
        relative_path = raw_path.relative_to(repo_root)
    except ValueError:
        relative_path = raw_path
    return relative_path.as_posix()


def read_gitignore_lines(gitignore_path: Path) -> list[str]:
    """Read the .gitignore file and return its lines without trailing breaks."""
    if not gitignore_path.exists():
        return []
    return gitignore_path.read_text(encoding="utf-8").splitlines()


def append_to_gitignore(gitignore_path: Path, entry: str) -> bool:
    """Append the entry to .gitignore if not already present."""
    lines = read_gitignore_lines(gitignore_path)
    if entry in lines:
        print(f"'{entry}' è già presente in .gitignore.")
        return False

    gitignore_path.parent.mkdir(parents=True, exist_ok=True)
    newline_needed = bool(lines and lines[-1].strip())

    with gitignore_path.open("a", encoding="utf-8") as handle:
        if newline_needed:
            handle.write("\n")
        handle.write(f"{entry}\n")
    print(f"Aggiunto '{entry}' a .gitignore.")
    return True


def run_git_command(args: Iterable[str]) -> subprocess.CompletedProcess[str]:
    """Execute a Git command and propagate detailed errors."""
    try:
        return subprocess.run(["git", *args], check=True, text=True, capture_output=True)
    except subprocess.CalledProcessError as exc:
        message = exc.stderr.strip() or exc.stdout.strip() or str(exc)
        raise RuntimeError(f"Errore durante l'esecuzione di 'git {' '.join(args)}': {message}") from exc


def remove_from_tracking(entry: str) -> None:
    """Remove the entry from Git tracking when necessary."""
    result = subprocess.run(
        ["git", "ls-files", "--error-unmatch", entry],
        text=True,
        capture_output=True,
    )
    if result.returncode != 0:
        print(f"'{entry}' non è attualmente tracciato da Git: nessuna rimozione necessaria.")
        return

    run_git_command(["rm", "-r", "--cached", entry])
    print(f"Rimosso '{entry}' dal tracking Git.")


def commit_changes(entry: str) -> None:
    """Create a commit that documents the ignore update."""
    run_git_command(["add", ".gitignore"])
    run_git_command(["commit", "-m", f"Ignora '{entry}' via .gitignore"])
    print("Commit eseguito con successo.")


def update_gitignore(path_to_ignore: str, repo_root: Path | None = None) -> None:
    """Add the desired path to .gitignore and remove it from tracking."""
    root = ensure_git_repository(repo_root or Path.cwd())
    gitignore_path = root / ".gitignore"
    entry = normalise_ignore_path(path_to_ignore, root)

    try:
        added = append_to_gitignore(gitignore_path, entry)
        remove_from_tracking(entry)
        if added:
            commit_changes(entry)
        else:
            print("Nessuna modifica al file .gitignore: commit saltato.")
    except RuntimeError as error:
        print(error)
        sys.exit(1)


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    """Parse command-line arguments for the module."""
    parser = argparse.ArgumentParser(
        description="Aggiunge un percorso a .gitignore e crea un commit coerente."
    )
    parser.add_argument("path", help="Percorso da escludere dal repository")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> None:
    """Execute the command-line entry point."""
    args = parse_arguments(argv)
    update_gitignore(args.path)


if __name__ == "__main__":
    main()
