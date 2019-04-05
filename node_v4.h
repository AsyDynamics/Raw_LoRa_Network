#ifndef NODE_V4_H
#define NODE_V4_H
#include <math.h>
#include <SPI.h>
#include <LoRa.h>
#include "LowPower.h"
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

byte sync_Hr, sync_Min, sync_Sec;
int msgCount = 0;
unsigned long previousMiilis = 0;


void run() {
  if ( sync_Hr % SP_h == 0 && (sync_Min - SP_offset_m) % SP_m == 0) { // at that hour, and that(SP) minute
    enter_sleep(SP_offset_s - preSP);                          // sleep to SP, then wake up and do some stuff
    previousMiilis = millis();
    readSensor();
    while (millis() - previousMiilis < preSP * 1000) {
      // do nothing
      delay (50);
    }
    sendMessage(sync_Min % 5); // dummy example to mimic multi node
    while (millis() - previousMiilis < (preSP + downlinkTimeout) * 1000) {
      readDownlink(LoRa.parsePacket());
      // callback(); // do some stuff, maybe run a actuator
    }
    // then go back to sleep again, may need to check if this is negative
    enter_sleep( 60 - preBP - (millis() - previousMiilis) / 1000 - (SP_offset_s - preSP));
  }
  else {
    enter_sleep(60 - preBP); // sleep to next BP
  }
  syncTime();
}

// *****************************************************
void byte2float(byte integer, byte decimal, float &value){
  value = integer + decimal/(integer<100? 100.0: 1000.0);
}

// *****************************************************
float byte2float(byte integer, byte decimal){
  return integer + decimal/(integer<100? 100.0: 1000.0);
}

// *****************************************************
void callback(command1, command2){
  // dummy demo
}

// *****************************************************
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

// *****************************************************
void float2byte(byte &integer, byte &decimal, float value){
  integer = value;
  decimal = byte((value-integer)*(value<100?100:1000));
}

// *****************************************************
void initLora() {
  while (!LoRa.begin(433E6)) {             // initialize ratio at 915 MHz
    delay (300);                       // if failed, do nothing
  }
  LoRa.setSyncWord(lora_syncword);
  LoRa.setSpreadingFactor(lora_sf);
  LoRa.setSignalBandwidth(lora_bandwidth);
  LoRa.setTxPower(lora_power_l);
}

// *****************************************************
void initSensor(){
  // dummy demo
}

// *****************************************************
void ledBlink(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMiilis >= interval) {
    previousMiilis = currentMillis;
    digitalWrite(led_pin, !digitalRead(led_pin));
  }
}

// *****************************************************
void readDownlink(byte packetSize){
  if (packetSize == 0)  return;
  byte msgLength = LoRa.available();
  if (msgLength != LoRa.read() || msgLength == 4) return; // first LoRa read should be msgLength

  byte recipient = LoRa.read();
  byte group = LoRa.read();
  if (recipient != localAddr && group != groupID) return;

  byte msgContent[msgLength-4]; // -4 rather than -3 in Gateway, because on nodeside there is no need to read sender
  for (int i = 0; i<msgLength-4; i++){ // remove the first three bytes
    msgContent[i] = LoRa.read();       // first two of the rest bytes: localAddr|sensorMode
  }
  // msgContent structure: commandMode|sw1|sw1_time|possible sw2 and sw3
  switch (msgContent[0]){ // commandMode, 0
    default: case 0:
      break;
    case 1: case 2: case 3:
      callback(msgContent[0], msgContent[1], msgContent[2]);
      break;
    case 4: case 5: case 6:
      callback(msgContent[0], msgContent[1], msgContent[2], msgContent[3], msgContent[4]);
      break;
  }
}

// *****************************************************
void readSensor(){
  // dummy demo
  return;
}

// *****************************************************
void sendMessage(byte localAddress) {
  LoRa.idle();
  LoRa.disableInvertIQ();
  LoRa.beginPacket();                   // start packet
  LoRa.write(18); // lora message length
  LoRa.write(destination);
  LoRa.write(groupID);
  LoRa.write(localAddress);
  LoRa.write(sensorMode);
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
  LoRa.enableInvertIQ();
  LoRa.receive();
}

// *****************************************************
byte setMode(){
  return 1; // dummy demo, 0-debug/lora test, 1-lowTxPower, 2-highTxPower, 3-senseOnly
}

// *****************************************************
void syncTime() {
  LoRa.enableInvertIQ();
  LoRa.receive();
  bool syncTimeFlag = false;
  while (!syncTimeFlag) {
    if (LoRa.parsePacket() != 0) {
      if (LoRa.available() == LoRa.read()){
        byte recipient = LoRa.read();
        byte group = LoRa.read();
        if ((recipient == localAddr || recipient == broadcastAddr) && (group == groupID)) {
          byte sender = LoRa.read();
          sync_Hr =  LoRa.read();
          sync_Min = LoRa.read();
          syncTimeFlag = true;
        }
      }
    }
  }
}


#endif
