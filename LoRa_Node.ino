#include <lora_node.h>

// declare modules that communicate via wire, e.g.max31865, bme280, lora

// define lora-node-based variable, e.g.ID
// define sync_peroid


void setup(){
  // initialize sensor modules that communicate
  // initialize lora module, frequency, spreading factor, band width, spreading factor
  // wait lora.recv() to get broadcast time/bell

  // set avr and lora to sleep mode
}

void loop(){
  if (runEvery(SYNC_PEROID)){
    // lora.recv() to get broadcast time/bell
    // should wait a bit here, depends on the time drift, maybe 20s
  }

  if (runEvery(UPLINK_PEROID)) {
    // read sensor data
    // construct data structure
    // uplink lora message
  }
}
