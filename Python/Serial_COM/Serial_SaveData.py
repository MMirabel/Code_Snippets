"""Stream data from a serial port to a text file.

This module provides a small CLI utility around pySerial to capture
line-oriented data coming from a configurable serial port and persist it
on disk. It handles directory creation, graceful shutdown on CTRL+C, and
optional limits on the number of lines recorded."""

# Example CLI invocations:
# python Python/Serial_COM/Serial_SaveData.py output.txt
# python Python/Serial_COM/Serial_SaveData.py data.log --port COM3 --baudrate 115200 --max-lines 1000

import argparse
import sys
from pathlib import Path

import serial

DEFAULT_PORT = "COM5"
DEFAULT_BAUDRATE = 9600
DEFAULT_TIMEOUT = 1.0
DEFAULT_ENCODING = "utf-8"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Read from a serial port and save the data to file."
    )
    parser.add_argument("output_file", help="Path to the output file.")
    parser.add_argument(
        "--port",
        default=DEFAULT_PORT,
        help="Serial port to open (default: %(default)s).",
    )
    parser.add_argument(
        "--baudrate",
        type=int,
        default=DEFAULT_BAUDRATE,
        help="Baud rate (default: %(default)s).",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT,
        help="Read timeout in seconds (default: %(default)s).",
    )
    parser.add_argument(
        "--max-lines",
        type=int,
        default=None,
        help="Stop after writing this many lines (default: infinite).",
    )
    parser.add_argument(
        "--encoding",
        default=DEFAULT_ENCODING,
        help="Encoding used to decode incoming data (default: %(default)s).",
    )
    parser.add_argument(
        "--append",
        action="store_true",
        help="Append to an existing file instead of overwriting it.",
    )
    parser.add_argument(
        "--flush-every",
        type=int,
        default=1,
        help="Number of lines between flushes (default: %(default)s).",
    )
    return parser.parse_args()


def open_output_file(path: Path, append: bool, encoding: str):
    if not path.parent.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
    mode = "a" if append else "w"
    return path.open(mode, encoding=encoding, newline="")


def stream_serial_data(args: argparse.Namespace) -> int:
    try:
        serial_kwargs = {"baudrate": args.baudrate, "timeout": args.timeout}
        ser = serial.serial_for_url(args.port, **serial_kwargs)
    except serial.SerialException as exc:
        print(f"Error opening serial port {args.port}: {exc}", file=sys.stderr)
        return 1

    output_path = Path(args.output_file)
    lines_written = 0
    flush_interval = max(1, args.flush_every)

    try:
        with ser, open_output_file(output_path, args.append, args.encoding) as output_file:
            while args.max_lines is None or lines_written < args.max_lines:
                try:
                    raw_line = ser.readline()
                except serial.SerialException as exc:
                    print(f"Error reading from the serial port: {exc}", file=sys.stderr)
                    return 1

                if not raw_line:
                    continue

                decoded = raw_line.decode(args.encoding, errors="replace").rstrip("\r\n")
                print(decoded)
                output_file.write(decoded + "\n")
                lines_written += 1

                if lines_written % flush_interval == 0:
                    output_file.flush()

    except KeyboardInterrupt:
        print("\nLogging interrupted by user.")
    except OSError as exc:
        print(f"I/O error with file {output_path}: {exc}", file=sys.stderr)
        return 1

    return 0


def main() -> int:
    args = parse_args()
    return stream_serial_data(args)


if __name__ == "__main__":
    raise SystemExit(main())
