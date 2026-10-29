#include "setup_portal.h"

#include <string.h>

#include <vector>

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_ota_ops.h>
#include <esp_wifi.h>

#include "network_list.h"
#include "reconnect.h"
#include "setup_fallback.h"
#include "setup_page.h"  // generated from web/setup.html by web/build_page.py

namespace {

// How long a password test may take before it counts as failed.
const unsigned long TEST_TIMEOUT_MS = 25000;
// After a new network works, the setup network stays up this long (or until
// Done) so the page can show the result.
const unsigned long RESULT_HOLD_MS = 120000;
// Firmware sent over Wi-Fi that has not connected by then is rolled back.
const unsigned long ROLLBACK_AFTER_MS = 600000;
// A setup network opened on request closes after this long unused.
const unsigned long REQUEST_OPEN_MS = 600000;

enum class Test { Idle, Connecting, Connected, WrongPassword, NotFound, Failed };

WebServer* web = nullptr;
const char* host = nullptr;
const char* setupName = nullptr;
const char* updateUser = nullptr;
const std::string* updatePass = nullptr;

Preferences store;
String savedSsid;
String savedPass;

DNSServer dns;
aquarium::SetupFallback fallback;
aquarium::ReconnectTimer reconnectTimer;
bool portalOpen = false;
bool wasConnected = false;
bool holding = false;
unsigned long holdUntilMs = 0;
bool requested = false;
unsigned long requestedAtMs = 0;

Test test = Test::Idle;
String testSsid;
String testPass;
unsigned long testStartMs = 0;
// Written from the Wi-Fi event task.
volatile uint8_t testFailures = 0;
volatile uint8_t lastReason = 0;

void onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  if (event != ARDUINO_EVENT_WIFI_STA_DISCONNECTED) return;
  uint8_t reason = info.wifi_sta_disconnected.reason;
  // Leaving a network on purpose (e.g. to try another one) is not a failure.
  if (reason == WIFI_REASON_ASSOC_LEAVE) return;
  lastReason = reason;
  testFailures = testFailures + 1;
}

String fixedString(const uint8_t* bytes, size_t size) {
  size_t len = strnlen(reinterpret_cast<const char*>(bytes), size);
  String s;
  s.concat(reinterpret_cast<const char*>(bytes), len);
  return s;
}

// The saved network lives in this firmware's own settings. Earlier firmware
// left it in the Wi-Fi driver's storage, so it is copied over once. After
// that the driver keeps nothing in flash, so testing a password never
// replaces the saved network.
void loadSavedNetwork() {
  store.begin("wifi", false);
  savedSsid = store.getString("ssid", "");
  savedPass = store.getString("pass", "");
  if (savedSsid.isEmpty()) {
    wifi_config_t conf;
    if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK && conf.sta.ssid[0]) {
      savedSsid = fixedString(conf.sta.ssid, sizeof(conf.sta.ssid));
      savedPass = fixedString(conf.sta.password, sizeof(conf.sta.password));
      store.putString("ssid", savedSsid);
      store.putString("pass", savedPass);
    }
  }
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
}

void join(const String& ssid, const String& pass) {
  WiFi.begin(ssid.c_str(), pass.isEmpty() ? nullptr : pass.c_str());
}

void joinSaved() {
  if (savedSsid.isEmpty()) {
    WiFi.disconnect();
  } else {
    join(savedSsid, savedPass);
  }
}

void openPortal() {
  // Start on the channel of the likeliest network to be chosen (the saved
  // one, else the strongest), so the phone usually stays connected when the
  // board joins it. The scan also fills the page's first network list.
  int channel = 1;
  int strongest = -1000;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == savedSsid) {
      channel = WiFi.channel(i);
      break;
    }
    if (WiFi.RSSI(i) > strongest) {
      strongest = WiFi.RSSI(i);
      channel = WiFi.channel(i);
    }
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(setupName, nullptr, channel);
  dns.start(53, "*", WiFi.softAPIP());
  portalOpen = true;
  Serial.printf("Setup network %s open on channel %d.\n", setupName, channel);
}

void closePortal() {
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  portalOpen = false;
  holding = false;
  test = Test::Idle;
  Serial.println("Setup network closed.");
}

