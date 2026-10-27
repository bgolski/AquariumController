# AquariumController

ESP32 Dev Board-based smart aquarium controller (ESP32 Dev Board, ESP32-WROOM-32, CP2102, USB-C) with a simple HTTP interface for Homebridge/HomeKit integration. The controller toggles aquarium lights via a relay and reads water temperature from a DS18B20 probe. A physical manual switch remains the final authority for power to the lights.

## Project goals

- Remote control of aquarium lighting (HomeKit via Homebridge)
- Water temperature monitoring
- Safe, compact wiring with a 3D-printed enclosure

## Hardware architecture

- ESP32 Dev Board (ESP32-WROOM-32, CP2102, USB-C; PlatformIO target: `esp32dev`)
- 18x24 hole prototyping board
- 3V relay board (ESP32-driven)
- Manual on/off switch (physical override)
- DS18B20 temperature probe
- LED lighting power supply
- 5V USB power for the ESP32

Power flow (lights):

Outlet -> Manual switch -> Relay (NO) -> LED power supply -> LEDs

If the manual switch is OFF, the lights are OFF regardless of the ESP32 state.

## Pin map (current)

- Relay IN: `GPIO25`
- DS18B20 DATA: `GPIO26` (with 4.7k pull-up to 3.3V)
- Relay VCC: `3.3V`
- Relay GND: `GND`
- DS18B20 VDD: `3.3V`
- DS18B20 GND: `GND`

## Wiring details

- Relay low-voltage side: `VCC -> 3.3V`, `GND -> GND`, `IN -> GPIO25`
- DS18B20: `VDD -> 3.3V`, `GND -> GND`, `DATA -> GPIO26`, plus `4.7k DATA -> 3.3V`
- ESP32 is powered by USB-C
- Relay module mounts adjacent to the proto board in the enclosure

## Circuit diagram source of truth

The hardware wiring source of truth is the detailed circuit document:

- `/Volumes/External/bradleygolski/Developer/AquariumController/docs/circuit-diagram.md`
- `/Volumes/External/bradleygolski/Developer/AquariumController/docs/circuit-diagram.svg`

![Aquarium Controller Circuit Diagram](docs/circuit-diagram.svg)

## Physical layout constraints

- Proto board: `70mm x 50mm` (`18x24` perfboard)
- ESP32 Dev Board: `52mm x 28.5mm x 12.5mm`
- Relay module: `70mm x 17mm x 19mm`
- Relay module is mounted adjacent in the enclosure (not flat on the same proto board surface)

## Enclosure openings (fixed orientation)

- Left wall: USB-C power cable
- Top wall: temperature probe cable
- Right wall: relay load cables (COM/NO in/out)

## Firmware behavior

The ESP32 connects to Wi-Fi, advertises mDNS, and exposes a small HTTP API:

- `GET /light/on` -> turn relay ON
- `GET /light/off` -> turn relay OFF
- `GET /light/status` -> report relay state
- `GET /temperature/c` -> read DS18B20 temperature in Celsius
- `GET /temperature/f` -> read DS18B20 temperature in Fahrenheit
- `GET /status/json` -> combined light + temperature JSON status
- `GET /homebridge/payload` -> Homebridge-oriented payload

## Configuration

Wi-Fi settings are injected at build time from `.env` using `env-extra.py`.

```ini
# .env
WIFI_SSID=your_wifi_network_name
WIFI_PASSWORD=your_wifi_password
```

Notes:

- `.env` is ignored by git.
- Do not hardcode credentials in source files.

## Build and flash

Using PlatformIO:

- Build: `pio run`
- Upload: `pio run -t upload`
- Monitor: `pio device monitor`

Serial settings are in `platformio.ini`.

## Contributing

See `AGENTS.md` for contribution guidance and project conventions.
