/*
  BindThings DHT22 Example
  ─────────────────────────
  Reads temperature and humidity from DHT22 sensor
  and sends to BindThings every 15 seconds.

  Wiring:
    DHT22 DATA → D5 (GPIO14 on NodeMCU)
    DHT22 VCC  → 3.3V
    DHT22 GND  → GND

  Required libraries:
    - BindThings
    - DHT sensor library by Adafruit
    - PubSubClient
    - ArduinoJson
*/

#include <BindThings.h>
#include <DHT.h>

#define WIFI_SSID    "YOUR_WIFI_SSID"
#define WIFI_PASS    "YOUR_WIFI_PASSWORD"
#define DEVICE_TOKEN "YOUR_DEVICE_TOKEN"

#define DHT_PIN  D5
#define DHT_TYPE DHT22

BindThings bt(DEVICE_TOKEN);
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastSend = 0;
const unsigned long INTERVAL = 15000; // FREE plan: 15s

void onCommand(const String& payload) {
  Serial.println("Command: " + payload);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  bt.onCommand(onCommand);
  bt.connect(WIFI_SSID, WIFI_PASS);
}

void loop() {
  bt.loop();

  if (millis() - lastSend >= INTERVAL) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("[DHT22] Read error");
      return;
    }

    // Build JSON and send
    String payload = "{\"temperature\":" + String(temp, 1) +
                     ",\"humidity\":"    + String(hum,  1) + "}";
    bt.send(payload);
  }
}
