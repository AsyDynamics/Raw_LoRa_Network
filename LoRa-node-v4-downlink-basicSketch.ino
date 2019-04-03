#define mode_pin1 7
#define mode_pin2 8

// Node info
const byte localAddr  = 1;
const byte targetAddr = 0;
const byte groupID    = 0;

const byte SP_m = 1; // per minute
const byet SP_h = 1; // per hour
const byte SP_offset_m = ceil(localAddr/10)-1;   // id 1-10 -> 0 minute, 11-20 -> 1 minute
const byte SP_offset_s = ((localAddr-1)%10+1)*5; // id 1-10 corresponds to 5,10,15,20,...,45,50
const byte senseMode = 1; // A-two pt100, B-enviroment temp, C-humid, D-water-leakage. THUS 1-AB,2-AC,3-AD,4-ABC,5-ABD,6-ACD,7-ABCD
const byte initLoraTimeout = 5; 
const byte debugTimeout = 60*5;  // time for debug, could be forever though. to disable debug mode, change short onboard  jumper 
const byte beaconTimeout = 60*5; // Let's keep it 5 minutes as max beaconTimeout
const byte downlinkTimeout = 3;  // let's say, 3 sec for receive downlink

sensor S-temp1, S-temp2, S-tempEnv, S-humid, S-leakage;

void setup(){
	initSensor();
	initLora();
	setOperationMode();
	syncTime();
	sleep(to-next-BP); // actually, it should be sleed to next BP-minus in and keep awake during [minus, plus] for fear of missing beacon
}

void loop(){
	// ************************ Beacon Period ************************
	if (second()>=59 || second()<1){ // Just use this to distinguis the current mode based on current time // 
		syncTime();
		sleep(to-next-BP-or-SP); // should calculate nearest wake-up-time
	}
	
	// ************************ Sense Period ************************
	if ( hour()%SP_h == 0 && minute()%(SP_m + SP_offset_m) == 0 && second >= SP_offset_s-1){ // wake up 1 sec eariler to read sensor
		readSensor(2);
		while (second()<SP_offset_s){
			delay(10)
		}
		sendMeesage(2); // mode 0 reserved for debug mode
		long sentTime = millis();
		
		// for recv donwlink message
		bool recvFlag = false;
		
		while(!recvFlag && millis() - sentTime < downlinkTimeout){ // Just use this to distinguis the current mode based on current time
			// receive downlink message // receive downlink lora message, parse instruction and do something;
			if (receiveDownlink()){
				runActuator();
			recvFlag = true;
			}
		}
		long wakedTime = millis() - recvTime;
		sleep(to-next-BP-or-SP-minus-wakedTime); // should calculate nearest wake-up-time
	}
}

void setOperationMode(){
	bool sw1 = digitalRead(sw1_pin);
	bool sw2 = digitalRead(sw2_pin);
	delay(50);
	if (digitalRead(sw1)==sw1 && digitalRead(sw2)==sw2){
		if (sw1 == false && sw2 == false){
			debugLora();}
		if (sw1 == true && sw2 == true){ // high power
			LoRa.setTxPower(17);}
		if (sw1 == false && sw2 == true){ // low power
			LoRa.setTxPower(10);}
		else{
			LoRa.setTxPower(15);} // medium power
	} else LoRa.setTxPower(15); // by default
}


void initSensor(){ // including debug pin
	pinMode(mode_pin1,INPUT);
	pinMode(mode_pin2, INPUT);
	S-temp1.setPin();
	S-temp2.setPin();
	S-tempEnv.setPin();
	S-humid.setPin();
}

void initLora(){
	LoRa.setPins(); // not necessary if using default pinout with arduino pro mini
	long initMillis = millis();
	while(!LoRa.begin(433E6) && millis() - initMillis < initLoraTimeout*1000){
		delay(100);
	}
	if (!LoRa.begin(433E6)){
		// indicate the init error;
	} else{
		LoRa.setSyncword(0xF1);
		LoRa.setSpreadingFactor(10);
		LoRa.setSignalBandwidth(125E3);
	}
}


