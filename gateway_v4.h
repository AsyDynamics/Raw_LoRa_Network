#ifndef GATEWAY_V4_H
#define GATEWAY_V4_H

#include "WiFi.h"
#include <SPI.h>
#include <LoRa.h>
#include "time.h"
#include <PubSubClient.h>

#define lora_cs_pin        5
#define lora_rst_pin	     14
#define lora_dio0_pin	     2

#define lora_freq          435E6
#define lora_syncword      0xF1
#define lora_bandwidth     125E3
#define lora_sf            10
#define lora_power_h       17
#define lora_power_l       12

#define localAddr          0
#define groupID            0
#define destination        255
#define broadcastAddr      0xFF
#define BP				         1
#define max_node_num       20 // ranging from 1 to 254;

#define WIFI_AP            "ZiggoF5A5142"
#define WIFI_PASSWORD      "Yhvsh6veykxy"

#define ntpServer		       "pool.ntp.org"
#define gmtOffset_sec	     3600
#define daylightOffset_sec 3600

#define mqtt_server		     "192.168.178.38"
// #define mqtt_username
// #define mqtt_password

struct command {
  byte commandMode = 0; // 0-no comand, 1-sw1, 2-sw2, 3-sw3, 4-sw12, 5-sw13, 6-sw23, 7-sw123
  byte sw1 = 0; // 1-min, 9-max, 0-off
  byte sw1_time = 0; // 1-9: 0.5h, 1h, 2h, 4h, 6h, 8h, 12h, 18h, 24h, 0-forever/once
  byte sw2 = 0;
  byte sw2_time = 0;
  byte sw3 = 0;
  byte sw3_time = 0;
};
// *****************************************************************************************
float byte2float(byte integer, byte decimal);
void byte2float(byte integer, byte decimal, float &value);
void callback(char* topic, byte* message, unsigned int length);
void configNTP();
void configMQTTclient();
byte encrypt(byte val);
void float2byte(byte &integer, byte &decimal, float value);
void initLora();
void initSerial();
void initWiFi();
void mqttReconnect();
void onReceive(int packetSize);
void sendBeacon();
void sendDownlink(byte nodeAddr);
void sendMsg(byte *message, byte msgLength);
void wifiReconnect();

// *****************************************************************************************
WiFiClient espClient;
PubSubClient client(espClient);
struct tm timeinfo;
struct command controlRecord[max_node_num + 1]; // pay attention, the node num start from 1 rather than 0
// *****************************************************
void run() {
  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();
  getLocalTime(&timeinfo);

  if ( timeinfo.tm_min % BP == 0 && timeinfo.tm_sec == 0) {
    Serial.println("Sending beacon");
    sendBeacon();
    delay(1000); // avoid sendin multiple beacon in one second
  }
  onReceive(LoRa.parsePacket());
}

// *****************************************************
void byte2float(byte integer, byte decimal, float &value) {
  value = integer + decimal / (integer < 100 ? 100.0 : 1000.0);
}

// *****************************************************
float byte2float(byte integer, byte decimal) {
  return integer + decimal / (integer < 100 ? 100.0 : 1000.0);
}

