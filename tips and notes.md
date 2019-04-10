# LoRa
* NTP tutorial: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/

* global variable defined in header file: extern TYPE VAR_NAME

* LoRa packet packet structure
preamble + header + payload
![Capture](https://user-images.githubusercontent.com/33332225/54922682-f3cad600-4f08-11e9-898f-91cd734f4e9b.PNG)

**The preamble** is used to synchronize receiver with the incoming data flow. By default the packet is configured with a 12 symbol long sequence. This is a programmable variable so the preamble length may be extended, for example in the interest of reducing to receiver duty cycle in receive intensive applications. However, the minimum length suffices for all communication. The transmitted preamble length may be changed by setting the register PreambleLength from 6 to 65535, yielding total preamble lengths of 6+4 to 65535+4 symbols, once the fixed overhead of the preamble data is considered. This permits the transmission of a near arbitrarily long preamble sequence.
 
The receiver undertakes a preamble detection process that periodically restarts. For this reason the preamble length should be configured identical to the transmitter preamble length. Where the preamble length is not known, or can vary, the maximum preamble length should be programmed on the receiver side. 

**Header** default mode is explict. In certain scenarios, where the payload, coding rate and CRC presence are fixed or known in advance, it may be advantageous to reduce transmission time by invoking implicit header mode. In this mode the header is removed from the packet. In this case the payload length, error coding rate and presence of the payload CRC must be manually configured on both sides of the radio link. 

**Power Amplifier** consists of two mode basically, the RFO and PA_BOOST, depends on the hardware/module design. The RFO is more efficient ranging from 0-14dBm, while the PA_BOOST has max +20dBm. The typical current of RFO mode at +13dBm is 29mA, and for PA_BOOST, it's 120mA when comes to +20dBm. By default, the arduino-lora library uses PA_BOOST mode and to switch to RFO, the second parameter in setTxPower() should be speicified.

**CAD** is a feature that be made use of to trigger mcu's external interrupt, thus putting it to sleep mode to save energy. One of the arduino-lora fork repo https://github.com/szotsaki/arduino-LoRa/blob/master/examples/LoRaCADDetectionWithInterrupt/LoRaCADDetectionWithInterrupt.ino has implmented it already. This feature comes out of box in Radiohead library.

# Node RED
## Database and Chart
* Using **DashDB** to store data in IBM cloud. https://internet-of-things.blog/en/how-to-backup-and-restore-chart-data-in-node-red-dashboard/
* Using **node-red-contrib-graphs** package with JSON format including UNIX timestamp. https://flows.nodered.org/node/node-red-contrib-graphs

## Homekit and Homekit-bridged
Bridged is better

## Others
* Using datepicker from dashboard and moment.js to convert to ISO8601 or UNIX timestamp, there is 48825(unix) drift, ahead of wanted time. That's to say, if pick 01-01-1970 it gives you 48825 rather than 0. Bear in mind with this when plot historical data that needs to compare timestammp. See following configure: <br>
![Capture](https://user-images.githubusercontent.com/33332225/55237804-44a53c00-5233-11e9-806e-5f284d71594c.PNG)

# JSON

# openHAB

# mimic NOIP
* remote.it or weaved https://remote.it/
* ngrok https://ngrok.com/ ```ngrok http portNum```
* dataplicity https://dataplicity.com 
* nginx https://www.nginx.com/ to host a webserver

# Software state flow
## TDMA
Using the Low Power library, arduino loses the function of keep date/time running. Thus the software could be modified as to sleep-to-next-BP-or-SP rather keep arduino running and checking if the BP or SP is triggered. To do so, seveal variable could be defined:
~~~
#define BP            1  // minute
#define preBP         2  // second, time to wakeup ahead of BP
#define localAddr     12 // from 1 to 254, 0 reserved for gateway, 255 reserved for broadcast
#define preSP         1  // second, time to wakep ahead of SP
#define SP_H          1  // sense period in hour
#define SP_M          15 // sense period in minute
#define SP_offset_m   ceil(localAddr / 10.0)-1   // minus one minute, as the current minute is 0
#define SP_offset_s   ((localAddr-1) % 10 + 1)*5 // fist minus one, otherwise the node with addr-10 is not easy to be allocated
#define recvTimeout   3  // second, time reserved for downlink slot
~~~
The **sleep-to-next-BP-or-SP** could be divided into two scenarios: <br>
For none Sense Period scenario, the sleep time after receving beacon is fixed, as the blue part indicated in the chart.
~~~
sleep(60-preBP);
~~~
![time-nonSP](https://user-images.githubusercontent.com/33332225/55567034-3193df80-56fd-11e9-8208-57fa47af6528.png)

For Sense Period scenario, the sleep time could be dynamic due to possible callback command from server. Thus the sleep time in that minute actually consists of two parts, the one before SP and the one after SP. For the previous one, just
~~~
sleep(SP_offset_s - preSP);
~~~
And for the latter one, it should be calculated 
~~~
sleep( 60-preBP - callback_finished_time_corresponding_to_0_second);
~~~
![time-SP](https://user-images.githubusercontent.com/33332225/55567030-2ccf2b80-56fd-11e9-8fa5-f7face27a53f.png)
The above mentioned logic could be represented by
~~~
HH = LoRa.read();
MM = LoRa.read();

if ( HH % SP_H == 0 && (MM-SP_offset_m) % SP_M == 0){  // at that hour, and that(SP) minute
  sleep(SP_offset_s - preSP);                          // sleep to SP, then wake up and do some stuff
  
  long preSPstamp = millis();
  readSensor();
  while(millis()-preSPstamp < preSP*1000){
    // do nothing
  }
  sendMessage();
  bool downlinkFlag = false;
  while(millis()-preSPstamp< (preSP + downlinkTimeout)*1000 && !downlinkFlag){
    readDownlink();
    callback(); // do some stuff, maybe run a actuator
    downlinkFlag = true;
  }
  // then go back to sleep again, may need to check if this is negative
  sleep( 60-preBP - (millis()-preSPstamp)/1000 - (SP_offset_s-preSP)); 
}
else {
  sleep(BP*60 - preBP);  // sleep to next BP
}
~~~
