# BindThings Arduino Library

Official Arduino library for the [BindThings](https://bindthings.com) IoT Platform.

Connect your ESP32, ESP8266, or Arduino to BindThings with just a few lines of code.

## Installation

### Arduino Library Manager
1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for `BindThings`
4. Click **Install**

### Manual
1. Download this repository as ZIP
2. Go to **Sketch → Include Library → Add .ZIP Library**
3. Select the downloaded ZIP

## Dependencies
Install these libraries from Library Manager:
- [PubSubClient](https://github.com/knolleary/pubsubclient) by Nick O'Leary
- [ArduinoJson](https://arduinojson.org) by Benoit Blanchon

## Quick Start

```cpp
#include <BindThings.h>

BindThings bt("YOUR_DEVICE_TOKEN");

void setup() {
  bt.connect("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
}

void loop() {
  bt.loop();
  bt.send("temperature", 25.5);
  delay(15000);
}
```

## API Reference

### Constructor
```cpp
BindThings bt(token);
```

### Connect
```cpp
bt.connect(ssid, password);  // Connect to WiFi and MQTT
```

### Send Telemetry
```cpp
bt.send("temperature", 25.5);        // float
bt.send("humidity", 60);             // int
bt.send("relay", true);              // bool
bt.send("status", "active");         // String
bt.send("{\"temp\":25.5,\"hum\":60}"); // Raw JSON
```

### Receive Commands
```cpp
void onCommand(const String& payload) {
  Serial.println("Command: " + payload);
}

bt.onCommand(onCommand);
```

### Status
```cpp
bt.setOnline(true);   // Mark device as online
bt.setOnline(false);  // Mark device as offline
bt.isConnected();     // Check MQTT connection
```

### Loop
```cpp
void loop() {
  bt.loop(); // Must be called in loop()
}
```

## Examples

| Example | Description |
|---------|-------------|
| [Basic](examples/Basic/Basic.ino) | Send temperature and humidity every 15 seconds |
| [DHT22](examples/DHT22/DHT22.ino) | Read from DHT22 sensor and send to BindThings |

## Connection Details

| Parameter | Value |
|-----------|-------|
| Broker | `mqtt.bindthings.io` |
| Port | `8883` (TLS) |
| Username | Your Device Token |
| Password | Your Device Token |

## Supported Hardware

| Board | Status |
|-------|--------|
| ESP32 | ✅ Tested |
| ESP8266 (NodeMCU) | ✅ Tested |
| Arduino + Ethernet Shield | ⚠️ Coming soon |

## License

MIT License — © 2025 BindThings
