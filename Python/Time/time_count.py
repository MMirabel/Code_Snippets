# /*********************************************************************************************
# PROJECT:          Code Snippets
# COMPONENT:        Python/Time
# FILE:             time_count.py
# AUTHOR:           Ing. Mirko Mirabella
#                   m.mirabella@neptunengineering.com
#                   www.neptunengineering.com
# REVISION:         v. 1.0
# DATE:             19/09/2025
# **********************************************************************************************/

"""Command-line utility to measure or repeat a sleep interval.

Example::
    python Python/Time/time_count.py 0.5 -n 3
"""

import argparse
import sys
import time


# Use perf_counter for higher resolution compared to monotonic.
def measure_interval(duration: float, repetitions: int) -> tuple[float, float, float]:
    """Return the start time, end time, and total wait duration."""
    start = time.perf_counter()
    for _ in range(repetitions):
        time.sleep(duration)
    end = time.perf_counter()
    return start, end, end - start


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Accurately measure the elapsed time of sleep intervals."
    )
    parser.add_argument(
        "duration",
        nargs="?",
        default=1.0,
        type=float,
        help="Length in seconds of each sleep interval (default: 1.0).",
    )
    parser.add_argument(
        "-n",
        "--repetitions",
        default=1,
        type=int,
        help="Number of times the sleep interval is executed (default: 1).",
    )
    return parser.parse_args()


# Example usage: python Python/Time/time_count.py 0.5 -n 5
def main() -> int:
    args = parse_args()

    if args.duration < 0:
        print("Duration must be non-negative.", file=sys.stderr)
        return 1
    if args.repetitions < 1:
        print("Repetitions must be at least 1.", file=sys.stderr)
        return 1

    start, end, elapsed = measure_interval(args.duration, args.repetitions)
    print(f"start     : {start:9.6f}")
    print(f"end       : {end:9.6f}")
    print(f"elapsed   : {elapsed:9.6f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
