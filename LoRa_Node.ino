#include "lora_node.h"

// declare modules that communicate via wire, e.g.max31865, bme280, lora

// define lora-node-based variable, e.g.ID
// define sync_peroid


void setup(){
  // initialize sensor modules that communicate

////////////////////////////////////////////////////
  // initialize lora module, frequency, spreading factor, band width, spreading factor

  if (!LoRa.begin(435E6)) {             // initialize ratio at 915 MHz
    while (true);                       // if failed, do nothing
    // do something
  }
  LoRa.setSpreadingFactor(10);
  LoRa.setBandWidth(125E3);
  LoRa.setSyncWord(0x12);
////////////////////////////////////////////////////
  // wait lora.recv() to get broadcast time/bell


////////////////////////////////////////////////////
  // set avr and lora to sleep mode
}

void loop(){
  if (T = BeaconPeriod){ // 00:00:00, 00:02:00, every 120 sec
    // wait LoRa.available() and run LoRa.read() to get broadcast time/bell
    sync_time = LoRa.read();
    setTime(syncTime);
    LoRa.sleep(); Sleep_avr();
  }

  if (T = SensePeriod) { // 00:00:node_ID*5
    // read sensor data
    // encode data structure
    LoRa.write(); // uplink lora message
    LoRa.sleep(); Sleep_avr();
  }

}
