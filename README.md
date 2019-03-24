
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
## v1-20.03.2019
No private downlink message. Instructions embedded in Beacon message following global time
![lora tdma](https://user-images.githubusercontent.com/33332225/54752895-ea2b3080-4bdf-11e9-9933-cb70d1354f7d.png)
## v2-21.03.2019
Modified to send downlink message upon each uplink with instruction. Thus reserve a lora.read() window for Node after each SP

# Data structure
Uplink | Beacon | Downlink
![data structure](https://user-images.githubusercontent.com/33332225/54878046-04a31b00-4e27-11e9-8c0c-db702a6616dd.png)
## v2-21.02.2019
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
## node-v3 and gateway-v3, 23.03.2019
More stable compared with v2. Fixed the syncTime problem.
## node-v2 and gateway-v2, 20.03.2019
Duplex mode. Broadcast Beacon per 2 minutes. No private downlink from Gateway after each uplink. SP based on localAddress not implemented yet in code but designed in comment. Bascially both Gateway and Node works fine but Node sometimes send uplink message not on correct time.

## node-v1 and gateway-v1
Simplex mode, just sensor data from Node to Gateway. Connected to thingsboard.

# Test log
## Distance
*  15.03.2019, 435Mhz, SF-10, BD-125, txPower-17, (5dbi?) antenna at 3rd floor indoor <br>. Result: 400m -108 ~ -125
![2](https://user-images.githubusercontent.com/33332225/54755892-53fb0880-4be7-11e9-851b-d2843232f41c.PNG)
## Power consumption

# Task
- [x] Play with sensors and oled
- [x] Design project-specific data structure
- [x] Get real time from NTP server
- [x] Connect to IoT platform
- [x] Duplex mode for both Gateway and Nodes
- [x] Play with Node-RED with conclusion that it doesn't support multi-user/dashboard by itself
- [ ] Make UI for instruction input
- [ ] Intergrate watchdog timer & power_down with current structure on pro mini
- [ ] Test how setTime() in time library works or if checking setStatus() is needed
- [ ] Evaluate if necessary to compensate time dirft of pro mini considering the Beacon Period could be set to 1 or 2 minutes which is quite short
- [ ] Implement readServer() on Gateway and save instruction in stack
- [ ] Implement sendDownlink() on Gateway
- [ ] Implement readDownlink() on Node, and reserve callAcutator() function
- [ ] Implement data encryption
- [ ] Node as forwarder or mesh network
- [ ] Listen Before Talk if necessary
