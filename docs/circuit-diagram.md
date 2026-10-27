# Circuit Diagram

- ESP32 Dev Board target: ESP32-WROOM-32 (`esp32dev`)
- Relay control pin: `GPIO25`
- DS18B20 data pin: `GPIO26`
- DS18B20 pull-up: `4.7k` from `DATA` to `3.3V`

```mermaid
flowchart LR
  subgraph LV["Low-Voltage Control (inside enclosure)"]
    USB["USB-C 5V"] --> ESP32["ESP32 Dev Board\n(ESP32-WROOM-32)"]

    ESP32 -->|"3.3V"| V3["3.3V Rail"]
    ESP32 -->|"GND"| GND["GND Rail"]

    ESP32 -->|"GPIO25"| RIN["Relay IN"]
    V3 --> RVCC["Relay VCC"]
    GND --> RGND["Relay GND"]

    V3 --> TVDD["DS18B20 VDD"]
    GND --> TGND["DS18B20 GND"]
    ESP32 -->|"GPIO26"| TDATA["DS18B20 DATA"]

    V3 -->|"4.7k pull-up"| TDATA
  end

  subgraph HV["Light Power Path (outside enclosure)"]
    OUTLET["AC Outlet"] --> SW["Manual Switch"]
    SW --> COM["Relay COM"]
    NO["Relay NO"] --> PSU["LED Power Supply"]
    PSU --> LEDS["Aquarium LEDs"]
  end

  RIN -. "Controls relay coil" .-> RELAY["Relay Module"]
  RVCC --> RELAY
  RGND --> RELAY
  COM --> RELAY
  RELAY --> NO
```

## Wiring Notes

- Manual switch remains the final authority for lights; if manual switch is OFF, lights stay OFF regardless of ESP32 state.
- The relay module sits adjacent to the proto board in the enclosure.
- Keep DS18B20 `DATA` connected directly to `GPIO26`; do not place the resistor in series.
- The 4.7k resistor must bridge `DATA` to `3.3V`.

## DS18B20 Probe Color Mapping

Use your probe vendor pinout as source-of-truth. Typical waterproof probe colors are:

- `Red` -> `VDD (3.3V)`
- `Black` -> `GND`
- `Yellow` or `White` -> `DATA`
