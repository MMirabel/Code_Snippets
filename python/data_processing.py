"""
Utility per elaborazione e analisi dati con numpy e pandas.

Dipendenze: numpy, pandas (opzionali - fallback per pure Python)
Complessità: O(n) per la maggior parte delle operazioni
"""

import sys
from typing import List, Dict, Any, Optional, Union, Tuple

# Try to import numpy and pandas, fallback to pure Python if not available
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False
    print("Warning: numpy not available, using pure Python implementations")

try:
    import pandas as pd
    HAS_PANDAS = True
except ImportError:
    HAS_PANDAS = False
    print("Warning: pandas not available, using pure Python implementations")


def normalize_data(data: List[float], method: str = 'min_max') -> List[float]:
    """
    Normalizza i dati usando diversi metodi.
    
    Args:
        data: Lista di valori numerici
        method: 'min_max', 'z_score', 'robust'
    
    Returns:
        Lista di valori normalizzati
        
    Complessità: O(n)
    """
    if not data:
        return []
    
    if HAS_NUMPY:
        arr = np.array(data)
        
        if method == 'min_max':
            min_val, max_val = arr.min(), arr.max()
            if max_val == min_val:
                return [0.0] * len(data)
            return ((arr - min_val) / (max_val - min_val)).tolist()
        
        elif method == 'z_score':
            mean_val, std_val = arr.mean(), arr.std()
            if std_val == 0:
                return [0.0] * len(data)
            return ((arr - mean_val) / std_val).tolist()
        
        elif method == 'robust':
            median_val = np.median(arr)
            mad = np.median(np.abs(arr - median_val))
            if mad == 0:
                return [0.0] * len(data)
            return ((arr - median_val) / mad).tolist()
    
    else:
        # Pure Python implementation
        if method == 'min_max':
            min_val, max_val = min(data), max(data)
            if max_val == min_val:
                return [0.0] * len(data)
            return [(x - min_val) / (max_val - min_val) for x in data]
        
        elif method == 'z_score':
            mean_val = sum(data) / len(data)
            variance = sum((x - mean_val) ** 2 for x in data) / len(data)
            std_val = variance ** 0.5
            if std_val == 0:
                return [0.0] * len(data)
            return [(x - mean_val) / std_val for x in data]
    
    raise ValueError(f"Unsupported normalization method: {method}")


def calculate_statistics(data: List[float]) -> Dict[str, float]:
    """
    Calcola statistiche descrittive dei dati.
    
    Args:
        data: Lista di valori numerici
        
    Returns:
        Dizionario con statistiche
        
    Complessità: O(n log n) per mediana e quartili
    """
    if not data:
        return {}
    
    if HAS_NUMPY:
        arr = np.array(data)
        return {
            'count': len(data),
            'mean': float(arr.mean()),
            'median': float(np.median(arr)),
            'std': float(arr.std()),
            'min': float(arr.min()),
            'max': float(arr.max()),
            'q25': float(np.percentile(arr, 25)),
            'q75': float(np.percentile(arr, 75)),
            'skewness': float(calculate_skewness(arr.tolist())),
            'kurtosis': float(calculate_kurtosis(arr.tolist()))
        }
    
    else:
        # Pure Python implementation
        sorted_data = sorted(data)
        n = len(data)
        mean_val = sum(data) / n
        variance = sum((x - mean_val) ** 2 for x in data) / n
        
        def percentile(data_sorted, p):
            k = (n - 1) * p / 100
            f = int(k)
            c = k - f
            if f == n - 1:
                return data_sorted[f]
            return data_sorted[f] * (1 - c) + data_sorted[f + 1] * c
        
        return {
            'count': n,
            'mean': mean_val,
            'median': percentile(sorted_data, 50),
            'std': variance ** 0.5,
            'min': min(data),
            'max': max(data),
            'q25': percentile(sorted_data, 25),
            'q75': percentile(sorted_data, 75),
            'skewness': calculate_skewness(data),
            'kurtosis': calculate_kurtosis(data)
        }


