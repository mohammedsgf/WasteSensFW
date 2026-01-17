# Smart Waste Monitoring Firmware

ESP32-based firmware for LilyGo T-SIM7000G that monitors trash bin fill levels using an ultrasonic sensor and publishes telemetry data via MQTT over cellular network.

## Features

- **Fill Level Monitoring** - US-100 ultrasonic sensor measures distance to calculate trash bin fill percentage
- **GPS Location Tracking** - Built-in SIM7000G GPS provides real-time coordinates
- **Cellular Connectivity** - GPRS/LTE-M connection via SIM7000G modem
- **MQTT Publishing** - Sends JSON telemetry to configurable MQTT broker
- **Battery Monitoring** - Reports battery level percentage
- **Auto-Recovery** - Automatic reconnection on network/MQTT disconnection
- **Layered Architecture** - Clean separation of concerns for maintainability

## Hardware Requirements

| Component | Description |
|-----------|-------------|
| **Board** | LilyGo T-SIM7000G (ESP32 + SIM7000G) |
| **Sensor** | US-100 Ultrasonic Sensor (GPIO mode) |
| **SIM Card** | Nano SIM with data plan |
| **Power** | 3.7V LiPo battery or USB power |

### Pin Connections

| Function | GPIO Pin | Notes |
|----------|----------|-------|
| Modem TX | 27 | Fixed (board design) |
| Modem RX | 26 | Fixed (board design) |
| Power Key | 4 | Fixed (board design) |
| LED | 12 | Status indicator |
| US-100 Trigger | 32 | Configurable in `config.h` |
| US-100 Echo | 35 | Configurable in `config.h` |

## Architecture

The firmware uses a 4-layer hierarchical architecture:

```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                     │
│                    SmartWasteApp                         │
│         (Business logic, orchestration, scheduling)      │
├─────────────────────────────────────────────────────────┤
│                     NETWORK LAYER                        │
│              GprsManager  │  MqttService                 │
│        (Connection management, MQTT publish/subscribe)   │
├─────────────────────────────────────────────────────────┤
│                       HAL LAYER                          │
│     ModemHAL  │  SensorHAL  │  GpsHAL  │  PowerHAL      │
│           (Hardware abstraction interfaces)              │
├─────────────────────────────────────────────────────────┤
│                  DEVICE DRIVER LAYER                     │
│  SIM7000Driver │ US100Driver │ AdcDriver │ GpioDriver   │
│            (Low-level hardware access)                   │
└─────────────────────────────────────────────────────────┘
```

### Data Flow

```
US-100 Sensor ──► SensorHAL ──► SmartWasteApp ──► MqttService ──► MQTT Broker
                                     │
SIM7000G GPS ───► GpsHAL ────────────┤
                                     │
Battery ADC ────► PowerHAL ──────────┘
```

## Project Structure

```
smartWasteFW/
├── platformio.ini              # PlatformIO build configuration
├── README.md                   # This file
│
├── include/
│   └── config.h                # All configurable parameters
│
├── src/
│   ├── main.cpp                # Entry point (setup/loop)
│   │
│   ├── drivers/                # Device Driver Layer
│   │   ├── gpio_driver.h/cpp   # GPIO pin operations
│   │   ├── adc_driver.h/cpp    # ADC voltage reading
│   │   ├── us100_driver.h/cpp  # US-100 ultrasonic driver
│   │   └── sim7000_driver.h/cpp# SIM7000G modem driver
│   │
│   ├── hal/                    # Hardware Abstraction Layer
│   │   ├── modem_hal.h/cpp     # Modem initialization & control
│   │   ├── sensor_hal.h/cpp    # Distance sensor interface
│   │   ├── gps_hal.h/cpp       # GPS location interface
│   │   └── power_hal.h/cpp     # Battery monitoring interface
│   │
│   ├── network/                # Network Layer
│   │   ├── gprs_manager.h/cpp  # GPRS connection management
│   │   └── mqtt_service.h/cpp  # MQTT client wrapper
│   │
│   └── app/                    # Application Layer
│       └── smart_waste_app.h/cpp # Main application logic
│
└── lib/                        # External libraries
    ├── TinyGSM/                # GSM modem library (LilyGo fork)
    ├── pubsubclient/           # MQTT client library
    └── StreamDebugger/         # AT command debugging
```

## Configuration

All configuration is centralized in `include/config.h`:

### Device Settings

```cpp
#define DEVICE_ID               "smartwaste_001"  // Unique device identifier
#define FIRMWARE_VERSION        "1.0.0"
```

### Network Settings

```cpp
// GPRS APN - Change to your carrier's APN
#define GPRS_APN                "internet"
#define GPRS_USER               ""
#define GPRS_PASS               ""

// SIM PIN (leave empty if not required)
#define SIM_PIN                 ""
```

### MQTT Settings

```cpp
#define MQTT_BROKER             "test.mosquitto.org"
#define MQTT_PORT               1883
#define MQTT_CLIENT_ID          DEVICE_ID
#define MQTT_USER               ""          // Leave empty for anonymous
#define MQTT_PASS               ""
```

