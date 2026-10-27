# AGENTS.md

## Project summary

This repo contains firmware and configuration for an ESP32 Dev Board-based smart aquarium controller (ESP32 Dev Board, ESP32-WROOM-32, CP2102, USB-C). The ESP32 hosts a lightweight HTTP server used by Homebridge (HomeKit compatibility) and controls aquarium lighting through a relay, with an optional DS18B20 temperature probe for water monitoring. A physical manual switch remains the final authority for power to the lights.

## Hardware overview

- ESP32 Dev Board (ESP32-WROOM-32, CP2102, USB-C; PlatformIO target: `esp32dev`)
- Relay board (3V logic; placed in series with the LED power line)
- Manual on/off switch (physical override)
- DS18B20 temperature sensor probe
- 5V USB power for the ESP32

Power flow (lights): Outlet -> Manual switch -> Relay (NO) -> LED power supply -> LEDs

Enclosure scope: only the ESP32 Dev Board and relay module live inside the enclosure; all external wiring exits the enclosure cleanly from designated sides.
Physical-fit rule: ESP32 mounts on the 70x50mm proto board; the 70x17mm relay module mounts adjacent in the enclosure (not flat on the same proto board surface).

Wiring standard: prefer side-edge pad buses on the proto board for long runs so no jumpers cross over the ESP32 body.
Hardware source of truth: `docs/circuit-diagram.md`.

Hardware layout validity checklist:

- wiring does not overlap the ESP32 module body
- routing is efficient and avoids unnecessary crossings
- side pins/pad buses are used where advantageous
- ESP32 header rows maintain the intended 9-hole center gap
- text and labels remain readable and non-overlapping
- enclosure footprint remains as small as possible while preserving serviceability

Enclosure opening standard:

- Left: USB-C power cable
- Top: temperature probe cable
- Right: relay load cables (COM/NO in/out)

## Pin map (current)

- Relay IN: GPIO25
- DS18B20 data: GPIO26 (4.7k pull-up to 3.3V)
- Relay VCC: 3.3V
- Relay GND: GND
- DS18B20 VDD: 3.3V
- DS18B20 GND: GND

## Firmware overview

- `src/main.cpp` connects to Wi-Fi, advertises mDNS, and exposes HTTP endpoints to toggle and query the relay.
- Endpoints:
  - `GET /light/on`
  - `GET /light/off`
  - `GET /light/status`
  - `GET /temperature/c`
  - `GET /temperature/f`
  - `GET /status/json`
  - `GET /homebridge/payload`

## Configuration and secrets

- Wi-Fi credentials are injected at build time from `.env` via `env-extra.py`.
- `.env` is ignored by git; `.env.example` is the template.
- Do **not** add secrets to source files.

## How to contribute effectively

- Keep changes small and reviewable; explain intent in commits/PRs.
- Prefer `.env` for configuration; never commit real credentials.
- If adding hardware features, update both code and docs (`README.md` + `docs/circuit-diagram.md`).
- For safety, do not change AC wiring assumptions without explicit review.
- Validate on hardware where possible; otherwise note what is untested.
- Keep project-critical information in `README.md` instead of adding new planning docs.

## Code pointers

- Firmware: `src/main.cpp`
- Build config: `platformio.ini`
- Env injection: `env-extra.py`
- Circuit source-of-truth: `docs/circuit-diagram.md`
- Wiring diagram (high-level): `README.md`
