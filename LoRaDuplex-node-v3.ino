#include <TimeLib.h>
#include <SPI.h>
#include <LoRa.h>
#include "data_convert.h"
#include <Adafruit_MAX31865.h>
#include <Bme280BoschWrapper.h>

Adafruit_MAX31865 max1 = Adafruit_MAX31865(7);
Adafruit_MAX31865 max2 = Adafruit_MAX31865(8);
Bme280BoschWrapper bme280(true);

byte sync_Hr, sync_Min, sync_Sec, sync_Day, sync_Mon, sync_Yer;
byte msgCount = 0;            // count of outgoing messages
const byte localAddress = 0x01;     // address of this device
const byte destination = 0x00;      // destination to send to
const byte groupID = 0x00;          // group ID to distinguish between groups
const byte sensorMode = 0;
const byte BeaconPeriod = 1;        // 2 min, BP, interval for sync time
const byte SensePeriod = 2;        // 10 sec, SP, interval for collecting sensor data
byte drift_min = floor(localAddress / 10.0); // 1-10(current minute), 11-20(next minute)
byte drift_sec = (localAddress - floor(localAddress / 10.0)) * 5; // 1-10 corresponds to 5,10,15,20...50 sec
unsigned long previousMillis = 0;
// unsigned long lastBeaconMillis = millis();

void setup() {
  initSensor();
  initLora();
  syncTime();
  LoRa.sleep();
}

void loop() {
  //*************** BeaconPeriod *************************
  // bool sync_cond1 = minute() % BeaconPeriod == 1 && second() >= 57; // sync time window [59,
  //bool sync_cond2 = minute() % BeaconPeriod == 0 && second() <= 1; // sync time window 1]
  // if (sync_cond1 || sync_cond2) {
  if (second()>=58 || second()<=1) {
    syncTime();
    LoRa.sleep();
    delay(4000);
  }

  //*************** SensePeriod *************************
  // bool send_cond1 = second() >= drift_sec && (second() - drift_sec) % SensePeriod == 0;
  // if (second() == 10 || second() == 15 || second() == 30 || second() == 50) { // time drift as ID*5
  if ( minute()%SensePeriod == 0 && (second() == 10 || second() == 50) ) {  
    sendMessage();
    delay(600);
    // shoud implement receive downlink and do if any operation
    LoRa.sleep();
  }

  //************** Beacon Watchdog **********************
/*  if ( millis()-lastBeaconMillis > 70*1000){
    syncTime();
    delay(50*1000);
    LoRa.sleep();
  }
*/
}

//////////////////////////////////////////////////////
void initSensor() {
  max1.begin(MAX31865_2WIRE);
  max2.begin(MAX31865_2WIRE);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  LoRa.setSyncWord(0xFF);
  LoRa.setSpreadingFactor(10);
  while (!bme280.beginSPI(A0))
  {
    delay(300);
  }
}

void initLora() {
  while (!LoRa.begin(435E6)) {             // initialize ratio at 915 MHz
    delay (500);                       // if failed, do nothing
  }
  LoRa.setSyncWord(0xF1);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
}

void syncTime() {
  //digitalWrite(A2, HIGH);
  bool syncTimeFlag = false;
  while (!syncTimeFlag) {
    ledBlink(300);
    if (LoRa.parsePacket() != 0) {
      byte recipient = LoRa.read();         // recipient address
      byte group = LoRa.read();             // common group ID
      if ((recipient == localAddress || recipient == 0xFF) && (group == groupID)) { // 0xFF reserved for broadcast
        byte sender = LoRa.read();            // sender address
        byte incomingMsgId = LoRa.read();     // incoming msg ID
        sync_Hr =  LoRa.read();
        sync_Min = LoRa.read();
        sync_Sec = LoRa.read();
        sync_Day = LoRa.read();
        sync_Mon = LoRa.read();
        sync_Yer = LoRa.read();
        setTime(sync_Hr, sync_Min, sync_Sec + 1, sync_Day, sync_Mon, sync_Yer); // set time 1 sec ahead because of delay
//        lastBeaconMillis = millis();
        syncTimeFlag = true;
      }
    }
  }
  digitalWrite(A2, LOW);
}

void sendMessage() {
  digitalWrite(A2, HIGH);
  float bmeT, bmeH, bmeP, bmeA;
  byte bmeTint, bmeTdec, bmeHint, bmeHdec, bmePint, bmePdec, bmeAint, bmeAdec, max1int, max1dec, max2int, max2dec;
  if (bme280.measure()) {
    bmeT = bme280.getTemperature() / 100.0;
    bmeH = bme280.getHumidity() / 1024.0;
    bmeP = bme280.getPressure() / 1000.0;
    bmeA = 44330.0 * (1.0 - pow((bme280.getPressure() / 100.0F) / 1013.25, 0.1903));
  }
  float2byte(bmeTint, bmeTdec, bmeT);
  float2byte(bmeHint, bmeHdec, bmeH);
  float2byte(bmePint, bmePdec, bmeP);
  float2byte(bmeAint, bmeAdec, bmeA);
  float2byte(max1int, max1dec, max1.temperature(100.0, 430.0));
  float2byte(max2int, max2dec, max2.temperature(100.0, 430.0));
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(groupID);                  // add group ID
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount++);                 // add message ID
  LoRa.write(sensorMode); // indicate sensor mode (how many sensors, how they list in data structure)
  LoRa.write(bmeTint);
  LoRa.write(bmeTdec);
  LoRa.write(bmeHint);
  LoRa.write(bmeHdec);
  LoRa.write(bmePint);
  LoRa.write(bmePdec);
  LoRa.write(bmeAint);
  LoRa.write(bmeAdec);
  LoRa.write(max1int);
  LoRa.write(max1dec);
  LoRa.write(max2int);
  LoRa.write(max2dec);
  LoRa.endPacket();                     // finish packet and send it
  digitalWrite(A2, LOW);
}

void ledBlink(int interval){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(A2, !digitalRead(A2));
  }
}
