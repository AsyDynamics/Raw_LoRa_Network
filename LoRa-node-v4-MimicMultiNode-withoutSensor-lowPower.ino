#include "node_v4.h"

void setup() {
  initSensor();
  initLora();
  setMode();
  attachInterrupt(digitalPinToInterrupt(button_pin), onRISING, RISING);
  syncTime();
  
}

void loop() {
  run();
  /*
  if ( sync_Hr % SP_H == 0 && (sync_Min - SP_offset_m) % SP_M == 0) { // at that hour, and that(SP) minute
    enter_sleep(SP_offset_s - preSP);                          // sleep to SP, then wake up and do some stuff
    previousMiilis = millis();
    // readSensor();
    while (millis() - previousMiilis < preSP * 1000) {
      // do nothing
    }
    sendMessage(sync_Min%5); // dummy example to mimic multi node
    bool downlinkFlag = false;
    while (millis() - previousMiilis < (preSP + downlinkTimeout) * 1000 && !downlinkFlag) {
      // readDownlink();
      // callback(); // do some stuff, maybe run a actuator
      downlinkFlag = true;
    }
    // then go back to sleep again, may need to check if this is negative
    enter_sleep( 60 - preBP - (millis() - previousMiilis) / 1000 - (SP_offset_s - preSP));
  }
  else {
    enter_sleep(60 - preBP); // sleep to next BP
  }
  syncTime();*/
}
