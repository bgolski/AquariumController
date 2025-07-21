#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// Fallback defaults if .env vars aren't injected
#ifndef WIFI_SSID
#define WIFI_SSID "your_default_ssid"
#warning "WIFI_SSID not defined, using fallback default"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_default_password"
#warning "WIFI_PASSWORD not defined, using fallback default"
#endif

#ifndef RELAY_PIN
#define RELAY_PIN 26
#endif

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WebServer server(80);
bool lightOn = false;

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // relay OFF initially

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

  server.begin();
}

void loop() {
  server.handleClient();
}