void syncTime() {
  bool syncTimeFlag = false;
  while (!syncTimeFlag) {
    if (LoRa.parsePacket() != 0) {
      byte recipient = LoRa.read();         // recipient address
      byte group = LoRa.read();             // common group ID
      if ((recipient == localAddress || recipient == 0xFF) && (group == groupID)) { // 0xFF reserved for broadcast
        byte sender = LoRa.read();
        byte msgLength = LoRa.read();
        sync_Hr =  LoRa.read();
        sync_Min = LoRa.read();
        sync_Sec = LoRa.read();
        sync_Day = LoRa.read();
        sync_Mon = LoRa.read();
        sync_Yer = LoRa.read();
        setTime(sync_Hr, sync_Min, sync_Sec + 1, sync_Day, sync_Mon, sync_Yer); // set time 1 sec ahead because of delay
        syncTimeFlag = true;
      }
    }
  }
}

void readSensor(byte mode){
	S-temp1.readValue();
	S-temp2.readValue();
	switch(mode){
		case 1:
			S-humid.readValue();
			break;
		case 2:
			S-tempEnv.readValue();
			break;
		case 3:
			S-leakage.readValue();
			break;
	}
}

void debugLora(){
	byte debugMillis = millis;
	while (millis() - debugMillis < debugTimeout*1000){
		// do some sending and recving stuff
		sendMeesage(0);  // mode 0 reserved for debug
		onReceive();
	}
}

void sendMessage(byte mode){
	float2byte(); // convert temp1, temp2, tempEnv, humid, leakage
	LoRa.beginPacket();
	LoRa.write(groupID);
	LoRa.write(targetAddr);
	LoRa.write(localAddr);
	LoRa.write(mode); // 0 reserved for debug mode, should begin from 1
	switch(mode){
		case 0:
			LoRa.write(msgLength); // used to verify if message is complete
			LoRa.write(); // debug flag or something else
			break;
		case 1:
			LoRa.write(msgLength);
			LoRa.write(S-temp1.integer);
			LoRa.write(S-temp1.decimal);
			LoRa.write(S-temp2.integer);
			LoRa.write(S-temp2.decimal);
			LoRa.write(S-humid.integer);
			LoRa.write(S-humid.decimal);
			break;
		case 2:
			LoRa.write(msgLength);
			LoRa.write(S-temp1.integer);
			LoRa.write(S-temp1.decimal);
			LoRa.write(S-temp2.integer);
			LoRa.write(S-temp2.decimal);
			LoRa.write(S-tempEnv.integer);
			LoRa.write(S-tempEnv.decimal);
			LORa.write(); // other value
			break;
		case 3:
			LoRa.write(msgLength);
			LoRa.write(S-temp1.integer);
			LoRa.write(S-temp1.decimal);
			LoRa.write(S-temp2.integer);
			LoRa.write(S-temp2.decimal);
			LoRa.write(S-leakage.integer);
			LoRa.write(S-leakage.decimal);
	}
	LoRa.endPacket();
}

bool receiveDownlink(){
	if (LoRa.parsePacket() != 0) {
		byte recipient = LoRa.read();         // recipient address
    byte group = LoRa.read();             // common group ID
    if ((recipient == localAddress || recipient == 0xFF) && (group == groupID)) { // 0xFF reserved for broadcast
      byte sender = LoRa.read();
      byte msgLength = LoRa.read();
      sync_Hr =  LoRa.read();
      sync_Min = LoRa.read();
      sync_Sec = LoRa.read();
      sync_Day = LoRa.read();
      sync_Mon = LoRa.read();
      sync_Yer = LoRa.read();
      setTime(sync_Hr, sync_Min, sync_Sec + 1, sync_Day, sync_Mon, sync_Yer); // set time 1 sec ahead because of delay
      return true;
    }
  } else return false;
}


void ledBlink(byte state){ // may have several state, syncTime, sending, low battery or so on
	switch(state){
		case 0:
			// error
			break;
		case 1:
			//
			break;
		case 2:
			//
			break;
	}
}

void runActuator(byte command1, byte time){
	// dummy demo, not correct
	long actuatorTime = millis();
	digitalWrite(actuator_pin, HIGH);
	if (millis()-actuatorTime > time*1000*60){
		digitalWrite(actuator_pin,LOW);
	}
}


struct sensor{
	// later implement constructor and destructor
public:
	float value = 255.255;
	byte integer = 255;
	byte decimal = 255;
	void setPin(byte pin){
		pinMode(pin, INPUT);
	}
	void readValue(){
		// dummy example
		value = 12.34; // value = analogRead(pin) and then do some process
		integer = value / 10;
		decimal = (value - integer)*100;
	}
}
	