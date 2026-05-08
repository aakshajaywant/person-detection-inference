#!/usr/bin/env python3
"""Read newline-delimited JSON from STM32 UART and publish it to MQTT."""

import argparse
import json
import sys
import time
from typing import Any, Dict

import paho.mqtt.client as mqtt
import serial


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="STM32 biometric UART-to-MQTT gateway")
    parser.add_argument("--serial-port", required=True, help="Serial port, e.g. /dev/ttyACM0 or COM5")
    parser.add_argument("--baud", type=int, default=115200, help="UART baud rate")
    parser.add_argument("--mqtt-host", default="localhost", help="MQTT broker hostname")
    parser.add_argument("--mqtt-port", type=int, default=1883, help="MQTT broker port")
    parser.add_argument("--topic", default="devices/stm32f767-bio-001/telemetry", help="MQTT telemetry topic")
    parser.add_argument("--client-id", default="stm32-bio-gateway", help="MQTT client ID")
    parser.add_argument("--username", default=None, help="MQTT username")
    parser.add_argument("--password", default=None, help="MQTT password")
    parser.add_argument("--qos", type=int, choices=[0, 1, 2], default=1, help="MQTT QoS")
    return parser.parse_args()


def connect_mqtt(args: argparse.Namespace) -> mqtt.Client:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=args.client_id)
    if args.username:
        client.username_pw_set(args.username, args.password)
    client.connect(args.mqtt_host, args.mqtt_port, keepalive=60)
    client.loop_start()
    return client


def normalize_payload(payload: Dict[str, Any]) -> Dict[str, Any]:
    payload.setdefault("gateway_ts", int(time.time()))
    payload.setdefault("schema", "biometric.telemetry.v1")
    return payload


def main() -> int:
    args = parse_args()
    client = connect_mqtt(args)

    print(f"Opening serial {args.serial_port} @ {args.baud}")
    print(f"Publishing to mqtt://{args.mqtt_host}:{args.mqtt_port}/{args.topic}")

    try:
        with serial.Serial(args.serial_port, args.baud, timeout=2) as ser:
            while True:
                raw = ser.readline().decode("utf-8", errors="replace").strip()
                if not raw:
                    continue

                try:
                    payload = normalize_payload(json.loads(raw))
                except json.JSONDecodeError:
                    print(f"Skipping non-JSON UART line: {raw}", file=sys.stderr)
                    continue

                encoded = json.dumps(payload, separators=(",", ":"))
                result = client.publish(args.topic, encoded, qos=args.qos)
                result.wait_for_publish(timeout=5)
                print(f"PUB {args.topic}: {encoded}")

    except KeyboardInterrupt:
        print("Exiting")
    finally:
        client.loop_stop()
        client.disconnect()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
