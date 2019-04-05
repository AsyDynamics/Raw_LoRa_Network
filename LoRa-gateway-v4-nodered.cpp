#include "gateway_v4.h"

void setup() {
  InitWiFi();
  initLora();
  configMQTTclient();
  configNTP();
}

void loop() {
  run();
}
