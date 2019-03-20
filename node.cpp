#include "node.h"

unsigned long previousMillis = 0;
volatile byte sleep_counter = 0;

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

// divede value with decimal(float) into to integer
void float2byte(byte &integer, byte &decimal, float value){
    integer = value;
    decimal = byte((value-integer)*(value<100?100:1000));
}

// sync time
void syncTime(byte syncHH, byte syncMM, byte syncSS, long &currentSS){
  int delta = HH*3600 + MM*60 + SS - currentSS;
  currentSS += delta;
}

void loop(){
  if (millis.() - lastSendTime > interval){
    // do something
    lastSendTime = millis.();
  }
  onReceive(LoRa.parsePacket);
}


void loop(){
  if (sleep_counter >= 5){
    sleep_counter = 0;
    // do something
    // finish
    LoRa.sleep();
    Sleep_avr();
  }
  else{
    LoRa.sleep();
    Sleep_avr;
  }
}

//
void setPeroid(byte sendHH, byte peroid){
  // send uplink at [sendHH + (ID-1)*15 sec + n*peroid HH] // 4 uplink per minture
}



////////////////////////////////////////////////////////////////////
// section: watchdog
void setup_watchdog(int ii) {//0-16ms 1-32ms 2-64ms 3-128ms 4-250ms 5-500ms 6-1s 7-2s 8-4s 9-8s
  byte bb;

  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}
//WDT interrupt
ISR(WDT_vect) {
  ++sleep_counter;
  // wdt_reset();
}

void Sleep_avr() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);// sleep mode is set here
  sleep_enable();
  sleep_mode();// System sleeps here
}