def calculate_skewness(data: List[float]) -> float:
    """Calcola la skewness (asimmetria) dei dati."""
    if len(data) < 2:
        return 0.0
    
    mean_val = sum(data) / len(data)
    variance = sum((x - mean_val) ** 2 for x in data) / len(data)
    std_val = variance ** 0.5
    
    if std_val == 0:
        return 0.0
    
    skewness = sum((x - mean_val) ** 3 for x in data) / len(data)
    return skewness / (std_val ** 3)


def calculate_kurtosis(data: List[float]) -> float:
    """Calcola la kurtosis dei dati."""
    if len(data) < 2:
        return 0.0
    
    mean_val = sum(data) / len(data)
    variance = sum((x - mean_val) ** 2 for x in data) / len(data)
    std_val = variance ** 0.5
    
    if std_val == 0:
        return 0.0
    
    kurtosis = sum((x - mean_val) ** 4 for x in data) / len(data)
    return (kurtosis / (std_val ** 4)) - 3  # Excess kurtosis


def find_outliers(data: List[float], method: str = 'iqr') -> List[int]:
    """
    Trova outliers nei dati.
    
    Args:
        data: Lista di valori numerici
        method: 'iqr' (Interquartile Range) o 'z_score'
        
    Returns:
        Lista degli indici degli outliers
        
    Complessità: O(n)
    """
    if len(data) < 4:
        return []
    
    if method == 'iqr':
        stats = calculate_statistics(data)
        q1, q3 = stats['q25'], stats['q75']
        iqr = q3 - q1
        lower_bound = q1 - 1.5 * iqr
        upper_bound = q3 + 1.5 * iqr
        
        return [i for i, x in enumerate(data) 
                if x < lower_bound or x > upper_bound]
    
    elif method == 'z_score':
        normalized = normalize_data(data, 'z_score')
        return [i for i, z in enumerate(normalized) if abs(z) > 3]
    
    raise ValueError(f"Unsupported outlier detection method: {method}")


def moving_average(data: List[float], window_size: int) -> List[float]:
    """
    Calcola la media mobile dei dati.
    
    Args:
        data: Lista di valori numerici
        window_size: Dimensione della finestra
        
    Returns:
        Lista con medie mobili
        
    Complessità: O(n)
    """
    if window_size <= 0 or window_size > len(data):
        raise ValueError("Window size must be positive and <= data length")
    
    if HAS_NUMPY:
        return np.convolve(data, np.ones(window_size)/window_size, mode='valid').tolist()
    
    else:
        # Pure Python implementation
        result = []
        for i in range(len(data) - window_size + 1):
            window = data[i:i + window_size]
            result.append(sum(window) / window_size)
        return result


def correlation_matrix(data: List[List[float]]) -> List[List[float]]:
    """
    Calcola la matrice di correlazione di Pearson.
    
    Args:
        data: Lista di liste (ogni lista è una variabile)
        
    Returns:
        Matrice di correlazione
        
    Complessità: O(n * m^2) dove n = numero campioni, m = numero variabili
    """
    if not data or not all(len(row) == len(data[0]) for row in data):
        raise ValueError("All data series must have the same length")
    
    if HAS_NUMPY:
        matrix = np.array(data)
        return np.corrcoef(matrix).tolist()
    
    else:
        # Pure Python implementation
        n_vars = len(data)
        n_samples = len(data[0])
        
        # Calcola medie
        means = [sum(series) / n_samples for series in data]
        
        # Matrice di correlazione
        corr_matrix = [[0.0] * n_vars for _ in range(n_vars)]
        
        for i in range(n_vars):
            for j in range(n_vars):
                if i == j:
                    corr_matrix[i][j] = 1.0
                else:
                    # Calcola correlazione tra serie i e j
                    numerator = sum((data[i][k] - means[i]) * (data[j][k] - means[j]) 
                                  for k in range(n_samples))
                    
                    sum_sq_i = sum((data[i][k] - means[i]) ** 2 for k in range(n_samples))
                    sum_sq_j = sum((data[j][k] - means[j]) ** 2 for k in range(n_samples))
                    
                    denominator = (sum_sq_i * sum_sq_j) ** 0.5
                    
                    if denominator == 0:
                        corr_matrix[i][j] = 0.0
                    else:
                        corr_matrix[i][j] = numerator / denominator
        
        return corr_matrix


