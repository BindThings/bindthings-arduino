#ifndef BINDTHINGS_H
#define BINDTHINGS_H

#include <Arduino.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecureBearSSL.h>
#else
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#endif

#include <PubSubClient.h>
#include <ArduinoJson.h>

#define BT_BROKER  "mqtt.bindthings.io"
#define BT_PORT    8883
#define BT_VERSION "1.0.0"

typedef void (*CommandCallback)(const String& payload);

class BindThings {
public:
  BindThings(const char* token);

  bool connect(const char* ssid, const char* password);
  bool loop();

  bool send(const char* key, float value);
  bool send(const char* key, int value);
  bool send(const char* key, bool value);
  bool send(const char* key, const String& value);
  bool send(const String& jsonPayload);

  void onCommand(CommandCallback cb);
  bool setOnline(bool online = true);
  bool isConnected();

private:
  const char* _token;
  char _topicTelemetry[80];
  char _topicStatus[80];
  char _topicCommands[80];
  char _clientId[32];

#ifdef ESP8266
  BearSSL::WiFiClientSecure _wifiClient;
#else
  WiFiClientSecure _wifiClient;
#endif

  PubSubClient    _mqttClient;
  CommandCallback _commandCb;

  bool _connectMQTT();
  static void _mqttCallback(char* topic, byte* payload, unsigned int length);
  static BindThings* _instance;
  void _handleMessage(char* topic, byte* payload, unsigned int length);
};

#endif
