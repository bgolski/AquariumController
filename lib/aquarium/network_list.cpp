#include "network_list.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <unordered_set>

namespace aquarium {

int signalBars(int rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

std::vector<Network> networksToOffer(const std::vector<Network>& scan, std::size_t limit) {
  std::vector<Network> filtered;
  filtered.reserve(scan.size());
  for (const auto& n : scan) {
    if (!n.ssid.empty()) filtered.push_back(n);
  }

  std::sort(filtered.begin(), filtered.end(), [](const Network& a, const Network& b) {
    if (a.rssi != b.rssi) return a.rssi > b.rssi; // stronger first
    return a.ssid < b.ssid; // byte order
  });

  std::vector<Network> result;
  result.reserve(limit);
  std::unordered_set<std::string> seen;
  for (const auto& n : filtered) {
    if (seen.find(n.ssid) != seen.end()) continue;
    seen.insert(n.ssid);
    result.push_back(n);
    if (result.size() == limit) break;
  }
  return result;
}

std::string jsonString(const std::string& s) {
  std::string out;
  out.push_back('"');
  for (unsigned char u : s) {
    switch (u) {
      case '"': out.append("\\\""); break;
      case '\\': out.append("\\\\"); break;
      case '\n': out.append("\\n"); break;
      case '\r': out.append("\\r"); break;
      case '\t': out.append("\\t"); break;
      default:
        if (u < 0x20) {
          char buf[8];
          std::snprintf(buf, sizeof buf, "\\u%04x", u);
          out.append(buf);
        } else {
          out.push_back(u);
        }
    }
  }
  out.push_back('"');
  return out;
}

std::string networksJson(const std::vector<Network>& networks) {
  std::string out;
  out.push_back('[');
  bool first = true;
  for (const auto& n : networks) {
    if (!first) out.push_back(',');
    first = false;
    out.append("{\"ssid\":");
    out.append(jsonString(n.ssid));
    out.append(",\"bars\":");
    out.append(std::to_string(signalBars(n.rssi)));
    out.append(",\"secure\":");
    out.append(n.secure ? "true" : "false");
    out.push_back('}');
  }
  out.push_back(']');
  return out;
}

} // namespace aquarium