// Requests to the setup endpoints are only answered on the setup network.
bool onSetupNetwork() {
  return portalOpen && web->client().localIP() == WiFi.softAPIP();
}

void sendJson(int code, const std::string& json) {
  web->sendHeader("Cache-Control", "no-store");
  web->send(code, "application/json", json.c_str());
}

void handleRoot() {
  if (!onSetupNetwork()) {
    web->send(404, "text/plain", "Not found");
    return;
  }
  web->sendHeader("Content-Encoding", "gzip");
  web->sendHeader("Cache-Control", "no-store");
  web->send_P(200, "text/html", reinterpret_cast<const char*>(SETUP_PAGE_GZ), SETUP_PAGE_GZ_LEN);
}

// Answers 202 while a scan is running; the page asks again.
void handleNetworks() {
  if (!onSetupNetwork()) return web->send(404, "text/plain", "Not found");
  if (web->hasArg("rescan")) WiFi.scanDelete();
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_FAILED) {
    WiFi.scanNetworks(true);
    n = WIFI_SCAN_RUNNING;
  }
  if (n < 0) return sendJson(202, "[]");
  std::vector<aquarium::Network> scan;
  for (int i = 0; i < n; i++) {
    scan.push_back({WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i) != WIFI_AUTH_OPEN});
  }
  sendJson(200, aquarium::networksJson(aquarium::networksToOffer(scan)));
}

void handleConnect() {
  if (!onSetupNetwork()) return web->send(404, "text/plain", "Not found");
  String ssid = web->arg("ssid");
  String pass = web->arg("password");
  if (ssid.isEmpty() || ssid.length() > 32) return sendJson(400, "{}");
  testSsid = ssid;
  testPass = pass;
  sendJson(202, "{}");
  if (!pass.isEmpty() && (pass.length() < 8 || pass.length() > 63)) {
    test = Test::WrongPassword;  // WPA passwords are 8 to 63 characters
    return;
  }
  Serial.printf("Trying %s.\n", ssid.c_str());
  test = Test::Connecting;
  testStartMs = millis();
  testFailures = 0;
  lastReason = 0;
  join(testSsid, testPass);
}

const char* testState() {
  switch (test) {
    case Test::Connecting: return "connecting";
    case Test::Connected: return "connected";
    case Test::WrongPassword: return "wrong_password";
    case Test::NotFound: return "not_found";
    case Test::Failed: return "failed";
    default: return "idle";
  }
}

void handleStatus() {
  if (!onSetupNetwork()) return web->send(404, "text/plain", "Not found");
  std::string json = std::string("{\"state\":\"") + testState() + "\",\"ssid\":" +
                     aquarium::jsonString(testSsid.c_str());
  if (test == Test::Connected) {
    json += std::string(",\"host\":") + aquarium::jsonString(std::string(host) + ".local") +
            ",\"ip\":" + aquarium::jsonString(WiFi.localIP().toString().c_str()) +
            ",\"updatePassword\":" + aquarium::jsonString(*updatePass);
  }
  sendJson(200, json + "}");
}

// Opens the setup network from the home network, e.g. to move the board to
// another network without reaching its BOOT button.
void handleOpen() {
  if (!web->authenticate(updateUser, updatePass->c_str())) {
    return web->requestAuthentication(DIGEST_AUTH, host);
  }
  requested = true;
  requestedAtMs = millis();
  web->send(202, "text/plain", "Setup network opening");
}

void handleDone() {
  if (!onSetupNetwork()) return web->send(404, "text/plain", "Not found");
  sendJson(200, "{}");
  requested = false;
  if (holding) holdUntilMs = millis() + 1000;  // long enough for the reply to go out
}

// Everything else on the setup network goes to the setup page, which makes
// phones open it on their own when they join (captive portal).
void handleNotFound() {
  if (onSetupNetwork()) {
    web->sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/");
    web->send(302, "text/plain", "");
    return;
  }
  web->send(404, "text/plain", "Not found");
}

