"""
Decoratori Python utili per timing, retry e caching.

Dipendenze: standard library
Complessità: O(1) per applicazione, varia per esecuzione
"""

import time
import functools
from typing import Any, Callable, Dict


def timing_decorator(func: Callable) -> Callable:
    """
    Decoratore per misurare il tempo di esecuzione di una funzione.
    
    Usage:
        @timing_decorator
        def my_function():
            time.sleep(1)
    """
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        start_time = time.time()
        result = func(*args, **kwargs)
        end_time = time.time()
        print(f"{func.__name__} executed in {end_time - start_time:.4f} seconds")
        return result
    return wrapper


def retry_decorator(max_attempts: int = 3, delay: float = 1.0):
    """
    Decoratore per retry automatico in caso di eccezioni.
    
    Args:
        max_attempts: Numero massimo di tentativi
        delay: Ritardo tra i tentativi (secondi)
    
    Usage:
        @retry_decorator(max_attempts=3, delay=0.5)
        def unreliable_function():
            # Potrebbe fallire occasionalmente
            pass
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            last_exception = None
            for attempt in range(max_attempts):
                try:
                    return func(*args, **kwargs)
                except Exception as e:
                    last_exception = e
                    if attempt < max_attempts - 1:
                        print(f"Attempt {attempt + 1} failed: {e}. Retrying in {delay}s...")
                        time.sleep(delay)
                    else:
                        print(f"All {max_attempts} attempts failed.")
            raise last_exception
        return wrapper
    return decorator


def simple_cache(func: Callable) -> Callable:
    """
    Decoratore per caching semplice (solo per funzioni con argomenti hashable).
    
    Usage:
        @simple_cache
        def expensive_calculation(n):
            return sum(range(n))
    """
    cache: Dict[Any, Any] = {}
    
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        # Crea chiave per cache (solo args hashable)
        key = (args, tuple(sorted(kwargs.items())))
        if key not in cache:
            cache[key] = func(*args, **kwargs)
        return cache[key]
    return wrapper


# Esempi di utilizzo
if __name__ == "__main__":
    @timing_decorator
    @simple_cache
    def fibonacci(n: int) -> int:
        """Calcolo fibonacci con cache e timing."""
        if n <= 1:
            return n
        return fibonacci(n-1) + fibonacci(n-2)
    
    @retry_decorator(max_attempts=3, delay=0.1)
    def flaky_network_call():
        """Simula una chiamata di rete instabile."""
        import random
        if random.random() < 0.7:  # 70% chance di fallimento
            raise ConnectionError("Network timeout")
        return "Success!"
    
    # Test
    print("Testing fibonacci with cache and timing:")
    result = fibonacci(10)
    print(f"Fibonacci(10) = {result}")
    
    print("\nTesting retry decorator:")
    try:
        result = flaky_network_call()
        print(f"Network call result: {result}")
    except Exception as e:
        print(f"Final failure: {e}")