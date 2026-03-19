# SensorIoT

Consolidated repository for the SensorIoT environmental sensing system — all embedded firmware and hardware designs.

## Architecture

```
Sensor Nodes (ATmega328P + BME280 + RF95/RFM69)
        │
        │  915 MHz packet radio
        ▼
Gateway (ESP8266 + RF radio + WiFi + OLED)
        │
        │  WiFi / MQTT
        ▼
Backend Server (see SensorIoT-Broker repo)
```

## Components

### firmware/gateway/
ESP8266-based gateway that receives sensor data from nodes via RF radio and publishes to MQTT.
- RFM69 / RF95 (LoRa) radio receiver
- MQTT publisher over WiFi
- 128x32 OLED display for real-time readings
- Embedded web interface for configuration
- OTA firmware updates
- Built with PlatformIO

### firmware/node/
Battery-operated sensor nodes (ATmega328P) that measure temperature, humidity, and pressure.
- BME280 environmental sensor
- RFM69 / RF95 radio transmitter
- Low-power sleep modes for extended battery life
- Multiple firmware variants (RadioHead, RFM69, power-boost)
- Built with PlatformIO

### firmware/display/
ESP32-based standalone display module (work in progress).
- ESP32-WROVER-IE with WiFi/BLE
- LTR-329 ambient light sensor

### PCBs/
EAGLE CAD hardware designs for all boards:
- `gateway/` — Gateway board revisions (v1.13–1.16)
- `node/` — Sensor node board revisions (v1.3b–1.5)
- `display/` — Display module board
- `accessories/` — BME280 daughter boards, ISP programmers

## Sensors

| Measurement | Sensor | Units |
|-------------|--------|-------|
| Temperature | BME280 | °F |
| Humidity | BME280 | % RH |
| Pressure | BME280 | inHg |
| Battery | ADC | V |
| Signal | RSSI | dBm |

## Building

Each firmware component has its own PlatformIO project:

```bash
cd firmware/gateway
pio run                    # Build
pio run -t upload          # Flash via USB
pio run -e ota -t upload   # Flash via OTA
```

## Related Repositories

- **SensorIoT-Broker** — Backend services (MQTT broker, REST API, MongoDB)
- **SensorIoT_app** — React Native mobile app

## Consolidation History

This repo now contains code previously split across:
- `SensorIoT-GW` — Gateway firmware and PCBs (original home of this repo)
- `SensorIoT-Node` — Sensor node firmware and PCBs
- `SensorIoT-Display` — Display module hardware and firmware
