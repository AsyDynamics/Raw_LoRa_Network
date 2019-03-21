#include "ThingsBoard.h"
#include <WiFiEspClient.h>
#include <WiFi.h>
#define WIFI_AP             "ZiggoF5A5142"
#define WIFI_PASSWORD       "Yhvsh6veykxy"

#define TOKEN               "LGrAtTvGL05JmhXolhxd"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

WiFiEspClient espClient;
ThingsBoard tb(espClient);

void setup() {
  Serial.begin(115220);
  WiFi.init();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
}

const size_t callbacks_size = 2;

RPC_Callback callbacks[callbacks_size] = {
  { "example_set_temperature",    processTemperatureChange },
  { "example_set_switch",         processSwitchChange }
};

bool subscribed = false;

void loop() {
  delay(100);
  if (WiFi.status() == WL_IDLE_STATUS) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to AP ...");
    Serial.println(WIFI_AP);
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    return;
  }

  if (!tb.connected()) {
    subscribed = false;
    Serial.print("Connecting to: ");
    Serial.println(THINGSBOARD_SERVER);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect, retrying ...");
      return;
    }
  }

  if (!subscribed) {
    Serial.println("Subscribing for RPC...");
    if (!tb.RPC_Subscribe(callbacks, callbacks_size)) {
      Serial.println("Failed to subscribe for RPC");
      return;
    }
    Serial.println("Subscribe done");
    subscribed = true;
  }
  Serial.println("Waiting for data...");
  tb.loop();
}


RPC_Response processTemperatureChange(const RPC_Data &data)
{
  Serial.println("Received the set temperature RPC method");
  float example_temperature = data["temp"];
  Serial.print("Example temperature: ");
  Serial.println(example_temperature);
  return RPC_Response("example_response", 42);
}

RPC_Response processSwitchChange(const RPC_Data &data)
{
  Serial.println("Received the set switch method");
  bool switch_state = data["switch"];
  Serial.print("Example switch state: ");
  Serial.println(switch_state);
  return RPC_Response("example_response", 22.02);
}
