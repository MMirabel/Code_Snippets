#!/usr/bin/env python3
"""
Build Index Tool for Code Snippets Repository

Scansiona automaticamente tutti i directory e file del repository
e aggiorna l'indice nel README.md principale.

Dipendenze: pathlib, re (standard library)
Uso: python tools/build_index.py
"""

import os
import re
from pathlib import Path
from typing import Dict, List, Tuple
from dataclasses import dataclass

@dataclass
class SnippetInfo:
    """Informazioni su uno snippet di codice."""
    path: Path
    language: str
    title: str
    description: str
    complexity: str
    dependencies: List[str]

class IndexBuilder:
    """Costruttore dell'indice per il repository di snippet."""
    
    def __init__(self, repo_root: Path = None):
        self.repo_root = repo_root or Path(__file__).parent.parent
        self.snippets: Dict[str, List[SnippetInfo]] = {}
        
        # Pattern per estrarre metadati dai file
        self.metadata_patterns = {
            'description': r'["\']([^"\']+)["\']',
            'complexity': r'Complessità:\s*([^"\n]+)',
            'dependencies': r'Dipendenze:\s*([^"\n]+)',
        }
    
    def scan_directory(self, directory: Path, language: str) -> List[SnippetInfo]:
        """Scansiona una directory per snippet di una lingua specifica."""
        snippets = []
        
        if not directory.exists():
            return snippets
        
        # Pattern file per ogni linguaggio
        file_patterns = {
            'python': ['*.py'],
            'matlab': ['*.m'],
            'simulink': ['*.slx', '*.mdl'],
            'c': ['*.c', '*.h'],
            'cpp': ['*.cpp', '*.hpp', '*.h']
        }
        
        patterns = file_patterns.get(language, ['*'])
        
        for pattern in patterns:
            for file_path in directory.rglob(pattern):
                # Salta file di test e temporanei
                if any(skip in file_path.name.lower() 
                       for skip in ['test', 'tmp', '__pycache__', '.git']):
                    continue
                
                snippet_info = self.extract_snippet_info(file_path, language)
                if snippet_info:
                    snippets.append(snippet_info)
        
        return snippets
    
    def extract_snippet_info(self, file_path: Path, language: str) -> SnippetInfo:
        """Estrae informazioni da un file di snippet."""
        try:
            # Leggi i primi righe per metadati
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                header = f.read(2000)  # Prime 2000 caratteri
            
            # Estrai titolo (dal nome file o commenti)
            title = self.extract_title(file_path, header)
            
            # Estrai descrizione
            description = self.extract_description(header, language)
            
            # Estrai complessità
            complexity = self.extract_complexity(header)
            
            # Estrai dipendenze
            dependencies = self.extract_dependencies(header)
            
            return SnippetInfo(
                path=file_path,
                language=language,
                title=title,
                description=description,
                complexity=complexity,
                dependencies=dependencies
            )
        
        except Exception as e:
            print(f"Errore nell'analisi di {file_path}: {e}")
            return None
    
    def extract_title(self, file_path: Path, content: str) -> str:
        """Estrae il titolo dal file."""
        # Prova a trovare titolo nei commenti
        patterns = [
            r'^\s*[#%/*]\s*([^\n]+)',  # Prima riga di commento
            r'@brief\s+([^\n]+)',      # Doxygen brief
            r'"""([^"]+)"""',          # Python docstring
        ]
        
        for pattern in patterns:
            match = re.search(pattern, content, re.MULTILINE | re.IGNORECASE)
            if match:
                title = match.group(1).strip()
                if len(title) > 10:  # Titolo significativo
                    return title
        
        # Fallback: nome file senza estensione
        return file_path.stem.replace('_', ' ').title()
    
    def extract_description(self, content: str, language: str) -> str:
        """Estrae la descrizione dal contenuto."""
        # Pattern specifici per linguaggio
        if language == 'python':
            # Docstring Python
            match = re.search(r'"""([^"]+)"""', content, re.DOTALL)
            if match:
                desc = match.group(1).strip()
                return ' '.join(desc.split()[:20])  # Prime 20 parole
        
        # Pattern generici
        patterns = [
            r'/\*\*([^*]+)\*/',  # C/C++ multi-line comment
            r'%\s*([^\n]{20,})', # MATLAB comment
            r'#\s*([^\n]{20,})', # Commento shell/Python
        ]
        
        for pattern in patterns:
            match = re.search(pattern, content)
            if match:
                desc = match.group(1).strip()
                return ' '.join(desc.split()[:15])  # Prime 15 parole
        
        return "Nessuna descrizione disponibile"
    
    def extract_complexity(self, content: str) -> str:
        """Estrae informazioni sulla complessità."""
        match = re.search(self.metadata_patterns['complexity'], content, re.IGNORECASE)
        return match.group(1).strip() if match else "N/A"
    
    def extract_dependencies(self, content: str) -> List[str]:
        """Estrae lista delle dipendenze."""
        match = re.search(self.metadata_patterns['dependencies'], content, re.IGNORECASE)
        if match:
            deps_str = match.group(1).strip()
            return [dep.strip() for dep in deps_str.split(',')]
        return []
    
    def scan_all_directories(self):
        """Scansiona tutte le directory del repository."""
        language_dirs = {
            'python': self.repo_root / 'python',
            'matlab': self.repo_root / 'matlab', 
            'simulink': self.repo_root / 'simulink',
            'c': self.repo_root / 'c',
            'cpp': self.repo_root / 'cpp'
        }
        
        for language, directory in language_dirs.items():
            print(f"Scansionando directory {language}...")
            snippets = self.scan_directory(directory, language)
            self.snippets[language] = snippets
            print(f"  Trovati {len(snippets)} snippet")
    
    def generate_index_content(self) -> str:
        """Genera il contenuto dell'indice in formato Markdown."""
        content = []
        content.append("## Indice degli Snippet\n")
        content.append("*Generato automaticamente con `python tools/build_index.py`*\n")
        
        total_snippets = sum(len(snippets) for snippets in self.snippets.values())
        content.append(f"**Totale snippet: {total_snippets}**\n")
        
        for language, snippets in self.snippets.items():
            if not snippets:
                continue
                
            content.append(f"### {language.upper()}\n")
            
            for snippet in sorted(snippets, key=lambda x: x.title):
                # Path relativo dal root
                rel_path = snippet.path.relative_to(self.repo_root)
                
                # Formato: - [Titolo](path) - Descrizione (Complessità: X)
                line = f"- [`{snippet.title}`]({rel_path}) - {snippet.description}"
                
                if snippet.complexity != "N/A":
                    line += f" (*{snippet.complexity}*)"
                
                if snippet.dependencies:
                    deps_str = ", ".join(snippet.dependencies[:3])  # Prime 3 dipendenze
                    line += f" [Req: {deps_str}]"
                
                content.append(line + "\n")
            
            content.append("")  # Riga vuota tra sezioni
        
        return "".join(content)
    
    def update_readme(self):
        """Aggiorna il README.md principale con l'indice generato."""
        readme_path = self.repo_root / "README.md"
        
        if not readme_path.exists():
            print("README.md non trovato!")
            return False
        
        # Leggi README esistente
        with open(readme_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Genera nuovo indice
        index_content = self.generate_index_content()
        
        # Pattern per trovare e sostituire la sezione indice
        index_pattern = r'(## Indice degli Snippet.*?)(?=\n## |\Z)'
        
        if re.search(index_pattern, content, re.DOTALL):
            # Sostituisci indice esistente
            new_content = re.sub(index_pattern, index_content.strip(), content, flags=re.DOTALL)
        else:
            # Aggiungi indice prima della sezione Licenza
            license_pattern = r'(## Licenza)'
            if re.search(license_pattern, content):
                new_content = re.sub(license_pattern, f"{index_content}\n\\1", content)
            else:
                # Aggiungi alla fine
                new_content = content + "\n\n" + index_content
        
        # Scrivi README aggiornato
        with open(readme_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"README.md aggiornato con {sum(len(s) for s in self.snippets.values())} snippet")
        return True

def main():
    """Funzione principale."""
    print("=== Build Index Tool ===")
    print("Analizzando repository per snippet di codice...\n")
    
    builder = IndexBuilder()
    builder.scan_all_directories()
    
    print(f"\nRiepilogo:")
    for language, snippets in builder.snippets.items():
        print(f"  {language}: {len(snippets)} snippet")
    
    print("\nAggiornando README.md...")
    success = builder.update_readme()
    
    if success:
        print("✓ Indice aggiornato con successo!")
    else:
        print("✗ Errore nell'aggiornamento dell'indice")

if __name__ == "__main__":
    main()