// *****************************************************
void callback(char* topic, byte* message, unsigned int length) {
  if (String(topic) == "group0/command") {
    Serial.println("Recv command from server");
    byte serverMsg[length];
    for (int i = 0; i < length; i++) {
      serverMsg[i] = message[i] - 48;
      Serial.print(String(byte(message[i])));
    }
    Serial.println("");
    // data structure, node num|command mode (0-8)|possible value
    byte nodeNum = serverMsg[0];
    byte commandMode = serverMsg[1];
    byte i = 2;
    controlRecord[nodeNum].commandMode = commandMode;
    switch (commandMode) {
    default : case 0:
        break;
      case 1: case 2: case 3: // single switch
        controlRecord[nodeNum].sw1 = serverMsg[i++];
        controlRecord[nodeNum].sw1_time = serverMsg[i++];
        break;
      case 4: case 5: case 6: // double switch
        controlRecord[nodeNum].sw1 = serverMsg[i++];
        controlRecord[nodeNum].sw1_time = serverMsg[i++];
        controlRecord[nodeNum].sw2 = serverMsg[i++];
        controlRecord[nodeNum].sw2_time = serverMsg[i++];
        break;
      case 7: // triple switch
        controlRecord[nodeNum].sw1 = serverMsg[i++];
        controlRecord[nodeNum].sw1_time = serverMsg[i++];
        controlRecord[nodeNum].sw2 = serverMsg[i++];
        controlRecord[nodeNum].sw2_time = serverMsg[i++];
        controlRecord[nodeNum].sw3 = serverMsg[i++];
        controlRecord[nodeNum].sw3_time = serverMsg[i++];
        break;
    }
  }
}

// *****************************************************
void configMQTTclient() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// *****************************************************
void configNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// *****************************************************
byte encrypt(byte val) {
  char key = '1'; // any
  return val ^ key;
}

// *****************************************************
void float2byte(byte &integer, byte &decimal, float value) {
  integer = value;
  decimal = byte((value - integer) * (value < 100 ? 100 : 1000));
}

// *****************************************************
void initLora() {
  LoRa.setPins(lora_cs_pin, lora_rst_pin, lora_dio0_pin);
  while (!LoRa.begin(lora_freq)) {
    delay(300);
  }
  LoRa.setSyncWord(lora_syncword);
  LoRa.setSpreadingFactor(lora_sf);
  LoRa.setSignalBandwidth(lora_bandwidth);
  LoRa.setTxPower(lora_power_h);
}

// *****************************************************
void initSerial() {
  Serial.begin(115200);
}

// *****************************************************
void initWiFi() {
  Serial.println("Connecting wifi");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connectedg wifi");
}

// *****************************************************
void mqttReconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("group0/command");
    } else {
      delay(500);
    }
  }
}

// *****************************************************
// data structure: msgLength|destination|groupID|
// localAddr|sensorMode|value
void onReceive(int packetSize) {
  if (packetSize == 0) return;
  byte msgLength = LoRa.available();
  if (msgLength != LoRa.read()) return; // first LoRa read should be msgLength
  byte recipient = LoRa.read();
  byte group = LoRa.read();
  if ( recipient != localAddr && group != groupID) return; // destination & group

  Serial.println("Received uplink");
  byte msgContent[msgLength - 3];
  for (int i = 0; i < msgLength - 3; i++) { // remove the first three bit |length,destination,group
    msgContent[i] = LoRa.read();       // first three of the rest bytes: localAddr,battery,sensorMode
    Serial.print(String(msgContent[i]));
  }
  Serial.println("");
  int loraRSSI = LoRa.packetRssi();
  // int loraSNR = LoRa.packetSnr();

  byte nodeNum = msgContent[0];
  byte battery = msgContent[1];
  byte sensorMode = msgContent[2];
  delay(50);
  sendDownlink(nodeNum); //
  controlRecord[nodeNum].commandMode = 0; // reset it to zero

  String topic = "group" + String(groupID) + "/node" + String(nodeNum); // sender
  String payload = "{";
  payload += "\"from\":"; payload += String(nodeNum); payload += ",";
  payload += "\"battery\":"; payload += String(battery); payload += ",";
  payload += "\"sensorMode\":"; payload += String(sensorMode); payload += ",";
  payload += "\"Rssi\":"; payload += String(loraRSSI); payload += ",";

  switch (sensorMode) { // read sensor mode, A-2*PT100, B-TempEnv, C-Humid, D-Pressure,
    case 0: {
        payload += "\"LoRa_test_rssi\":"; payload += String(msgContent[3]);
        payload += "}";
      }
      break;
  default: case 1: {
        byte i = 2; // first two bits: 0-localAddr, 1-sensorMode, sensor value start from 2
        float bmeT = byte2float(msgContent[i++], msgContent[i++]);
        float bmeH = byte2float(msgContent[i++], msgContent[i++]);
        float bmeP = byte2float(msgContent[i++], msgContent[i++]);
        float bmeA = byte2float(msgContent[i++], msgContent[i++]);
        float maxOne = byte2float(msgContent[i++], msgContent[i++]);
        float maxTwo = byte2float(msgContent[i++], msgContent[i++]);
        payload += "\"Temp1\":"; payload += String(maxOne); payload += ",";
        payload += "\"Temp2\":"; payload += String(maxTwo); payload += ",";
        payload += "\"TempIndoor\":"; payload += String(bmeT); payload += ",";
        payload += "\"Pressure\":"; payload += String(bmeP); payload += ",";
        payload += "\"Humidity\":"; payload += String(bmeH); payload += ",";
        payload += "\"Altitude\":"; payload += String(bmeA);
        payload += "}";
      }
      break;
    case 2: {
      }
      break;
    case 3: {
      }
      break;
  }

  if (WiFi.status() != WL_CONNECTED) {
    wifiReconnect();
  }

  char Topic[20]; // groupxx/nodexxx = 15 bit (max)
  topic.toCharArray(Topic, 20);
  char Payload[150];
  payload.toCharArray(Payload, 150);
  client.publish(Topic, Payload);
}

