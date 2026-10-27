#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

// Fallback defaults if .env vars aren't injected
#ifndef WIFI_SSID
#define WIFI_SSID "your_default_ssid"
#warning "WIFI_SSID not defined, using fallback default"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_default_password"
#warning "WIFI_PASSWORD not defined, using fallback default"
#endif

// Hardware pin map (ESP32 Dev Board)
#ifndef RELAY_PIN
#define RELAY_PIN 25
#endif

#ifndef TEMP_SENSOR_PIN
#define TEMP_SENSOR_PIN 26
#endif

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
bool lightOn = false;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature tempSensors(&oneWire);
float lastTempC = NAN;
bool tempProbePresent = false;
unsigned long lastTempReadMs = 0;
const unsigned long TEMP_REFRESH_MS = 5000;

String buildStatusJson() {
  String payload = "{";
  payload += "\"light\":\"";
  payload += (lightOn ? "ON" : "OFF");
  payload += "\",\"lightOn\":";
  payload += (lightOn ? "true" : "false");
  payload += ",\"tempProbePresent\":";
  payload += (tempProbePresent ? "true" : "false");
  payload += ",\"temperatureC\":";
  payload += (tempProbePresent ? String(lastTempC, 2) : "null");
  payload += ",\"temperatureF\":";
  payload += (tempProbePresent ? String((lastTempC * 9.0f / 5.0f) + 32.0f, 2) : "null");
  payload += ",\"relayPin\":";
  payload += String(RELAY_PIN);
  payload += ",\"tempSensorPin\":";
  payload += String(TEMP_SENSOR_PIN);
  payload += ",\"uptimeMs\":";
  payload += String(millis());
  payload += ",\"temperatureUpdatedMs\":";
  payload += String(lastTempReadMs);
  payload += ",\"wifiRssiDbm\":";
  payload += String(WiFi.RSSI());
  payload += ",\"ip\":\"";
  payload += WiFi.localIP().toString();
  payload += "\"}";
  return payload;
}

String buildHomebridgePayloadJson() {
  String payload = "{";
  payload += "\"accessory\":\"AquariumController\",";
  payload += "\"services\":{";
  payload += "\"lightbulb\":{\"on\":";
  payload += (lightOn ? "true" : "false");
  payload += "},";
  payload += "\"temperatureSensor\":{\"statusActive\":";
  payload += (tempProbePresent ? "true" : "false");
  payload += ",\"currentTemperatureC\":";
  payload += (tempProbePresent ? String(lastTempC, 2) : "null");
  payload += "}";
  payload += "},";
  payload += "\"meta\":{";
  payload += "\"uptimeMs\":";
  payload += String(millis());
  payload += ",\"temperatureUpdatedMs\":";
  payload += String(lastTempReadMs);
  payload += ",\"wifiRssiDbm\":";
  payload += String(WiFi.RSSI());
  payload += "}";
  payload += "}";
  return payload;
}

void refreshTemperature(bool forceRead = false) {
  unsigned long now = millis();
  if (!forceRead && (now - lastTempReadMs) < TEMP_REFRESH_MS) {
    return;
  }

  tempSensors.requestTemperatures();
  float tempC = tempSensors.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C || tempC < -55.0f || tempC > 125.0f) {
    tempProbePresent = false;
  } else {
    tempProbePresent = true;
    lastTempC = tempC;
  }
  lastTempReadMs = now;
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // relay OFF initially

  tempSensors.begin();
  tempSensors.setWaitForConversion(true);
  refreshTemperature(true);

  Serial.println("Relay initialized.");
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected!");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("aquarium-lights")) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started at http://aquarium-lights.local");
  }

  server.on("/light/on", []() {
    digitalWrite(RELAY_PIN, HIGH);
    lightOn = true;
    server.send(200, "text/plain", "Light turned ON");
  });

  server.on("/light/off", []() {
    digitalWrite(RELAY_PIN, LOW);
    lightOn = false;
    server.send(200, "text/plain", "Light turned OFF");
  });

  server.on("/light/status", []() {
    server.send(200, "text/plain", lightOn ? "ON" : "OFF");
  });

  server.on("/temperature/c", []() {
    refreshTemperature(true);
    if (!tempProbePresent) {
      server.send(503, "text/plain", "Temperature probe unavailable");
      return;
    }
    server.send(200, "text/plain", String(lastTempC, 2));
  });

  server.on("/temperature/f", []() {
    refreshTemperature(true);
    if (!tempProbePresent) {
      server.send(503, "text/plain", "Temperature probe unavailable");
      return;
    }
    float tempF = (lastTempC * 9.0f / 5.0f) + 32.0f;
    server.send(200, "text/plain", String(tempF, 2));
  });

  server.on("/status/json", []() {
    refreshTemperature(true);
    server.send(200, "application/json", buildStatusJson());
  });

  server.on("/homebridge/payload", []() {
    refreshTemperature(true);
    server.send(200, "application/json", buildHomebridgePayloadJson());
  });

  server.begin();
}

void loop() {
  refreshTemperature(false);
  server.handleClient();
}
