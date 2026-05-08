#!/usr/bin/env python3
"""Simple MQTT subscriber for biometric telemetry."""

import argparse
import json
import paho.mqtt.client as mqtt


def parse_args():
    parser = argparse.ArgumentParser(description="Subscribe to STM32 biometric MQTT telemetry")
    parser.add_argument("--mqtt-host", default="localhost")
    parser.add_argument("--mqtt-port", type=int, default=1883)
    parser.add_argument("--topic", default="devices/stm32f767-bio-001/telemetry")
    return parser.parse_args()


def main():
    args = parse_args()

    def on_connect(client, userdata, flags, reason_code, properties=None):
        print(f"Connected: {reason_code}; subscribing to {args.topic}")
        client.subscribe(args.topic, qos=1)

    def on_message(client, userdata, msg):
        text = msg.payload.decode("utf-8", errors="replace")
        try:
            data = json.loads(text)
            print(json.dumps(data, indent=2))
        except json.JSONDecodeError:
            print(f"{msg.topic}: {text}")

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="stm32-bio-subscriber")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(args.mqtt_host, args.mqtt_port, keepalive=60)
    client.loop_forever()


if __name__ == "__main__":
    main()
