---

# Jetson Edge Person Detection (MQTT Pipeline)

A lightweight **embedded device-to-cloud pipeline** built on **Jetson Orin Nano** using a USB camera for **real-time person detection**, with event-driven communication via **MQTT**.

---

## Overview

This project simulates a real-world **edge AI device**:

* Captures frames from a USB camera
* Runs **local AI inference (person detection)**
* Generates events based on detection confidence
* Saves image snapshots locally
* Publishes telemetry to cloud via **MQTT**
* Reports device health periodically

---

## Architecture
<img width="1536" height="1024" alt="image" src="https://github.com/user-attachments/assets/b3fd69e8-3cc1-482c-bdea-9e89eab08a4f" />

### MQTT Handshake
<img width="3400" height="2078" alt="image" src="https://github.com/user-attachments/assets/a9f6ac9e-0771-4d95-b69c-1c86bf3e0acd" />


```text
USB Camera
   ↓
Jetson Orin Nano (Edge Device)
   ├── Camera Capture (OpenCV)
   ├── Person Detection (Inference Engine)
   ├── Event Manager (threshold + cooldown)
   ├── Snapshot Storage
   ├── MQTT Client (publish events)
   └── Health Monitor (heartbeat)
        ↓
MQTT Broker
        ↓
Cloud Subscriber / Dashboard
```

---

## Features

* USB camera integration (V4L2/OpenCV)
* Local AI inference (person detection)
* Event-driven system (threshold + cooldown)
* Snapshot capture on detection
* MQTT-based device-to-cloud communication
* Device health telemetry (CPU, memory, uptime)
* Retry-ready architecture (extensible)
* Modular design (clean separation of concerns)
* systemd service support (auto-start)

---

## Project Structure

```text
jetson_person_mqtt/
├── app.py                  # Main application loop
├── config.py               # Configuration parameters
├── requirements.txt
├── snapshots/              # Saved event images
├── logs/
├── services/
│   └── edge_person.service # systemd service
├── src/
│   ├── camera_manager.py
│   ├── inference_engine.py
│   ├── event_manager.py
│   ├── snapshot_manager.py
│   ├── mqtt_client.py
│   ├── health_monitor.py
│   ├── retry_queue.py
│   └── utils.py
└── cloud/
    ├── subscriber.py       # MQTT subscriber
    └── dashboard_app.py    # (optional UI)
```

---

## Setup Instructions

### 1. Clone repository

```bash
git clone <your-repo-url>
cd jetson_person_mqtt
```

---

### 2. Install dependencies

```bash
sudo apt update
sudo apt install python3-pip -y
pip3 install -r requirements.txt
```

---

### 3. Start MQTT broker (local)

```bash
sudo apt install mosquitto mosquitto-clients -y
sudo systemctl start mosquitto
```

---

### 4. Verify camera

```bash
ls /dev/video*
v4l2-ctl --list-devices
```

---

## Running the Project

### Terminal 1 → Start subscriber

```bash
python3 cloud/subscriber.py
```

---

### Terminal 2 → Start edge device app

```bash
python3 app.py
```

---

## Expected Output

Subscriber will print:

```text
device/jetson-orin-01/events {"device_id": "...", "confidence": 0.8}
```

Snapshots will be saved in:

```bash
snapshots/
```

---

## MQTT Topics

```text
device/jetson-orin-01/events
device/jetson-orin-01/health
device/jetson-orin-01/status
```

---

## Example Event Payload

```json
{
  "device_id": "jetson-orin-01",
  "event_type": "person_detected",
  "confidence": 0.84,
  "timestamp": "2026-04-21T21:30:15Z",
  "snapshot": "event_1713829201.jpg"
}
```

---

## Health Telemetry

Published every ~60 seconds:

```json
{
  "device_id": "jetson-orin-01",
  "cpu": 27,
  "memory": 45,
  "timestamp": 1713829300
}
```

---

## Current State

* Person detection is currently **stubbed (dummy logic)**
* Replace `src/inference_engine.py` with:

  * YOLO / MobileNet / Jetson inference for real detection

---

## Run as systemd Service

```bash
sudo cp services/edge_person.service /etc/systemd/system/
sudo systemctl daemon-reexec
sudo systemctl enable edge_person.service
sudo systemctl start edge_person.service
```

Check logs:

```bash
journalctl -u edge_person.service -f
```

---

## Troubleshooting

### Camera not detected

```bash
ls /dev/video*
```

---

### MQTT not working

```bash
mosquitto_sub -t "device/#"
```

---

### No snapshots

```bash
mkdir -p snapshots
```

---

## Future Improvements

* Replace dummy inference with real model
* Dockerize edge application
* Integrate with AWS IoT Core
* Add TLS certificates for MQTT
* Build web dashboard
* Add multi-object detection
* Implement store-and-forward retry queue
* Upload images to cloud storage (S3)

---

## Author

**Aaksha Jaywant**
Embedded Systems | Edge AI | Firmware
