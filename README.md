
# Raw LoRa Network
This is my attemp to implement a private network in **raw LoRa layer** based on arduino-LoRa library(https://github.com/sandeepmistry/arduino-LoRa). The hardware consists of Nodemcu as Gateway and Arduino pro mini as Node. Both Gateway and Node use Ra-02 (rfm98) as LoRa transceiver module. The system features:
* TDMA like class B to save energy
* Beacon message to sync time needed for TDMA
* Star network with forwarder

![3](https://user-images.githubusercontent.com/33332225/54754480-eb5e5c80-4be3-11e9-84cb-c3918142bf16.PNG)


# Overall process
## Node
* During bootup/setup, init lora, wait for Beacon message and set local time
* At each BP (Beacon Period), wake up, wait for Beacon and sync local time, then go sleep
* At each SP (Sense Period), wake up, read sensors and send uplink message, wait for downlink message for X sec, call actuator (if there is instruction from Gateway), then go sleep
## Gateway
* During bootup/setup, init lora, connect wifi, connect NTP server and sync local time
* At each BP, broadcast Beacon message, read from server and save instructions in stack, then go onReceive mode
* onReceive uplink from Node, decode data and send to server, send downlink message with if any instruction

![lora state machie (1)](https://user-images.githubusercontent.com/33332225/54755549-74769300-4be6-11e9-8199-ed42681b27d6.png)


# TDMA scheme
## v2-4.4.2019
Added some sketch to better illustrate and calculate the desired sleep time. Detail at https://github.com/AsyDynamics/Raw_LoRa_Network/blob/master/tips%20and%20notes.md#tdma <br>
## v2-21.3.2019
Modified to send downlink message upon each uplink with instruction. Thus reserve a lora.read() window for Node after each SP
## v1-20.3.2019
No private downlink message. Instructions embedded in Beacon message following global time
![lora tdma](https://user-images.githubusercontent.com/33332225/54752895-ea2b3080-4bdf-11e9-9933-cb70d1354f7d.png)


# Data structure
Uplink | Beacon | Downlink | MQTT-payload from server
![data (1)](https://user-images.githubusercontent.com/33332225/55631222-d672f300-57b7-11e9-8637-3f4aea1ae191.png) <br>
## v3.2-5.4.2019
* Include message length; remove useless msgID; Implement control command of max 3 switches in downlink
## v3.1-3.4.2019
* Consider removing Year, Month, Day, Second in beacon message with v4-program, since there is no clock running on nodes. They just need to know when is XX:YY:00 <br>
## v3-3.4.2019
* Consider sending message length as well to make sure the recved message is complete || haven't implemented
## v2-21.2.2019
* uplink and beacon remained
* downlink from gateway <br>
[ destination Addr | groupID | local Addr | msg count | actuator ID | instruction | if any more ]
## v1-20.03.2019
* uplink message <br>
[ destination Addr | groupID | local Addr | msg count | integer of pt100-1 | decimal of pt100-1 | other sensor values divied into integer and decimal ]
* beacon message <br>
[ destination Addr | groupID | local Addr | msg count | Hour | Minute | Second | Day | Month | Year ] <br>
Destination Addr: 0x00-Gateway, 0xFF-broadcast (reserved for beacon message)
Local Addr: 0x00-Gateway, 0x01-0xFE for Nodes


# Dashboard on Thingsboard
## v1-20.02.2019
Single node with two PT100 sensors and one BME280
![Capture](https://user-images.githubusercontent.com/33332225/54751624-d41b7100-4bdb-11e9-89bb-22feffd5c008.PNG)


# Development log
## gateway-v4, node-v4, 8.4.2019 <br>
* Use led blink to mimic downlink instruction, test passed
* Add flag to exit recv mode once downlink arrived but Node still in the recv slot, to save energy
## gateway-v4, node-v4, 5.4.2019
**Gateway**: <br>
* Implement sendDownlink inside LoRa onReceive function
* implement reading command from MQTT server and sendDownlink()
* Simplify the beacon message
* Simple encryption <br>

**Node**: <br> 
* Implement readDownlink
* Simple encryption
## node-v4-mimicMultiNode-withoutSensor-lowPower, 4.4.2019
* Upgrade the software structure with updated TDMA (https://github.com/AsyDynamics/Raw_LoRa_Network/blob/master/tips%20and%20notes.md#tdma) strategy
* enable power down with LowPower library <br>
## node-v4-downlink-basicSketch, 3.4.2019
* Not complete yet, added mode selection at bootup (setup) stage, with four modes in total, 00-debug, 01-lowTxPower, 10-mediumTxPower, 11-highTxPower respectively
* Make it clear how Sense Period offset is calculated based on SP and localAddr
* Define sensor structor to use it more conviently
* Implement the basic of receiving slot after each SP, the downlink recv and actuator operation <br>
## node-v3-mimic-multiNode and gateway-nodered-v2-mimic, 27.03.2019
Mimic multiple node and create node red dashboard <br>
![screencapture-192-168-178-38-1880-ui-2019-03-27-22_29_00](https://user-images.githubusercontent.com/33332225/55115008-bb332400-50e3-11e9-9f00-bb2a69edd5f9.jpg)
## node-v3 and gateway-v3, 25.03.2019
More stable compared with v2. Fixed the syncTime problem.
## node-v2 and gateway-v2, 20.03.2019
Duplex mode. Broadcast Beacon per 2 minutes. No private downlink from Gateway after each uplink. SP based on localAddress not implemented yet in code but designed in comment. Bascially both Gateway and Node works fine but Node sometimes send uplink message not on correct time.
## node-v1 and gateway-v1
Simplex mode, just sensor data from Node to Gateway. Connected to thingsboard.


# Test log
## Distance
*  2.04.2019, 435Mhz, SF-10, BD-125, txPower-17, (5dbi?) antenna at 3rd floor indoor, 3dbi antenna on receive node <br>
Results: 815m, RSSI:-122 ~ -125
![1](https://user-images.githubusercontent.com/33332225/55487685-deebf200-562e-11e9-8b9b-4da61f3b3e5b.PNG)

*  2.04.2019, 435Mhz, SF-10, BD-125, txPower-17, (5dbi?) antenna at 3rd floor indoor, 3dbi antenna on receive node <br>
Results: 440m, RSSI:-117 ~ -119
![2](https://user-images.githubusercontent.com/33332225/55487786-10fd5400-562f-11e9-9253-1c4da2966d2f.PNG)

*  15.03.2019, 435Mhz, SF-10, BD-125, txPower-17, (5dbi?) antenna at 3rd floor indoor <br>. Result: 400m, RSSI: -108 ~ -125
![2](https://user-images.githubusercontent.com/33332225/54755892-53fb0880-4be7-11e9-851b-d2843232f41c.PNG)
## Power consumption


# Task
- [x] Play with sensors and oled
- [x] Design project-specific data structure
- [x] Get real time from NTP server
- [x] Connect to IoT platform
- [x] Duplex mode for both Gateway and Nodes
- [x] Play with Node-RED with conclusion that it doesn't support multi-user/dashboard by itself
- [x] Listen Before Talk if necessary; need timeout to avoid lockup; not intergrate with current code
- [x] Make UI for instruction input; simpled button and switch on node red
- [x] Implement re-syncTime for fear of lost beacon and timedrift caused beacon-recv-window drift || not necessary, if beacon is lost or local time is incorrect, it will go into syncTime mode anyway, and until it get synced it will not jump out of the loop
- [x] Timeout of syncTime || not necesary, if the beacon message is not received on time, it means either the local time is incorrect or something wrong with the beacon server, thus need to figure it out
- [ ] Intergrate watchdog timer & power_down with current structure on pro mini
- [x] Test how setTime() in time library works or if checking setStatus() is needed || the +1 sec ahead sometimes is too fast, maybe insert some delay
- [x] Evaluate if necessary to compensate time dirft of pro mini considering the Beacon Period could be set to 1 or 2 minutes which is quite short || the time drift of pro mini 8Mhz is around 1s per 2 minutes, thus with a 1-min-beacon-period there is no worry
- [x] Implement readServer() on Gateway and save instruction in stack
- [x] Implement sendDownlink() on Gateway
- [x] Implement readDownlink() on Node, and reserve callAcutator() function
- [x] Implement data encryption || Implemented a simple XOR encryption, the key could be updated periodically by lora message if needed
- [ ] Node as forwarder or mesh network || With radiohead this feature comes out of box
- [x] Create Timeseries database using influxDB and sqlite; display historical chart on Grafana with influxDB and on Node-RED dashboard with sqlite; implement time range selection
- [ ] Play with Radiohead library
- [ ] Figure out CAD mode and interrupt to save energy, either using radiohead or arduino-lora-forked
- [ ] Test RFO and PA_BOOST setup's range and signal strength, assuming both set +14
- [ ] Press test/ packet loss test
- [ ] Add watchdog for nodemcu as it easily gets frozen
- [ ] Add led or oled screen on gateway/nodemcu to indicate current and historical status
