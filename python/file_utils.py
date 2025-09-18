"""
Utility per I/O file robusto con gestione errori e backup.

Dipendenze: pathlib, shutil (standard library)
Complessità: O(n) dove n è la dimensione del file
"""

import os
import shutil
import json
import csv
from pathlib import Path
from typing import List, Dict, Any, Union
from contextlib import contextmanager


def safe_read_file(filepath: Union[str, Path], encoding: str = 'utf-8') -> str:
    """
    Lettura sicura di un file di testo con gestione errori.
    
    Args:
        filepath: Percorso del file
        encoding: Codifica del file
        
    Returns:
        Contenuto del file come stringa
        
    Raises:
        FileNotFoundError: Se il file non esiste
        UnicodeDecodeError: Se la codifica è errata
    """
    try:
        with open(filepath, 'r', encoding=encoding) as f:
            return f.read()
    except FileNotFoundError:
        raise FileNotFoundError(f"File not found: {filepath}")
    except UnicodeDecodeError as e:
        raise UnicodeDecodeError(f"Encoding error in {filepath}: {e}")


def safe_write_file(filepath: Union[str, Path], content: str, 
                   encoding: str = 'utf-8', backup: bool = True) -> None:
    """
    Scrittura sicura di un file con backup opzionale.
    
    Args:
        filepath: Percorso del file
        content: Contenuto da scrivere
        encoding: Codifica del file
        backup: Se True, crea backup del file esistente
    """
    filepath = Path(filepath)
    
    # Crea backup se file esiste e backup è richiesto
    if backup and filepath.exists():
        backup_path = filepath.with_suffix(filepath.suffix + '.bak')
        shutil.copy2(filepath, backup_path)
        print(f"Backup created: {backup_path}")
    
    # Crea directory se non esiste
    filepath.parent.mkdir(parents=True, exist_ok=True)
    
    # Scrittura atomica usando file temporaneo
    temp_path = filepath.with_suffix(filepath.suffix + '.tmp')
    try:
        with open(temp_path, 'w', encoding=encoding) as f:
            f.write(content)
        # Rinomina solo se scrittura completata con successo
        temp_path.replace(filepath)
        print(f"File written successfully: {filepath}")
    except Exception as e:
        # Cleanup file temporaneo in caso di errore
        if temp_path.exists():
            temp_path.unlink()
        raise e


def read_json_safe(filepath: Union[str, Path]) -> Dict[str, Any]:
    """
    Lettura sicura di file JSON con validazione.
    
    Args:
        filepath: Percorso del file JSON
        
    Returns:
        Dizionario dal contenuto JSON
    """
    try:
        content = safe_read_file(filepath)
        return json.loads(content)
    except json.JSONDecodeError as e:
        raise ValueError(f"Invalid JSON in {filepath}: {e}")


def write_json_safe(filepath: Union[str, Path], data: Dict[str, Any], 
                   indent: int = 2, backup: bool = True) -> None:
    """
    Scrittura sicura di file JSON con formattazione.
    
    Args:
        filepath: Percorso del file JSON
        data: Dati da serializzare
        indent: Indentazione per formattazione
        backup: Se True, crea backup del file esistente
    """
    content = json.dumps(data, indent=indent, ensure_ascii=False)
    safe_write_file(filepath, content, backup=backup)


def read_csv_safe(filepath: Union[str, Path], delimiter: str = ',') -> List[Dict[str, str]]:
    """
    Lettura sicura di file CSV.
    
    Args:
        filepath: Percorso del file CSV
        delimiter: Delimitatore del CSV
        
    Returns:
        Lista di dizionari (una riga per dizionario)
    """
    try:
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            reader = csv.DictReader(f, delimiter=delimiter)
            return list(reader)
    except Exception as e:
        raise ValueError(f"Error reading CSV {filepath}: {e}")


@contextmanager
def working_directory(path: Union[str, Path]):
    """
    Context manager per cambiare temporaneamente directory di lavoro.
    
    Usage:
        with working_directory('/path/to/dir'):
            # Operazioni nella nuova directory
            pass
    """
    old_cwd = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(old_cwd)


def find_files(directory: Union[str, Path], pattern: str = "*", 
               recursive: bool = True) -> List[Path]:
    """
    Trova file in una directory con pattern matching.
    
    Args:
        directory: Directory di ricerca
        pattern: Pattern di ricerca (es. "*.py", "test_*.txt")
        recursive: Se True, cerca ricorsivamente nelle sottodirectory
        
    Returns:
        Lista di Path dei file trovati
    """
    directory = Path(directory)
    if not directory.exists():
        raise FileNotFoundError(f"Directory not found: {directory}")
    
    if recursive:
        return list(directory.rglob(pattern))
    else:
        return list(directory.glob(pattern))


# Esempi di utilizzo
if __name__ == "__main__":
    # Test delle utility
    test_dir = Path("/tmp/file_utils_test")
    test_dir.mkdir(exist_ok=True)
    
    # Test scrittura/lettura file
    test_file = test_dir / "test.txt"
    safe_write_file(test_file, "Hello, World!\nTest content.", backup=False)
    content = safe_read_file(test_file)
    print(f"File content: {repr(content)}")
    
    # Test JSON
    json_file = test_dir / "test.json"
    test_data = {"name": "Test", "values": [1, 2, 3]}
    write_json_safe(json_file, test_data, backup=False)
    loaded_data = read_json_safe(json_file)
    print(f"JSON data: {loaded_data}")
    
    # Test CSV
    csv_file = test_dir / "test.csv"
    with open(csv_file, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['name', 'age'])
        writer.writerow(['Alice', '30'])
        writer.writerow(['Bob', '25'])
    
    csv_data = read_csv_safe(csv_file)
    print(f"CSV data: {csv_data}")
    
    # Test find files
    found_files = find_files(test_dir, "*.txt")
    print(f"Found .txt files: {found_files}")
    
    # Cleanup
    shutil.rmtree(test_dir)
    print("Test completed and cleaned up.")