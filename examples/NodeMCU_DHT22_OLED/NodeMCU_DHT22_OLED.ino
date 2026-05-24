/*
  BindThings — NodeMCU ESP8266 + DHT22 + OLED
  ─────────────────────────────────────────────
  Wiring:
    DHT22  → D5 (GPIO14)
    OLED   → D1 (SCL/GPIO5), D2 (SDA/GPIO4)  [I2C]

  Required libraries:
    - PubSubClient       by Nick O'Leary
    - ArduinoJson        by Benoit Blanchon
    - DHT sensor library by Adafruit
    - Adafruit GFX Library
    - Adafruit SSD1306

  Topics:
    Publish:   devices/{token}/telemetry  → { temperature, humidity }
               devices/{token}/status     → { online: true/false }
               devices/{token}/attributes → { sendInterval, firmware }
    Subscribe: devices/{token}/feedback
    LWT:       devices/{token}/status     → { online: false, reason: "lwt" }
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

// ── Configuration ─────────────────────────────────────────────────────────────

#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASS      "YOUR_WIFI_PASSWORD"

#define MQTT_HOST      "mqtt.bindthings.io"
#define MQTT_PORT      8883

#define DEVICE_TOKEN   "YOUR_DEVICE_TOKEN"

#define DHT_PIN        D5
#define DHT_TYPE       DHT22

#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_ADDR      0x3C

#define SEND_INTERVAL  15000   // ms — FREE plan minimum: 15s

// ── Objects ───────────────────────────────────────────────────────────────────

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);

char topicTelemetry[80];
char topicStatus[80];
char topicFeedback[80];
char topicAttributes[80];

unsigned long lastSend = 0;
String rateLimitMsg    = "";

// ── MQTT Callback ─────────────────────────────────────────────────────────────

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("[MQTT] Feedback: " + msg);
  if (msg.indexOf("rate_limit") > 0) rateLimitMsg = "Rate limited!";
}

// ── WiFi ──────────────────────────────────────────────────────────────────────

void connectWiFi() {
  Serial.print("[WiFi] Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] Connected: " + WiFi.localIP().toString());
}

// ── MQTT ──────────────────────────────────────────────────────────────────────

void connectMQTT() {
  wifiClient.setInsecure(); // TLS without certificate validation

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setBufferSize(512);

  String lwtPayload = "{\"online\":false,\"reason\":\"lwt\"}";

  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting...");
    String clientId = "bt_" + String(ESP.getChipId(), HEX);

    bool ok = mqtt.connect(
      clientId.c_str(),
      DEVICE_TOKEN,       // username = token
      DEVICE_TOKEN,       // password = token
      topicStatus,        // LWT topic
      1,                  // LWT QoS
      true,               // LWT retain
      lwtPayload.c_str()  // LWT payload
    );

    if (ok) {
      Serial.println(" OK");
      mqtt.subscribe(topicFeedback);

      // Send online status
      mqtt.publish(topicStatus, "{\"online\":true}", true);

      // Send device attributes
      char attrPayload[64];
      snprintf(attrPayload, sizeof(attrPayload),
        "{\"sendInterval\":%d,\"firmware\":\"1.0.0\"}", SEND_INTERVAL);
      mqtt.publish(topicAttributes, attrPayload, true);
      Serial.println("[MQTT] Attributes sent");
    } else {
      Serial.println(" FAILED, rc=" + String(mqtt.state()));
      delay(3000);
    }
  }
}

// ── OLED ──────────────────────────────────────────────────────────────────────

void updateDisplay(float temp, float hum, bool sending) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("  BindThings Device");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(temp, 1);
  display.print((char)247);
  display.println("C");

  display.setTextSize(2);
  display.setCursor(0, 38);
  display.print(hum, 1);
  display.println("%");

  display.setTextSize(1);
  display.setCursor(0, 56);
  if (rateLimitMsg.length() > 0) display.println(rateLimitMsg);
  else if (sending)               display.println(">> Sending...");
  else                            display.println("OK");

  display.display();
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  snprintf(topicTelemetry,  sizeof(topicTelemetry),  "devices/%s/telemetry",  DEVICE_TOKEN);
  snprintf(topicStatus,     sizeof(topicStatus),     "devices/%s/status",     DEVICE_TOKEN);
  snprintf(topicFeedback,   sizeof(topicFeedback),   "devices/%s/feedback",   DEVICE_TOKEN);
  snprintf(topicAttributes, sizeof(topicAttributes), "devices/%s/attributes", DEVICE_TOKEN);

  Wire.begin(D2, D1);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] Not found!");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 28);
  display.println("BindThings...");
  display.display();

  dht.begin();
  connectWiFi();
  connectMQTT();
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastSend >= SEND_INTERVAL) {
    lastSend = now;

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("[DHT22] Read error");
      return;
    }

    char payload[64];
    snprintf(payload, sizeof(payload),
      "{\"temperature\":%.1f,\"humidity\":%.1f}", temp, hum);

    Serial.print("[MQTT] Sending: ");
    Serial.println(payload);

    bool ok = mqtt.publish(topicTelemetry, payload, false);
    rateLimitMsg = "";
    updateDisplay(temp, hum, ok);
  } else {
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    if (!isnan(temp) && !isnan(hum)) updateDisplay(temp, hum, false);
  }

  delay(500);
}
