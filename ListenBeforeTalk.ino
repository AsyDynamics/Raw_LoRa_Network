// Listen Before Talk scheme

void LBT(){
  bool sendFlag = false;
  while (!sendFlag){
    if (LoRa.parsePacket() != 0){
      byte recipient = LoRa.read();
	  byte group  = LoRa.read();
	  if (group != groupID){
	    // After listen, now talk
	    sendMessage();
	    sendFlag = true;
	  }
	  else delay(100);
    }
    else { // no lora signal now
      sendMessage();
	  sendFlag = true;
    }
  }
}