Test failedTest() {
  switch (lastReason) {
    case WIFI_REASON_NO_AP_FOUND:
      return Test::NotFound;
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_MIC_FAILURE:
    case WIFI_REASON_802_1X_AUTH_FAILED:
      return Test::WrongPassword;
    default:
      return Test::Failed;
  }
}

// Settles a password test: on success the network is saved; on failure the
// board goes back to the saved network, if any.
void checkTest(bool connected, unsigned long now) {
  if (test != Test::Connecting) return;
  if (connected) {
    savedSsid = testSsid;
    savedPass = testPass;
    store.putString("ssid", savedSsid);
    store.putString("pass", savedPass);
    test = Test::Connected;
    requested = false;
    holding = true;
    holdUntilMs = now + RESULT_HOLD_MS;
    Serial.printf("Joined %s; saved.\n", savedSsid.c_str());
    return;
  }
  // The Wi-Fi driver retries once on its own, so wait for a second failure.
  bool timedOut = now - testStartMs >= TEST_TIMEOUT_MS;
  if (testFailures < 2 && !timedOut) return;
  test = failedTest();
  Serial.printf("Could not join %s (reason %u).\n", testSsid.c_str(), lastReason);
  joinSaved();
}

}  // namespace

void wifiSetupBegin(WebServer& server, const char* hostname, const char* setupNetwork,
                    const char* updateUserName, const std::string& updatePassword) {
  web = &server;
  host = hostname;
  setupName = setupNetwork;
  updateUser = updateUserName;
  updatePass = &updatePassword;

  WiFi.onEvent(onWiFiEvent);
  WiFi.setHostname(hostname);
  WiFi.mode(WIFI_STA);
  loadSavedNetwork();
  // Reconnects are paced by reconnectTimer instead, so retries for an
  // unreachable network do not keep pulling the setup network off its channel.
  WiFi.setAutoReconnect(false);
  joinSaved();
  Serial.println(savedSsid.isEmpty() ? "No saved WiFi network." : "Connecting to WiFi...");

  server.on("/", handleRoot);
  server.on("/setup/networks", HTTP_GET, handleNetworks);
  server.on("/setup/connect", HTTP_POST, handleConnect);
  server.on("/setup/status", HTTP_GET, handleStatus);
  server.on("/setup/done", HTTP_POST, handleDone);
  server.on("/setup/open", HTTP_POST, handleOpen);
  server.onNotFound(handleNotFound);
}

void wifiSetupLoop() {
  unsigned long now = millis();
  bool connected = WiFi.status() == WL_CONNECTED;

  checkTest(connected, now);
  if (test != Test::Connecting && !savedSsid.isEmpty() && reconnectTimer.due(connected, now)) {
    Serial.println("WiFi not connected, retrying...");
    WiFi.reconnect();
  }

  if (connected && !wasConnected) {
    Serial.print("Connected: ");
    Serial.println(WiFi.localIP());
    WiFi.setSleep(false);
    MDNS.end();
    MDNS.begin(host);
    // Connected, so this firmware works: keep it.
    esp_ota_mark_app_valid_cancel_rollback();
  }
  wasConnected = connected;

  // New firmware that never connects is rolled back by restarting.
  esp_ota_img_states_t state;
  if (!connected && now >= ROLLBACK_AFTER_MS &&
      esp_ota_get_state_partition(esp_ota_get_running_partition(), &state) == ESP_OK &&
      state == ESP_OTA_IMG_PENDING_VERIFY) {
    Serial.println("New firmware has not connected; rolling back.");
    ESP.restart();
  }

  if (holding && (long)(holdUntilMs - now) <= 0) holding = false;
  if (requested && test != Test::Connecting && now - requestedAtMs >= REQUEST_OPEN_MS) requested = false;
  bool wanted = fallback.update(!savedSsid.isEmpty(), connected, now) ||
                test == Test::Connecting || holding || requested;
  if (wanted && !portalOpen) openPortal();
  if (!wanted && portalOpen) closePortal();
}

void wifiForgetNetwork() {
  store.begin("wifi", false);
  store.clear();
  // Also clear the copy earlier firmware left in the Wi-Fi driver.
  esp_wifi_set_storage(WIFI_STORAGE_FLASH);
  esp_wifi_restore();
}
