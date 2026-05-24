/*
  BindThings Basic Example
  ─────────────────────────
  Sends temperature and humidity every 15 seconds.
  
  Required libraries:
    - BindThings
    - PubSubClient
    - ArduinoJson
*/

#include <BindThings.h>

#define WIFI_SSID    "YOUR_WIFI_SSID"
#define WIFI_PASS    "YOUR_WIFI_PASSWORD"
#define DEVICE_TOKEN "YOUR_DEVICE_TOKEN"

BindThings bt(DEVICE_TOKEN);

unsigned long lastSend = 0;
const unsigned long INTERVAL = 15000; // FREE plan: 15s

void onCommand(const String& payload) {
  Serial.println("Command received: " + payload);
  // Handle command here
  // e.g. if (payload == "{\"relay\":1}") digitalWrite(RELAY_PIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  
  bt.onCommand(onCommand);
  bt.connect(WIFI_SSID, WIFI_PASS);
}

void loop() {
  bt.loop();

  if (millis() - lastSend >= INTERVAL) {
    lastSend = millis();

    // Send multiple values in one payload
    bt.send("{\"temperature\":25.5,\"humidity\":60}");

    // Or send individual values:
    // bt.send("temperature", 25.5);
    // bt.send("humidity", 60);
  }
}
