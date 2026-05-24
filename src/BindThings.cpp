#include "BindThings.h"

BindThings* BindThings::_instance = nullptr;

BindThings::BindThings(const char* token)
  : _token(token), _mqttClient(_wifiClient), _commandCb(nullptr)
{
  _instance = this;
  snprintf(_topicTelemetry,  sizeof(_topicTelemetry),  "devices/%s/telemetry",  token);
  snprintf(_topicStatus,     sizeof(_topicStatus),     "devices/%s/status",     token);
  snprintf(_topicCommands,   sizeof(_topicCommands),   "devices/%s/commands",   token);
  snprintf(_topicAttributes, sizeof(_topicAttributes), "devices/%s/attributes", token);
  snprintf(_clientId,        sizeof(_clientId),        "bt_%08lX", ESP.getChipId ? (unsigned long)ESP.getChipId() : (unsigned long)esp_random());
}

bool BindThings::connect(const char* ssid, const char* password) {
  Serial.print("[BindThings] Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 15000) {
      Serial.println("\n[BindThings] WiFi timeout!");
      return false;
    }
  }
  Serial.print("\n[BindThings] WiFi connected: ");
  Serial.println(WiFi.localIP());

  return _connectMQTT();
}

bool BindThings::_connectMQTT() {
  _wifiClient.setInsecure(); // TLS without certificate validation
  _mqttClient.setServer(BT_BROKER, BT_PORT);
  _mqttClient.setCallback(_mqttCallback);
  _mqttClient.setBufferSize(512);

  // LWT payload
  String lwtPayload = "{\"online\":false,\"reason\":\"lwt\"}";

  Serial.print("[BindThings] Connecting to MQTT...");
  bool ok = _mqttClient.connect(
    _clientId,
    _token,
    _token,
    _topicStatus,
    1,
    true,
    lwtPayload.c_str()
  );

  if (ok) {
    Serial.println(" OK");
    _mqttClient.subscribe(_topicCommands);
    setOnline(true);
  } else {
    Serial.print(" FAILED, rc=");
    Serial.println(_mqttClient.state());
  }
  return ok;
}

bool BindThings::loop() {
  if (!_mqttClient.connected()) {
    Serial.println("[BindThings] MQTT disconnected, reconnecting...");
    _connectMQTT();
  }
  return _mqttClient.loop();
}

bool BindThings::send(const char* key, float value) {
  StaticJsonDocument<128> doc;
  doc[key] = value;
  String payload;
  serializeJson(doc, payload);
  return send(payload);
}

bool BindThings::send(const char* key, int value) {
  StaticJsonDocument<128> doc;
  doc[key] = value;
  String payload;
  serializeJson(doc, payload);
  return send(payload);
}

bool BindThings::send(const char* key, bool value) {
  StaticJsonDocument<128> doc;
  doc[key] = value;
  String payload;
  serializeJson(doc, payload);
  return send(payload);
}

bool BindThings::send(const char* key, const String& value) {
  StaticJsonDocument<128> doc;
  doc[key] = value;
  String payload;
  serializeJson(doc, payload);
  return send(payload);
}

bool BindThings::send(const String& jsonPayload) {
  if (!_mqttClient.connected()) return false;
  bool ok = _mqttClient.publish(_topicTelemetry, jsonPayload.c_str(), false);
  if (ok) {
    Serial.print("[BindThings] Sent: ");
    Serial.println(jsonPayload);
  }
  return ok;
}

bool BindThings::setOnline(bool online) {
  String payload = online
    ? "{\"online\":true}"
    : "{\"online\":false}";
  return _mqttClient.publish(_topicStatus, payload.c_str(), true);
}

void BindThings::onCommand(CommandCallback cb) {
  _commandCb = cb;
}

bool BindThings::isConnected() {
  return _mqttClient.connected();
}

void BindThings::_mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (_instance) {
    _instance->_handleMessage(topic, payload, length);
  }
}

void BindThings::_handleMessage(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == String(_topicCommands) && _commandCb) {
    _commandCb(msg);
  }
}
