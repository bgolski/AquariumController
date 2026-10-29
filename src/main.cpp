#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
#include <Update.h>

#include "aquarium.h"
#include "relay.h"
#include "probe_schedule.h"
#include "update_password.h"
#include "network_list.h"
#include "temp_history.h"
#include "home_page.h"  // generated from web/home.html by web/build_page.py
#include "setup_portal.h"

// Hardware pin map (ESP32 Dev Board)
#ifndef RELAY_PIN
#define RELAY_PIN 25
#endif

#ifndef TEMP_SENSOR_PIN
#define TEMP_SENSOR_PIN 26
#endif

// The board's BOOT button. Holding it for WIFI_RESET_HOLD_MS while running clears
// the saved Wi-Fi network and restarts into setup.
#ifndef WIFI_RESET_PIN
#define WIFI_RESET_PIN 0
#endif

const char* HOSTNAME = "aquarium-lights";
// Open network the board broadcasts while it has no working Wi-Fi to join.
const char* SETUP_NETWORK = "Aquarium-Setup";
const unsigned long WIFI_RESET_HOLD_MS = 5000;

// User name for firmware updates; the password is generated on the board.
const char* UPDATE_USER = "admin";

WebServer server(80);
bool lightOn = false;
Preferences preferences;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensors(&oneWire);
aquarium::Temperature temperature;
unsigned long lastTempReadMs = 0;
const unsigned long TEMP_REFRESH_MS = 5000;
const unsigned long TEMP_MAX_AGE_MS = 60000;
unsigned long resetPressedSinceMs = 0;
aquarium::ProbeSchedule probe(TEMP_REFRESH_MS, 750);
bool haveValidReading = false;
unsigned long lastValidReadingMs = 0;
std::string updatePassword;
aquarium::TempHistory history;

aquarium::Snapshot snapshot() {
  aquarium::Snapshot s;
  s.lightOn = lightOn;
  s.temperature = temperature;
  s.relayPin = RELAY_PIN;
  s.tempSensorPin = TEMP_SENSOR_PIN;
  s.uptimeMs = millis();
  s.temperatureUpdatedMs = lastTempReadMs;
  s.wifiRssiDbm = WiFi.RSSI();
  s.ip = WiFi.localIP().toString().c_str();
  return s;
}

void pollTemperature() {
  switch (probe.next(millis())) {
    case aquarium::ProbeAction::Request:
      tempSensors.requestTemperatures();
      break;
    case aquarium::ProbeAction::Read:
      temperature = aquarium::applyReading(temperature, tempSensors.getTempCByIndex(0));
      lastTempReadMs = millis();
      if (temperature.present) {
        haveValidReading = true;
        lastValidReadingMs = lastTempReadMs;
        history.add(aquarium::toFahrenheit(temperature.celsius), lastTempReadMs);
      }
      break;
    default:
      break;
  }
}

// New firmware sent over Wi-Fi only stays installed once it has
// connected to Wi-Fi (see setup_portal.cpp); until then a restart rolls
// back to the previous version.
extern "C" bool verifyRollbackLater() {
  return true;
}

void checkWiFiResetButton() {
  if (digitalRead(WIFI_RESET_PIN) != LOW) {
    resetPressedSinceMs = 0;
    return;
  }
  if (resetPressedSinceMs == 0) {
    resetPressedSinceMs = millis();
  } else if (millis() - resetPressedSinceMs >= WIFI_RESET_HOLD_MS) {
    Serial.println("Clearing saved WiFi and restarting into setup.");
    wifiForgetNetwork();
    ESP.restart();
  }
}

void sendTemperature(bool fahrenheit) {
  if (!aquarium::readingFresh(haveValidReading, lastValidReadingMs, millis(), TEMP_MAX_AGE_MS)) {
    server.send(503, "text/plain", "Temperature probe unavailable");
    return;
  }
  float value = fahrenheit ? aquarium::toFahrenheit(temperature.celsius) : temperature.celsius;
  server.send(200, "text/plain", aquarium::formatTwoDecimals(value).c_str());
}

// Switches the light and remembers the choice across restarts.
void setLight(bool on) {
  lightOn = on;
  digitalWrite(RELAY_PIN, aquarium::relayLevel(on) ? HIGH : LOW);
  preferences.putInt("light", on ? 1 : 0);
}

