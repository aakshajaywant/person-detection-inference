# STM32F767 Real-Time Biometric IoT System: I2C Pulse Oximeter + MQTT Cloud Streaming

This project interfaces an **STM32F767 Nucleo-144** board with a **SparkFun Pulse Oximeter and Heart Rate Sensor - MAX30101 + MAX32664 (Qwiic / I2C)** and streams processed biometric telemetry to an MQTT broker for remote monitoring.

> Educational prototype only. Do not use this project for medical diagnosis, emergency monitoring, or safety-critical decisions.

## What the system does

- Reads heart rate, SpO2, confidence, and finger-detection status over I2C.
- Uses STM32 HAL I2C driver with MAX32664 command framing.
- Emits newline-delimited JSON over UART.
- A Python gateway reads UART and publishes telemetry to MQTT.
- Includes local subscriber for quick testing.

## Architecture

```text
+----------------------------+      I2C       +--------------------------------+
| STM32F767 Nucleo-144       | <------------> | SparkFun MAX30101 + MAX32664  |
|                            |                | Pulse Oximeter Sensor Hub     |
| I2C1: SCL/SDA              |                | I2C address: 0x55             |
| UART3: JSON telemetry      |                +--------------------------------+
+-------------+--------------+
              |
              | USB ST-LINK Virtual COM Port / UART
              v
+-------------+--------------+        MQTT        +----------------------------+
| Python MQTT Gateway        | -----------------> | MQTT Broker                |
| serial_to_mqtt.py          |                     | Mosquitto / HiveMQ / AWS*  |
+-------------+--------------+                     +----------------------------+
              |
              v
+----------------------------+
| Remote subscriber/dashboard|
+----------------------------+
```

\* For AWS IoT Core or Azure IoT Hub, add TLS certificates and broker-specific authentication.

## Hardware used

- STM32F767ZI Nucleo-144 or equivalent STM32F767 board
- SparkFun Pulse Oximeter and Heart Rate Sensor, MAX30101 + MAX32664, Qwiic / I2C
- Qwiic-to-male jumper cable or direct 4-wire connection
- USB cable for programming and UART logs
- Optional: Raspberry Pi / PC as MQTT gateway

## Important sensor facts

The SparkFun board uses a MAX30101 optical sensor plus MAX32664 biometric hub. SparkFun documents it as an I2C-based biometric sensor; the board exposes Qwiic/I2C pins, and its documented I2C address is `0x55`.

References:

- SparkFun product page / hookup guide: MAX30101 + MAX32664 biometric sensor hub
- SparkFun Arduino library: command framing and FIFO data model

## Wiring

### STM32F767 Nucleo-144 default pin example

This README assumes STM32CubeMX is configured as:

| Function | STM32F767 Nucleo Pin | Sensor Pin | Notes |
|---|---:|---:|---|
| 3.3V | 3V3 | 3V3/VCC | Do not use 5V unless your breakout explicitly supports it |
| GND | GND | GND | Common ground required |
| I2C1 SCL | PB8 / D15 | SCL | 100 kHz recommended first |
| I2C1 SDA | PB9 / D14 | SDA | Pull-ups usually present on Qwiic board |
| RESET | PB0 | RST | Optional but recommended |
| MFIO | PB1 | MFIO | Optional but recommended for app-mode reset |
| UART TX | PD8 / USART3_TX | PC USB VCP | Telemetry to gateway |
| UART RX | PD9 / USART3_RX | PC USB VCP | Optional command input |

If your board uses different I2C/UART pins, keep the application code and update CubeMX peripheral pin mapping.

## Firmware folder structure

```text
firmware/
  Core/
    Inc/
      app_config.h
      max32664.h
      telemetry.h
    Src/
      main.c
      max32664.c
      telemetry.c
```

This is intentionally written as **CubeMX/CubeIDE-friendly source** rather than a complete generated project. Create a normal STM32CubeIDE project for `NUCLEO-F767ZI`, enable I2C/UART/GPIO, then copy these files into `Core/Inc` and `Core/Src`.

## STM32CubeMX / CubeIDE setup

1. Create a new STM32 project for your STM32F767 board.
2. Enable `I2C1`:
   - Mode: I2C
   - Speed: Standard Mode 100 kHz for first bring-up
   - Own address: default
