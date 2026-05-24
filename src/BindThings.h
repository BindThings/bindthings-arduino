#ifndef BINDTHINGS_H
#define BINDTHINGS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define BT_BROKER     "mqtt.bindthings.io"
#define BT_PORT       8883
#define BT_VERSION    "1.0.0"

typedef void (*CommandCallback)(const String& payload);

class BindThings {
public:
  BindThings(const char* token);

  // WiFi + MQTT qoşul
  bool connect(const char* ssid, const char* password);

  // Yenidən qoşulmağa cəhd et (loop-da çağır)
  bool loop();

  // Tək dəyər göndər: bt.send("temperature", 25.5)
  bool send(const char* key, float value);
  bool send(const char* key, int value);
  bool send(const char* key, bool value);
  bool send(const char* key, const String& value);

  // JSON string göndər: bt.send("{\"temp\":25.5}")
  bool send(const String& jsonPayload);

  // Komanda callback: bt.onCommand(myCallback)
  void onCommand(CommandCallback cb);

  // Online/offline status göndər
  bool setOnline(bool online = true);

  bool isConnected();

private:
  const char* _token;
  char _topicTelemetry[80];
  char _topicStatus[80];
  char _topicCommands[80];
  char _topicAttributes[80];
  char _clientId[32];

  WiFiClientSecure _wifiClient;
  PubSubClient     _mqttClient;
  CommandCallback  _commandCb;

  bool _connectMQTT();
  static void _mqttCallback(char* topic, byte* payload, unsigned int length);
  static BindThings* _instance;
  void _handleMessage(char* topic, byte* payload, unsigned int length);
};

#endif
