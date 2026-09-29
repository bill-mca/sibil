"""Log button presses from the ESP32 to a text file.

The firmware prints a line like ``PRESS red 123456`` for every press.
This script reads those lines over USB serial and appends one line per
press to the log, stamped with this computer's clock:

    2026-09-29T15:40:12.345+10:00 red

The ESP32 has no real-time clock, so the timestamp is taken here, the
moment the line arrives; USB adds only a millisecond or two.

Run with:  uv run log_buttons.py [--port /dev/ttyACM0] [--log presses.txt]
"""

import argparse
import sys
import time
from datetime import datetime
from pathlib import Path

import serial


def open_port(port: str) -> serial.Serial:
    """Open the serial port, waiting for the board if it isn't there yet."""
    announced = False
    while True:
        try:
            return serial.Serial(port, 115200, timeout=1)
        except serial.SerialException as err:
            if not announced:
                print(f"waiting for {port}: {err}", file=sys.stderr)
                announced = True
            time.sleep(1)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--port", default="/dev/ttyACM0")
    parser.add_argument("--log", type=Path, default=Path("button_presses.txt"))
    args = parser.parse_args()

    print(f"logging presses to {args.log.resolve()}  (Ctrl+C to stop)")
    with args.log.open("a", encoding="utf-8") as log:
        while True:
            port = open_port(args.port)
            print(f"connected to {args.port}")
            try:
                while True:
                    # readline() gives b"" after a quiet second; keep waiting.
                    line = port.readline().decode(errors="replace").strip()
                    if line.startswith("PRESS "):
                        colour = line.split()[1]
                        stamp = datetime.now().astimezone().isoformat(timespec="milliseconds")
                        log.write(f"{stamp} {colour}\n")
                        log.flush()
                        print(f"{stamp} {colour}")
                    elif line.startswith("READY"):
                        # Sent once at boot, with each pin's idle level.
                        print(f"board: {line}")
                        if "=HIGH" in line:
                            print("warning: a button reads pressed while idle; "
                                  "check its wiring", file=sys.stderr)
            except serial.SerialException:
                # Unplugged or reset: go back to waiting for the port.
                print(f"lost {args.port}, reconnecting", file=sys.stderr)
            finally:
                port.close()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        pass
