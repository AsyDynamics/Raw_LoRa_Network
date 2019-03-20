// NTP tutorial: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/


unsigned long previousMillis = 0; // last time update
long interval = 2000; // interval at which to do something (milliseconds)

void setup(){
}

void loop(){
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
     previousMillis = currentMillis;

     // do something
  }
}

//

// global variable defined header file: extern TYPE VAR_NAME