// *****************************************************
// data structure: msgLength|destination|groupID|localAddr|HH|MM
// *****************************************************
void sendBeacon() {
  byte message[] = {6, broadcastAddr, groupID, localAddr, byte(timeinfo.tm_hour), byte(timeinfo.tm_min)};
  sendMsg(message, 6);
}

// *****************************************************
void sendDownlink(byte nodeNum) { // by default no command
  byte commandMode = controlRecord[nodeNum].commandMode;
  Serial.println("Sending downlink to Node " + String(nodeNum) + " ,case " + String(commandMode) );
  byte dataLength = 5; // fixed
  switch (commandMode) {
  default: case 0: {
        byte message[] = {dataLength, nodeNum, groupID, localAddr, commandMode};
        sendMsg(message, dataLength);
        break;
      }
      break;
    case 1: case 2: case 3: {
        dataLength += 2;
        byte message[] = {dataLength, nodeNum, groupID, localAddr, commandMode, \
                          controlRecord[nodeNum].sw1, controlRecord[nodeNum].sw1_time
                         };
        sendMsg(message, dataLength);
        break;
      }
    case 4: case 5: case 6: {
        dataLength += 4;
        byte message[] = {dataLength, nodeNum, groupID, localAddr, commandMode, \
                          controlRecord[nodeNum].sw1, controlRecord[nodeNum].sw1_time, \
                          controlRecord[nodeNum].sw2, controlRecord[nodeNum].sw2_time
                         };
        sendMsg(message, dataLength);
        break;
      }
    case 7: {
        dataLength += 6;
        byte message[] = {dataLength, nodeNum, groupID, localAddr, commandMode, \
                          controlRecord[nodeNum].sw1, controlRecord[nodeNum].sw1_time, \
                          controlRecord[nodeNum].sw2, controlRecord[nodeNum].sw2_time, \
                          controlRecord[nodeNum].sw3, controlRecord[nodeNum].sw3_time
                         };
        sendMsg(message, dataLength);
        break;
      }
  }
}


// *****************************************************
void sendMsg(byte *message, byte msgLength) {
  LoRa.beginPacket();
  for (byte i = 0; i < msgLength; i++) {
    LoRa.write(message[i]);
  }
  LoRa.endPacket();
}

// *****************************************************
void wifiReconnect() {
  if ( WiFi.status() != WL_CONNECTED ) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }
}

#endif
