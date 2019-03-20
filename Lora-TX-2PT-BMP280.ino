#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_MAX31865.h>

// cs of lora: pin 10(default in h file)
Adafruit_MAX31865 max1 = Adafruit_MAX31865(7);
Adafruit_MAX31865 max2 = Adafruit_MAX31865(8);
#define RREF      430.0
#define RNOMINAL  100.0
#include <Bme280BoschWrapper.h>

Bme280BoschWrapper bme280(true);
#define seaLevel 1013.25

int message_counter = 0;
volatile byte sleep_counter = 0;


void setup() {
  max1.begin(MAX31865_2WIRE);
  max2.begin(MAX31865_2WIRE);
  if (!LoRa.begin(435E6)) {
    while (1);
  }
  LoRa.setSpreadingFactor(10);
  LoRa.setSyncWord(0xFF);
  //7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, and 250E3(default)
  LoRa.setSignalBandwidth(125E3);
  setup_watchdog(7);//0-16ms 1-32ms 2-64ms 3-128ms 4-250ms 5-500ms 6-1s 7-2s 8-4s 9-8s
  ACSR |= _BV(ACD);//OFF ACD
  ADCSRA = 0;//OFF ADC
  Sleep_avr();//Sleep_Mode
  LoRa.sleep();
  while(!bme280.beginSPI(A0))
  {
    delay(1000);
  }
}

void loop() {
  if (sleep_counter >= 5) {
    sleep_counter = 0;
    //-------------------------------
    float bmeT, bmeH, bmeP, bmeA;
    delay(300);
    bool bmeState = bme280.measure();
    if (bmeState){
      bmeT = bme280.getTemperature()/100.0;
      bmeH = bme280.getHumidity() / 1024.0;
      bmeP = bme280.getPressure()/1000.0;
      bmeA = 44330.0 * (1.0 - pow((bme280.getPressure()/100.0F) / seaLevel, 0.1903));
    }
    LoRa.beginPacket();
    LoRa.println(message_counter);
    LoRa.print("T: ");
    LoRa.print(max1.temperature(RNOMINAL, RREF));
    LoRa.print(","); 
    LoRa.println(max2.temperature(RNOMINAL, RREF));
    if (bmeState){
      LoRa.print("T&H: ");
      LoRa.print(bmeT);
      LoRa.print(" ");
      LoRa.print(bmeH);
      LoRa.println("%");
      LoRa.print(bmeP);
      LoRa.print("kpa ");
      LoRa.print(bmeA);
      LoRa.println("m");
    }
    LoRa.endPacket();
    message_counter++;

    
    //--------------------------------   
    LoRa.sleep();
    Sleep_avr();
  }
  else {
    LoRa.sleep();
    Sleep_avr();
  }

}

//Sleep mode is activated
void setup_watchdog(int ii) {
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
