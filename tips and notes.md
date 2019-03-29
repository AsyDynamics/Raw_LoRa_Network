# LoRa
* NTP tutorial: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/

* global variable defined in header file: extern TYPE VAR_NAME

* LoRa packet packet structure
preamble + header + payload
![Capture](https://user-images.githubusercontent.com/33332225/54922682-f3cad600-4f08-11e9-898f-91cd734f4e9b.PNG)

**The preamble** is used to synchronize receiver with the incoming data flow. By default the packet is configured with a 12 symbol long sequence. This is a programmable variable so the preamble length may be extended, for example in the interest of reducing to receiver duty cycle in receive intensive applications. However, the minimum length suffices for all communication. The transmitted preamble length may be changed by setting the register PreambleLength from 6 to 65535, yielding total preamble lengths of 6+4 to 65535+4 symbols, once the fixed overhead of the preamble data is considered. This permits the transmission of a near arbitrarily long preamble sequence.
 
The receiver undertakes a preamble detection process that periodically restarts. For this reason the preamble length should be configured identical to the transmitter preamble length. Where the preamble length is not known, or can vary, the maximum preamble length should be programmed on the receiver side. 

**Header** default mode is explict. In certain scenarios, where the payload, coding rate and CRC presence are fixed or known in advance, it may be advantageous to reduce transmission time by invoking implicit header mode. In this mode the header is removed from the packet. In this case the payload length, error coding rate and presence of the payload CRC must be manually configured on both sides of the radio link. 

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