### Sensor Settings

```cpp
// US-100 Ultrasonic Sensor Pins
#define US100_TRIGGER_PIN       32
#define US100_ECHO_PIN          35

// Trash can height (sensor to bottom) in centimeters
#define TRASH_CAN_HEIGHT_CM     120.0f
```

### Timing Settings

```cpp
#define PUBLISH_INTERVAL_MS     300000      // 5 minutes between publishes
#define GPS_TIMEOUT_MS          120000      // 2 minutes GPS fix timeout
#define NETWORK_TIMEOUT_MS      180000      // 3 minutes network timeout
```

### Debug Settings

```cpp
#define SERIAL_DEBUG            1           // Enable serial debug output
#define DUMP_AT_COMMANDS        0           // Show AT commands (verbose)
```

## MQTT Data Format

### Topic

```
smartwaste/{device_id}/data
```

Example: `smartwaste/smartwaste_001/data`

### Payload (JSON)

```json
{
  "device_id": "smartwaste_001",
  "location": {
    "latitude": 25.276987,
    "longitude": 55.296249
  },
  "battery_level": 85,
  "fill_level": 65
}
```

| Field | Type | Description |
|-------|------|-------------|
| `device_id` | string | Unique device identifier |
| `location.latitude` | float | GPS latitude (degrees) |
| `location.longitude` | float | GPS longitude (degrees) |
| `battery_level` | int | Battery percentage (0-100) |
| `fill_level` | int | Trash bin fill percentage (0-100) |

### Fill Level Calculation

```
fill_level = ((trash_can_height - measured_distance) / trash_can_height) × 100
```

- **0%** = Empty (sensor reads maximum distance)
- **100%** = Full (sensor reads minimum distance)

## Build & Upload

### Prerequisites

1. **PlatformIO** - Install via VS Code extension or CLI
2. **USB Driver** - CP2102 or CH340 driver for your board
3. **SIM Card** - Insert nano SIM with active data plan

### Build

```bash
# Using PlatformIO CLI
pio run

# Or using VS Code
# Click PlatformIO icon → Build
```

### Upload

```bash
# Connect board via USB, then:
pio run -t upload

# Or using VS Code
# Click PlatformIO icon → Upload
```

### Monitor Serial Output

```bash
pio device monitor

# Or using VS Code
# Click PlatformIO icon → Monitor
```

Expected output on successful startup:

```
========================================
  Smart Waste Monitoring System
  Firmware: 1.0.0
  Device ID: smartwaste_001
========================================
[App] Initializing hardware...
[SIM7000] Initializing modem...
[SIM7000] Power on sequence complete
[ModemHAL] Modem ready
[ModemHAL] Name: SIM7000G
[GPRS] Connecting to network...
[GPRS] Connected successfully
[GpsHAL] GPS enabled
[MQTT] Connected successfully
[App] Initialization complete
```

## Testing MQTT Messages

### Using Mosquitto CLI

```bash
# Subscribe to your device's topic
mosquitto_sub -h test.mosquitto.org -t "smartwaste/smartwaste_001/data" -v
```

### Using MQTT Explorer

1. Connect to `test.mosquitto.org:1883`
2. Subscribe to `smartwaste/#`
3. View incoming messages

## Troubleshooting

### Modem Not Responding

```
[SIM7000] Modem not responding, power cycling...
```

**Solutions:**
- Check SIM card is properly inserted
- Verify antenna is connected
- Ensure adequate power supply (USB may not be enough)

### Network Registration Failed

```
[GPRS] Network registration failed
```

**Solutions:**
- Verify SIM has active data plan
- Check APN settings in `config.h`
- Try moving to area with better signal

### GPS Fix Timeout

```
[GpsHAL] GPS fix timeout, using default location
```

**Solutions:**
- Move device to open sky area
- GPS can take 2-3 minutes for first fix (cold start)
- Check GPS antenna connection

### Invalid Distance Reading

```
[SensorHAL] Invalid distance reading
```

**Solutions:**
- Check US-100 wiring (Trigger/Echo pins)
- Verify sensor is in GPIO mode (not UART mode)
- Ensure nothing blocks sensor path

### MQTT Connection Failed

```
[MQTT] Connection failed, state: -2
```

**Solutions:**
- Verify GPRS connection is active
- Check broker address and port
- Try different MQTT broker

## Memory Usage

| Resource | Used | Available | Usage |
|----------|------|-----------|-------|
| RAM | 22 KB | 320 KB | 6.7% |
| Flash | 333 KB | 1.3 MB | 25.4% |

## License

MIT License - See LICENSE file for details.

## References

- [LilyGo T-SIM7000G](https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G)
- [TinyGSM Library](https://github.com/vshymanskyy/TinyGSM)
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- [SIM7000 AT Commands](https://simcom.ee/documents/SIM7000x/SIM7000%20Series_AT%20Command%20Manual_V1.06.pdf)