3. Enable `USART3` asynchronous:
   - Baud: `115200`
   - 8 data bits, no parity, 1 stop bit
4. Configure two GPIO outputs:
   - `BIO_RST_Pin` on PB0
   - `BIO_MFIO_Pin` on PB1
5. Generate code.
6. Copy this repository's `firmware/Core/Inc/*` to your project's `Core/Inc`.
7. Copy this repository's `firmware/Core/Src/*` to your project's `Core/Src`.
8. If CubeMX already generated `main.c`, merge the `USER CODE` sections from this project instead of blindly replacing your file.
9. Build and flash.

## Expected UART output

When your finger is detected, the STM32 prints JSON lines like:

```json
{"device_id":"stm32f767-bio-001","seq":42,"hr_bpm":74.2,"spo2_pct":98,"confidence":92,"status":3,"finger_detected":true,"uptime_ms":123456}
```

Status values from the MAX32664 algorithm commonly map as:

| Status | Meaning |
|---:|---|
| 0 | Success |
| 1 | Not ready |
| 2 | Object detected |
| 3 | Finger detected |

## Python gateway setup

From a PC, Raspberry Pi, or Jetson connected to the STM32 USB serial port:

```bash
cd gateway
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Run with a public test broker:

```bash
python serial_to_mqtt.py \
  --serial-port /dev/ttyACM0 \
  --baud 115200 \
  --mqtt-host test.mosquitto.org \
  --mqtt-port 1883 \
  --topic devices/stm32f767-bio-001/telemetry
```

On Windows, the serial port may look like `COM5`:

```powershell
python serial_to_mqtt.py --serial-port COM5 --mqtt-host test.mosquitto.org --topic devices/stm32f767-bio-001/telemetry
```

Open another terminal and subscribe:

```bash
python mqtt_subscriber.py \
  --mqtt-host test.mosquitto.org \
  --topic devices/stm32f767-bio-001/telemetry
```

Or use Mosquitto tools:

```bash
mosquitto_sub -h test.mosquitto.org -t devices/stm32f767-bio-001/telemetry -v
```

## Using a local Mosquitto broker

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto

python gateway/serial_to_mqtt.py --serial-port /dev/ttyACM0 --mqtt-host localhost --topic devices/stm32f767-bio-001/telemetry
mosquitto_sub -h localhost -t devices/stm32f767-bio-001/telemetry -v
```

## Bring-up checklist

1. Confirm 3.3V and GND at the sensor.
2. Confirm I2C pull-ups are present. Qwiic boards usually include pull-ups.
3. Start I2C at 100 kHz before trying 400 kHz.
4. Check that address `0x55` ACKs.
5. Confirm UART prints boot logs at 115200 baud.
6. Run the Python gateway and verify MQTT publishes.
7. Place your finger gently and keep it still for stable SpO2/heart-rate readings.

## Troubleshooting

### Sensor not detected

- Verify SDA/SCL are not swapped.
- Verify common ground.
- Use 3.3V logic.
- Lower I2C speed to 100 kHz.
- Confirm the STM32 HAL address uses left-shifted 8-bit address: `0x55 << 1`.

### Values stay zero or status says not ready

- Keep finger steady on the sensor for several seconds.
- Avoid bright ambient light leakage.
- Confirm `max32664_config_bpm()` returns success.
- Check RST/MFIO wiring if using app-mode reset.

### MQTT messages not received

- Verify serial port path.
- Confirm the gateway prints received JSON lines.
- Make sure publisher and subscriber use the same broker/topic.
- Public brokers can be rate limited or temporarily unavailable; try local Mosquitto.

## Resume-friendly project description

Developed a real-time biometric IoT monitoring system using STM32F767 and an I2C MAX30101/MAX32664 pulse oximeter, implementing reliable heart-rate/SpO2 acquisition, UART telemetry framing, and MQTT-based cloud streaming for remote health-data visualization.

## Suggested enhancements

- Add FreeRTOS tasks: `sensor_task`, `telemetry_task`, `watchdog_task`.
- Add hardware watchdog and reconnect logic.
- Store offline telemetry in external flash/SD card.
- Add TLS MQTT for AWS IoT Core.
- Add Node-RED dashboard.
- Add BLE or Ethernet directly on embedded gateway hardware.
