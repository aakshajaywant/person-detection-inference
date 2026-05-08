#!/usr/bin/env python3
"""Debug helper: print raw UART lines from the STM32."""

import argparse
import serial

parser = argparse.ArgumentParser()
parser.add_argument("--serial-port", required=True)
parser.add_argument("--baud", type=int, default=115200)
args = parser.parse_args()

with serial.Serial(args.serial_port, args.baud, timeout=2) as ser:
    while True:
        line = ser.readline().decode("utf-8", errors="replace").rstrip()
        if line:
            print(line)