def linear_regression(x: List[float], y: List[float]) -> Tuple[float, float, float]:
    """
    Regressione lineare semplice: y = a*x + b
    
    Args:
        x: Valori della variabile indipendente
        y: Valori della variabile dipendente
        
    Returns:
        Tupla (slope, intercept, r_squared)
        
    Complessità: O(n)
    """
    if len(x) != len(y) or len(x) < 2:
        raise ValueError("x and y must have the same length >= 2")
    
    n = len(x)
    sum_x = sum(x)
    sum_y = sum(y)
    sum_xy = sum(x[i] * y[i] for i in range(n))
    sum_x2 = sum(xi ** 2 for xi in x)
    sum_y2 = sum(yi ** 2 for yi in y)
    
    # Calcola coefficienti
    denominator = n * sum_x2 - sum_x ** 2
    if denominator == 0:
        raise ValueError("Cannot perform regression: x values are constant")
    
    slope = (n * sum_xy - sum_x * sum_y) / denominator
    intercept = (sum_y - slope * sum_x) / n
    
    # Calcola R²
    y_mean = sum_y / n
    ss_tot = sum((yi - y_mean) ** 2 for yi in y)
    ss_res = sum((y[i] - (slope * x[i] + intercept)) ** 2 for i in range(n))
    
    r_squared = 1 - (ss_res / ss_tot) if ss_tot > 0 else 0
    
    return slope, intercept, r_squared


# Esempi di utilizzo
if __name__ == "__main__":
    import random
    
    print("=== Data Processing Utils Examples ===\n")
    
    # Genera dati di test
    random.seed(42)
    test_data = [random.gauss(50, 15) for _ in range(100)]
    test_data.extend([120, 130, -20])  # Aggiungi outliers
    
    print(f"Dataset di test: {len(test_data)} punti")
    
    # Test normalizzazione
    normalized = normalize_data(test_data, 'min_max')
    print(f"Normalizzazione min-max: range [{min(normalized):.3f}, {max(normalized):.3f}]")
    
    # Test statistiche
    stats = calculate_statistics(test_data)
    print(f"\nStatistiche descrittive:")
    for key, value in stats.items():
        print(f"  {key}: {value:.3f}")
    
    # Test outliers
    outliers = find_outliers(test_data, 'iqr')
    print(f"\nOutliers trovati (IQR): {len(outliers)} indices")
    print(f"Valori outliers: {[test_data[i] for i in outliers]}")
    
    # Test media mobile
    ma = moving_average(test_data[:20], 5)
    print(f"\nMedia mobile (finestra 5): {len(ma)} valori")
    print(f"Prime 5 medie: {[f'{x:.2f}' for x in ma[:5]]}")
    
    # Test correlazione
    x_data = list(range(50))
    y_data = [2 * x + random.gauss(0, 5) for x in x_data]
    z_data = [x ** 2 / 10 + random.gauss(0, 3) for x in x_data]
    
    corr_matrix = correlation_matrix([x_data, y_data, z_data])
    print(f"\nMatrice di correlazione (3x3):")
    for i, row in enumerate(corr_matrix):
        print(f"  [{', '.join(f'{x:6.3f}' for x in row)}]")
    
    # Test regressione lineare
    slope, intercept, r2 = linear_regression(x_data, y_data)
    print(f"\nRegressione lineare y = {slope:.3f}*x + {intercept:.3f}")
    print(f"R² = {r2:.3f}")
    
    print("\nTutti i test completati!")