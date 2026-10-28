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

- `docs/circuit-diagram.md`
- `docs/circuit-diagram.svg`

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

## Wi-Fi setup

Wi-Fi details are not built into the firmware. They are entered from a phone and stored on the board, so changing networks never needs a reflash.

1. Power the board. If it has no saved network, or cannot reach the saved one within about 20 seconds, it broadcasts an open network called `Aquarium-Setup`.
2. Join `Aquarium-Setup` from a phone or laptop. A setup page opens (if not, browse to `http://192.168.4.1`).
3. Choose **Configure WiFi**, pick your network, enter its password and save. The board joins it and stays on it after restarts.

The setup network closes after 5 minutes if nobody uses it, and the board restarts and tries the saved network again, so a router outage does not leave it stuck in setup.

To switch to a different network later, hold the board's **BOOT** button for 5 seconds while it is running. It forgets the saved network and restarts into setup.

Once connected, the board answers at `http://aquarium-lights.local`.

## Build and flash

Using PlatformIO:

- Test the board-independent logic on your computer: `pio test -e native`
- Build: `pio run`
- Upload over USB: `pio run -t upload` (PlatformIO finds the port; add `--upload-port <port>` if several boards are connected)
- Monitor: `pio device monitor`

The firmware uses the `min_spiffs.csv` partition layout (1.9 MB per app slot). Switching to it from the default layout needs one upload over USB; saved Wi-Fi settings are kept.

Serial settings are in `platformio.ini`.

## Contributing

See `AGENTS.md` for contribution guidance and project conventions.