// GET /home/status: everything the home page shows.
void sendHomeStatus() {
  bool fresh = aquarium::readingFresh(haveValidReading, lastValidReadingMs, millis(), TEMP_MAX_AGE_MS);
  std::string json = std::string("{\"light\":") + (lightOn ? "true" : "false");
  json += ",\"tempF\":" + (fresh ? aquarium::formatTwoDecimals(aquarium::toFahrenheit(temperature.celsius)) : std::string("null"));
  json += ",\"history\":" + history.json();
  json += ",\"ssid\":" + aquarium::jsonString(WiFi.SSID().c_str());
  json += ",\"bars\":" + std::to_string(aquarium::signalBars(WiFi.RSSI()));
  json += ",\"ip\":" + aquarium::jsonString(WiFi.localIP().toString().c_str());
  json += ",\"firmware\":" + aquarium::jsonString(FIRMWARE_DATE);
  json += ",\"setup\":" + std::to_string(wifiSetupSecondsLeft()) + "}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json.c_str());
}

void setup() {
  Serial.begin(115200);

  preferences.begin("aquarium", false);
  pinMode(RELAY_PIN, OUTPUT);
  setLight(aquarium::restoredLightState(preferences.getInt("light", -1)));
  pinMode(WIFI_RESET_PIN, INPUT_PULLUP);

  tempSensors.begin();
  tempSensors.setWaitForConversion(false);

  Serial.println("Relay initialized.");

  updatePassword = preferences.getString("otaPass", "").c_str();
  if (!aquarium::validPassword(updatePassword)) {
    updatePassword = aquarium::makePassword(esp_random);
    preferences.putString("otaPass", updatePassword.c_str());
  }
  Serial.print("Update password: ");
  Serial.println(updatePassword.c_str());

  wifiSetupBegin(server, HOSTNAME, SETUP_NETWORK);

  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started at http://aquarium-lights.local");
  }

  // The setup page on the setup network, the home page everywhere else.
  server.on("/", []() {
    if (wifiSetupHandleRoot()) return;
    server.sendHeader("Content-Encoding", "gzip");
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "text/html", reinterpret_cast<const char*>(HOME_PAGE_GZ), HOME_PAGE_GZ_LEN);
  });

  server.on("/home/status", HTTP_GET, sendHomeStatus);

  server.on("/light/on", []() {
    setLight(true);
    server.send(200, "text/plain", "Light turned ON");
  });

  server.on("/light/off", []() {
    setLight(false);
    server.send(200, "text/plain", "Light turned OFF");
  });

  server.on("/light/status", []() {
    server.send(200, "text/plain", lightOn ? "ON" : "OFF");
  });

  server.on("/temperature/c", []() { sendTemperature(false); });
  server.on("/temperature/f", []() { sendTemperature(true); });

  server.on("/status/json", []() {
    server.send(200, "application/json", aquarium::statusJson(snapshot()).c_str());
  });

  server.on("/homebridge/payload", []() {
    server.send(200, "application/json", aquarium::homebridgeJson(snapshot()).c_str());
  });

  // Firmware update: POST the .bin as a multipart file field, with
  // digest authentication. Restarts into the new firmware on success.
  server.on(
      "/update", HTTP_POST,
      []() {
        if (!server.authenticate(UPDATE_USER, updatePassword.c_str())) {
          server.requestAuthentication(DIGEST_AUTH, HOSTNAME);
          return;
        }
        bool ok = Update.isFinished() && !Update.hasError();
        server.sendHeader("Connection", "close");
        server.send(ok ? 200 : 500, "text/plain",
                    ok ? "Update installed, restarting" : Update.errorString());
        if (ok) {
          delay(500);
          ESP.restart();
        }
      },
      []() {
        if (!server.authenticate(UPDATE_USER, updatePassword.c_str())) {
          return;
        }
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
          Serial.println("Firmware update started.");
          Update.begin(UPDATE_SIZE_UNKNOWN);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          Update.write(upload.buf, upload.currentSize);
        } else if (upload.status == UPLOAD_FILE_END) {
          Update.end(true);
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
          Update.abort();
        }
      });

  server.begin();
}

void loop() {
  wifiSetupLoop();
  checkWiFiResetButton();
  pollTemperature();
  server.handleClient();
}
