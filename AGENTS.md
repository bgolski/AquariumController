# AGENTS.md

## Project summary
This repo contains firmware and configuration for an ESP32-based smart aquarium controller. The ESP32 hosts a lightweight HTTP server used by Homebridge (HomeKit compatibility) and controls aquarium lighting through a relay, with an optional DS18B20 temperature probe for water monitoring. A physical manual switch remains the final authority for power to the lights.

## Hardware overview
- ESP32 dev board (currently configured as `esp32cam` in PlatformIO)
- Relay board (3V logic; placed in series with the LED power line)
- Manual on/off switch (physical override)
- DS18B20 temperature sensor probe
- 5V USB power for the ESP32

Power flow (lights): Outlet → Manual switch → Relay (NO) → LED power supply → LEDs

## Firmware overview
- `src/main.cpp` connects to Wi‑Fi, advertises mDNS, and exposes HTTP endpoints to toggle and query the relay.
- Endpoints:
  - `GET /light/on`
  - `GET /light/off`
  - `GET /light/status`

## Configuration and secrets
- Wi‑Fi credentials are injected at build time from `.env` via `env-extra.py`.
- `.env` is ignored by git; `.env.example` is the template.
- Do **not** add secrets to source files.

## How to contribute effectively
- Keep changes small and reviewable; explain intent in commits/PRs.
- Prefer `.env` for configuration; never commit real credentials.
- If adding hardware features, update both code and docs (README + diagrams).
- For safety, do not change AC wiring assumptions without explicit review.
- Validate on hardware where possible; otherwise note what’s untested.
- Keep project-critical information in `README.md` instead of adding new planning docs.

## Code pointers
- Firmware: `src/main.cpp`
- Build config: `platformio.ini`
- Env injection: `env-extra.py`
- Diagram: `AquariumSensors.html`, `AquariumLightDiagram.png`
