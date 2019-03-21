# Notes of thingsboard
## Arduino library
link: https://github.com/thingsboard/ThingsBoard-Arduino-MQTT-SDK <br>
This library implment sendTelemetry, sendAttribute and server-side RPC features.
### sendTelemetry & sendAttributes
`tb.sendAttributes(attributes, attribute_items)` where `attributes` could be defined as <br>
`Attribute attributes[attribute_items] = {` <br>
`    { "device_type",  "sensor" },` <br>
`    { "active",       true     },` <br>
`  };` <br>
This is identical to sendTelemetry

### server-side RPC
* The RPC message is in json form and the key word could be extracted by using []. Upon RPC, call predefined function. <br>
Possbile procedure: check keyword "nodeID" and "Instruction". This applies to single RPC/nodeID only, or multiple? If multiple, should build an array to save the instruction, and after each SP, check the flag in array, if(flag), send instruction within downlink.
