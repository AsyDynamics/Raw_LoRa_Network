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
#define sensorMode        1
#define sensorMode_test   0

// operation configuration
#define preBP             2
#define preSP             1
#define SP_m              1
#define SP_h              1
#define slotInterval      5 // slot for each node, from 2-5 seconds
#define numSlot           byte((60-slotInterval*2-preBP)/slotInterval+1) // num of slot per minute, range from 10 - 28
#define SP_offset_m       byte(ceil(localAddr*1.0/numSlot)-1)
#define SP_offset_s       ( (localAddr-1)%numSlot + 1) * slotInterval


// timeout configuration
#define initLoraTimeout   5
#define downlinkTimeout   2
#define testInterval      5
#define displayTimeout    5

// sensor configuration
#define led_pin           7
#define oneWire_pin       A0
#define sensor_cs_pin     A0
#define button_pin        3

// lora configuration
#define lora_freq         435E6
#define lora_syncword     0xF1
#define lora_bandwidth    125E3
#define lora_sf           10
#define lora_power_h      17
#define lora_power_l      12

// **********************************************************************
float byte2float(byte integer, byte decimal);
void byte2float(byte integer, byte decimal, float &value);
void callback(byte command, byte sw1, byte sw1_t);
void callback(byte command, byte sw1, byte sw1_t, byte sw2, byte sw2_t);
void callback(byte command, byte sw1, byte sw1_t, byte sw2, byte sw2_t, byte sw3, byte sw3_t);
// void displaySensor();
byte encrypt(byte val);
void enter_sleep(float sleep_t);
void float2byte(byte &integer, byte &decimal, float value);
byte getDecimal(float value);
byte getInteger(float value);
void initLora();
void initSensor();
void ledBlink(int interval);
void ledFlash(byte times);
void onFALLING();
byte readBattery();
void readDownlink(byte packetSize, bool &downlinkFlag);
void readSensor(byte *value);
void sendMsg(byte *message, byte msgLength);
void sendMsg(byte *preamble, byte preambleLength, byte *sensorValue, byte sensorLength);
void sendUplink(/*dummy multi Node*/byte localAddress, byte *sensorValue);
void setMode();
void syncTime();


// **********************************************************************
byte sync_Hr, sync_Min;
unsigned long previousMiilis = 0;

