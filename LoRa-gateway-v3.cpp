#include "ThingsBoard.h"#include "WiFi.h"#include < Wire.h > #include "data_convert.h"#include < SPI.h > // include libraries
  #include < LoRa.h > #include "time.h"

define WIFI_AP "ZiggoF5A5142"#
define WIFI_PASSWORD "Yhvsh6veykxy"#
define TOKEN "uyfyu7vxtdx8BuNlm3Je"#
define THINGSBOARD_SERVER "demo.thingsboard.io"

const int csPin = 5; // LoRa radio chip select
const int resetPin = 14; // LoRa radio reset
const int irqPin = 2; // change for your board; must be a hardware interrupt pin
// for NTP time server
const char * ntpServer = "europe.pool.ntp.org"; // no need to define global
const long gmtOffset_sec = 3600; // no need to define global
const int daylightOffset_sec = 3600; // no need to define global
// for lora communication
byte msgCount = 0; // count of outgoing messages
byte localAddress = 0x00; // address of this device
byte destination = 0xFF; // destination to send to
byte groupID = 0x00;
byte BeaconPeriod = 2; // 2 min, BP, interval between sends
int counter = 0;
struct tm timeinfo;

WiFiClient espClient;
ThingsBoard tb(espClient);
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(115200);
  Serial.println("Begin setup");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
  initializeLora();

  pinMode(32, OUTPUT);
  pinMode(33, OUTPUT);
  digitalWrite(32, LOW);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //WiFi.disconnect(true); WiFi.mode(WIFI_OFF);
  getNTPtime();
}

void loop() {
  while (!getLocalTime( & timeinfo)) {
    delay(50);
  }
  if ((timeinfo.tm_min % BeaconPeriod == 0) && timeinfo.tm_sec == 0) {
    sendMessage();
    delay(900);
    // recv lora message and sync time
  }

  //*************** BeaconPeriod *************************

  onReceive(LoRa.parsePacket());
}

//////////////////////////////////////////////////////
void onReceive(int packetSize) {
  if (packetSize == 0) return; // if there's no packet, return
  digitalWrite(33, HIGH);
  // read packet header bytes:
  byte recipient = LoRa.read(); // recipient address, 0x00 reserved for broadcast
  byte group = LoRa.read();
  if ((recipient != localAddress && recipient != 0xFF) && (group = groupID)) {
    Serial.println("This message is not mine.");
    return; // skip rest of function
  }
  Serial.println("Lora received");
  byte sender = LoRa.read(); // sender address
  byte incomingMsgId = LoRa.read(); // incoming msg ID
  byte bmeTint = LoRa.read();
  byte bmeTdec = LoRa.read();
  byte bmeHint = LoRa.read();
  byte bmeHdec = LoRa.read();
  byte bmePint = LoRa.read();
  byte bmePdec = LoRa.read();
  byte bmeAint = LoRa.read();
  byte bmeAdec = LoRa.read();
  byte max1int = LoRa.read();
  byte max1dec = LoRa.read();
  byte max2int = LoRa.read();
  byte max2dec = LoRa.read();

  int loraRSSI = LoRa.packetRssi();
  int loraSNR = LoRa.packetSnr();

  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data to thingsboard server");
  /////////////////////////////////
  float bmeT = byte2float(bmeTint, bmeTdec);
  float bmeH = byte2float(bmeHint, bmeHdec);
  float bmeP = byte2float(bmePint, bmePdec);
  float bmeA = byte2float(bmeAint, bmeAdec);
  float maxOne = byte2float(max1int, max1dec);
  float maxTwo = byte2float(max2int, max2dec);

  tb.sendTelemetryInt("Node", sender);
  tb.sendTelemetryInt("MessageID", incomingMsgId);
  tb.sendTelemetryInt("RSSI", loraRSSI);
  tb.sendTelemetryInt("SNR", loraSNR);
  tb.sendTelemetryFloat("Temperature", bmeT);
  tb.sendTelemetryFloat("Humid", bmeH);
  tb.sendTelemetryFloat("Pressure", bmeP);
  tb.sendTelemetryFloat("Altitude", bmeA);
  tb.sendTelemetryFloat("PT100-A", maxOne);
  tb.sendTelemetryFloat("PT100-B", maxTwo);
  tb.sendTelemetryInt("LoRa packet size:", packetSize);

  Serial.print("From:" + String(sender, HEX));
  Serial.print(" To:" + String(recipient, HEX));
  Serial.println(" " + String(incomingMsgId));
  Serial.println("Lora packet size: " + String(packetSize));
  Serial.println("BME Temp:  " + String(bmeT));
  Serial.println("BME Humid: " + String(bmeH));
  Serial.println("BME Press: " + String(bmeP));
  Serial.println("BME Allti: " + String(bmeA));
  Serial.println("Max1 Temp: " + String(maxOne));
  Serial.println("Max1 Temp: " + String(maxTwo));
  Serial.print("RSSI:" + String(loraRSSI));
  Serial.println(" SNR:" + String(loraSNR));
  Serial.println();

  digitalWrite(33, LOW);
}

//////////////////////////////////////////////////////
void sendMessage() {
  Serial.println("Enter sendMessage function...");
  LoRa.beginPacket();
  LoRa.write(0xFF); // destination, 0xFF reserved for broadcast
  LoRa.write(groupID); // common ID
  LoRa.write(0x00); // local address, 0x00 reserved for gateway
  LoRa.write(msgCount++);
  LoRa.write(byte(timeinfo.tm_hour));
  LoRa.write(byte(timeinfo.tm_min));
  LoRa.write(byte(timeinfo.tm_sec));
  LoRa.write(byte(timeinfo.tm_mday));
  LoRa.write(byte(timeinfo.tm_mon));
  LoRa.write(byte(timeinfo.tm_year - 100));
  LoRa.endPacket();
  Serial.println("Sent date/time signal " + String(timeinfo.tm_year - 100) +
    "/" + String(timeinfo.tm_mon) + "/" + String(timeinfo.tm_mday) +
    ", " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) +
    ":" + String(timeinfo.tm_sec));
  Serial.println("Exit sendMessage function");
}

//////////////////////////////////////////////////////
void getNTPtime() {
  Serial.println("Geting NTP time...");
  while (！getLocalTime( & timeinfo)) {
    delay(100); // wait and try again to connect NTP server
  }
  Serial.println( & timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println("Exit getNTPtime function");
}

//////////////////////////////////////////////////////
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

//////////////////////////////////////////////////////
void reconnect() {
  status = WiFi.status();
  if (status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}

//////////////////////////////////////////////////////
void initializeLora() {
  LoRa.setPins(csPin, resetPin, irqPin); // set CS, reset, IRQ pin
  if (!LoRa.begin(435E6)) {
    while (true); // if failed, do nothing
  }
  LoRa.setSyncWord(0xFF);
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  Serial.println("LoRa init succeeded.");
}