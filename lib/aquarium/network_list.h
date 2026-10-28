#pragma once

// Wi-Fi networks shown on the setup page.

#include <cstddef>
#include <string>
#include <vector>

namespace aquarium {

struct Network {
  std::string ssid;
  int rssi;  // dBm, e.g. -60
  bool secure;
};

// Signal strength in bars, 0 to 4: 4 at -55 dBm or stronger, 3 at -65 or
// stronger, 2 at -75 or stronger, 1 at -85 or stronger, otherwise 0.
int signalBars(int rssi);

// The networks to offer: drops entries with an empty name, keeps one entry per
// name (the one with the strongest signal), sorts strongest first and, for
// equal signal, by name in byte order, then keeps at most `limit`.
std::vector<Network> networksToOffer(const std::vector<Network>& scan, std::size_t limit = 20);

// `s` as a JSON string literal, quotes included. Escapes `"` and `\` with a
// backslash, newline, carriage return and tab as \n \r \t, other bytes below
// 0x20 as \u00XX with lowercase hex; every other byte is copied unchanged.
std::string jsonString(const std::string& s);

// The networks as a JSON array with no spaces, in the given order:
// [{"ssid":"Home","bars":4,"secure":true},{"ssid":"Cafe","bars":1,"secure":false}]
std::string networksJson(const std::vector<Network>& networks);

}  // namespace aquarium