void run() {
  if ( sync_Hr % SP_h == 0 && (sync_Min - SP_offset_m) % SP_m == 0) { // at that hour, and that(SP) minute
    enter_sleep(SP_offset_s - preSP);                          // sleep to SP, then wake up and do some stuff
    byte sensorValue[12] = {}; // max 6 sensors with each 2 byte value
    readSensor(sensorValue);
    while (millis() - previousMiilis < preSP * 1000) {
      // do nothing
      delay (50);
    }
    previousMiilis = millis(); // millis at SP exactly
    sendUplink((sync_Min % 5+1), sensorValue); // dummy example to mimic multi node
    bool downlinkFlag = false;
    while (millis() - previousMiilis < (preSP + downlinkTimeout) * 1000 && !downlinkFlag) {
      readDownlink(LoRa.parsePacket(), downlinkFlag);
    }
    // then go back to sleep again, may need to check if this is negative
    enter_sleep( 60 - preBP - SP_offset_s - (millis() - previousMiilis)/1000 );
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
void callback(byte command, byte sw1, byte sw1_t){
  // dummy demo
  ledFlash(command); // blink time 1-3
}

void callback(byte command, byte sw1, byte sw1_t, byte sw2, byte sw2_t){
  // dummy demo
  ledFlash(command); // blink time 4-6
}

void callback(byte command, byte sw1, byte sw1_t, byte sw2, byte sw2_t, byte sw3, byte sw3_t){
  // dummy demo
  ledFlash(command);  // blink time 7
}

// *****************************************************
/*void displaySensor(){
  oled.set2x();
  oled.println("Node" + String(localAddr) + " Sensor Data");
  oled.set1x();
  oled.print("Temp_in:"); oled.print(String(Temp_in));
  oled.print(" Temp_out:"); oled.println(String(Temp_out));
  oled.print("Temp_env:"); oled.print(String(Temp_env));
  oled.print("Humid:"); oled.println(String(Humid));
}*/

// *****************************************************
byte encrypt(byte val){
  char key = '1'; // any
  return val^key;
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
byte getDecimal(float value){
  return byte((value-byte(value))*(value<100?100:1000));
}

// *****************************************************
byte getInterger(float value){
  return value;
}

// *****************************************************
void initLora() {
  while (!LoRa.begin(lora_freq)) {             // initialize ratio at 915 MHz
    delay (300);                       // if failed, do nothing
  }
  LoRa.setSyncWord(lora_syncword);
  LoRa.setSpreadingFactor(lora_sf);
  LoRa.setSignalBandwidth(lora_bandwidth);
  LoRa.setTxPower(lora_power_l);
}

// *****************************************************
void initSensor(){
  pinMode(button_pin, INPUT);
  digitalWrite(button_pin, HIGH); // pull up resistor
  pinMode(led_pin, OUTPUT);
  pinMode(8,OUTPUT); // ground pin for led
  digitalWrite(8, LOW);
  digitalWrite(led_pin, LOW);
}

// *****************************************************
void ledBlink(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMiilis >= interval) {
    previousMiilis = currentMillis;
    digitalWrite(led_pin, !digitalRead(led_pin));
  }
}

// ****************************************************
void ledFlash(byte times){
  while(times-- >0){
    digitalWrite(led_pin, HIGH);
    delay(150);
    digitalWrite(led_pin, LOW);
    delay(150);
  }
}

// ****************************************************
// should register interrupt in setup: attachInterrupt(digitalPinToInterrupt(button_pin), onRISING, RISING); //
void onFALLING(){
  //readSensor(); // should read sensor, display on oled
  ledFlash(3);
}

// *****************************************************
byte readBattery(){
  return rand()%10 + 80; // dummy demo
}

// *****************************************************
void readDownlink(byte packetSize){
  if (packetSize == 0)  return;
  byte msgLength = LoRa.available();
  if (msgLength != LoRa.read() || msgLength == 4) return; // first LoRa read should be msgLength
  byte recipient = LoRa.read();
  byte group = LoRa.read();
  if ( recipient != localAddr && group != groupID) return;
  byte sender = LoRa.read(); // localAddr

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
    case 7:
      callback(msgContent[0], msgContent[1], msgContent[2], msgContent[3], msgContent[4],  msgContent[5], msgContent[6]);
      break;
  }
  LoRa.sleep();
  downlinkFlag = true;
}

// *****************************************************
void readSensor(byte *value){
  switch(sensorMode){
    default: case 1:
      //float val1 = sensor(); // in total 6 sensors
      //value[0] = getInterger(val1);
      //value[1] = getDecimal(val1);
      value[0] = random(15, 30), value[1] = random(0, 99), value[2] = random(15, 55), value[3] = random(0, 99);
      value[4] = random(98, 102), value[5] = random(0, 99), value[6] = random(45, 125), value[7] = random(0, 99);
      value[8] = random(15, 55), value[9] = random(0, 99), value[10] =random(15, 55), value[11] = random(0, 99);
      break;
    case 0:
      break;
  }
}

// *****************************************************
void sendMsg(byte *message, byte msgLength){
  LoRa.beginPacket();
  for (byte i = 0; i<msgLength; i++){
    LoRa.write(message[i]);
  }
  LoRa.endPacket();
  LoRa.sleep();
}

// *****************************************************
void sendMsg(byte *preamble, byte preambleLength, byte *sensorValue, byte sensorLength){
  LoRa.beginPacket();
  for (byte i = 0; i<preambleLength; i++){
    LoRa.write(preamble[i]);
  }
  for (byte i = 0; i<sensorLength; i++){
    LoRa.write(sensorValue[i]);
  }
  LoRa.endPacket();
  LoRa.sleep();
}

// *****************************************************
void sendUplink(/*dummy multi Node*/byte localAddress, byte *sensorValue) {
  byte preambleLength = 6;  //  basic length
  switch(sensorMode){
    default: case 1:{
      byte preambleMsg[] = {preambleLength + 12, destination, groupID, localAddress, readBattery(), sensorMode};
      sendMsg(preambleMsg, preambleLength, sensorValue, 12); // 6 sensors
      break;
    }
    case 0: // no need to include case 0 as it's already implememnted in setMode()
      break;
  }
}

// *****************************************************
void setMode(){
  bool status1 = !digitalRead(button_pin);
  delay(50);
  bool status2 = !digitalRead(button_pin);
  delay(50);
  bool status3 = !digitalRead(button_pin);
  unsigned long testMillis = millis();
  byte testRSSI = 0;
  while (status1 && status2 && status3){
    byte testMode = 0;
    while( millis() - testMillis > testInterval*1000 ){
      testMillis = millis();
      byte message[] = {10, destination, groupID, localAddr, readBattery(), testMode, testRSSI, \
                        random(256), random(256), random(256)};
      sendMsg(message, 10);
    }

    if (LoRa.parsePacket()){
      if (LoRa.available() == LoRa.read()){ // first byte is msg length
        byte recipient = LoRa.read();
        byte group  = LoRa.read();
        if ( (recipient==localAddr || recipient==broadcastAddr) && group == groupID ){
          testRSSI = 0 - LoRa.packetRssi();
        }
      }
    }
  }
}

// *****************************************************
void syncTime() {
  bool syncTimeFlag = false;
  while (!syncTimeFlag) {
    if (LoRa.parsePacket() != 0) {
      if (LoRa.available() == LoRa.read()){
        if ( LoRa.read() == broadcastAddr && LoRa.read() == groupID ) {
          byte sender = LoRa.read();
          sync_Hr =  LoRa.read();
          sync_Min = LoRa.read();
          syncTimeFlag = true;
        }
      }
    }
  }
  LoRa.sleep();
}



#endif
