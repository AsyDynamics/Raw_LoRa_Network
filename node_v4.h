#ifndef NODE_V4_H
#define NODE_V4_H
#include <math.h>
#include <SPI.h>
#include <LoRa.h>
#include "LowPower.h"

byte sync_Hr, sync_Min, sync_Sec;
int msgCount = 0;
unsigned long previousMiilis = 0;

// node configuration
#define localAddr         3
#define groupID           0
#define destination       0 // gateway
#define broadcastAddr     0xFF
#define sensorMode        0

// operation configuration
#define preBP             2
#define preSP             1
#define SP_m              1
#define SP_h              1
#define SP_offset_m       byte(ceil(localAddr/10.0)-1)
#define SP_offset_s       ((localAddr-1)%10+1)*5
#define senseMode         0

// timeout configuration
#define initLoraTimeout   5
#define downlinkTimeout   3
#define debugTimeout      120

// sensor configuration
#define led_pin           A0
#define oneWire_pin       A0
#define sensor_cs_pin     A0

// lora configuration
#define lora_freq         435E6
#define lora_syncword     0xF1
#define lora_bandwidth    125E3
#define lora_sf           10
#define lora_power_h      17
#define lora_power_l      12

void initSensor(){
  // dummy demo
}

byte setMode(){
  return 1; // dummy demo, 0-debug/lora test, 1-lowTxPower, 2-highTxPower, 3-senseOnly
}

void readSensor(float &val1, float &val2, float &val3){
  // dummy demo
}

void readDownlink(byte &command1, byte &command2){
  // dummy demo
}

void callback(command1, command2){
  // dummy demo
}

void initLora() {
  while (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    delay (300);                       // if failed, do nothing
  }
  LoRa.setSyncWord(lora_syncword);
  LoRa.setSpreadingFactor(lora_sf);
  LoRa.setSignalBandwidth(lora_bandwidth);
  LoRa.setTxPower(lora_power_l);
}



void syncTime() {
  bool syncTimeFlag = false;
  while (!syncTimeFlag) {
    if (LoRa.parsePacket() != 0) {
      byte recipient = LoRa.read();         // recipient address
      byte group = LoRa.read();             // common group ID
      if ((recipient == localAddr || recipient == broadcastAddr) && (group == groupID)) { // 0xFF reserved for broadcast
        byte sender = LoRa.read();            // sender address
        byte incomingMsgId = LoRa.read();     // incoming msg ID
        sync_Hr =  LoRa.read();
        sync_Min = LoRa.read();
        // sync_Sec = LoRa.read();
        // byte sync_Day = LoRa.read();
        // byte sync_Mon = LoRa.read();
        // byte sync_Yer = LoRa.read();
        syncTimeFlag = true;
      }
    }
  }
}

void sendMessage(byte localAddress) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(groupID);                  // add group ID
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount++);                 // add message ID
  LoRa.write(sensorMode); // indicate sensor mode (how many sensors, how they list in data structure)
  LoRa.write(random(15, 30));
  LoRa.write(random(0, 99));
  LoRa.write(random(15, 55));
  LoRa.write(random(0, 99));
  LoRa.write(random(98, 102));
  LoRa.write(random(0, 99));
  LoRa.write(random(45, 125));
  LoRa.write(random(0, 99));
  LoRa.write(random(15, 55));
  LoRa.write(random(0, 99));
  LoRa.write(random(15, 55));
  LoRa.write(random(0, 99));
  LoRa.endPacket();                     // finish packet and send it
}

void ledBlink(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMiilis >= interval) {
    previousMiilis = currentMillis;
    digitalWrite(led_pin, !digitalRead(led_pin));
  }
}

void enter_sleep(float sleep_t) {
  byte counter8 = byte(sleep_t / 8);
  for (byte i = 0; i < counter8; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  for (byte i = 0; i < byte(sleep_t - counter8 * 8); i++) {
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  }
  delay((sleep_t - byte(sleep_t)) * 1000);
}

void run() {
  if ( sync_Hr % SP_h == 0 && (sync_Min - SP_offset_m) % SP_m == 0) { // at that hour, and that(SP) minute
    enter_sleep(SP_offset_s - preSP);                          // sleep to SP, then wake up and do some stuff
    previousMiilis = millis();
    // readSensor();
    while (millis() - previousMiilis < preSP * 1000) {
      // do nothing
      delay (50);
    }
    sendMessage(sync_Min % 5); // dummy example to mimic multi node
    bool downlinkFlag = false;
    while (millis() - previousMiilis < (preSP + downlinkTimeout) * 1000 && !downlinkFlag) {
      // readDownlink();
      // callback(); // do some stuff, maybe run a actuator
      delay (50);
      downlinkFlag = true;
    }
    // then go back to sleep again, may need to check if this is negative
    enter_sleep( 60 - preBP - (millis() - previousMiilis) / 1000 - (SP_offset_s - preSP));
  }
  else {
    enter_sleep(60 - preBP); // sleep to next BP
  }
  syncTime();
}

#endif
