#ifndef NODE_H
#define NODE_H

#define SYNC_PEROID 60*60 // per hour
#define UPLINK_PEROID 3*60*60 // per 3 hour
volatile byte sleep_counter = 0;

class Node{
public:



private:
// whether store ID here


}

#endif

bool runEvery(int interval);
// extract sensor data to two parts
void float2byte(byte &integer, byte &decimal, float value);

overall process - task scheduler
setup:

  initialize lora;
  wait for boradcast time;
  sync time;
  record current millis, used later as an offset
  make lora and avr go sleep;
loop:
  around every clock [-10s, 10s]
    wake up avr and lora
    sync time
  at each peroid T (based on ID)
    wake up avr and lora
    read sensor data
    uplink data via lora
  additional interrupt
    by button, wake up avr, read sensor data, display on OLED, delay a while, sleep




/* data type and range
bool, 1
char, 1, -128~127
unsigned char, 1, 0-255
byte, 1, 0-255
int, 2, -32768~32767
unsigned int, 2, 0-65535
long, 4, -2147483648~2147483647
unsigned long, 4, 0~2^32-1
float, 4, -3.4028235E+38~3.4028235E+38
double, 4,
*/

/* uplink message structure
| GroupID | TargetID |SelfID | NumOfSensor | Sensor_1 | Sensor_2 | Sensor_3 |
|    06   |    0     |  99   |      3      |   65|12  |   12|45  |   33|78  |
*/

/* downlink message structure
| GroupID | TargetID | SelfID | HH | MM | SS | NumOfInstruction | TargetID | Instruction |
|    06   |    255   |    0   | 16 | 35 | 47 |         2        |    99    |      4      |
TargetID could be single node or broadcast
*/

/* range of data
GroupID: 0-255, assigned to different company/project
NodeID: 1-25x, maxium 25x node
reserved ID: 0-gateway, 254-forwarder, 255-broadcast
*/

/* forwarder logic
if group ID satisfied, check if TargetID==GW, then read whole message, forward without change
*/
