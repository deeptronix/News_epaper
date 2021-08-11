

void initComm(){
  channel.begin(BAUD, SERIAL_8N1, RXD2, TXD2);
  pinMode(ACK_IN, INPUT);
  pinMode(IRQ_to_Teensy, OUTPUT);
  pinMode(LED, OUTPUT);
}

void requestTeensy(){ // IMAGE DATA SPECIFIC! Will halt execution until Teensy responses
  uint32_t cnt = 0;
  digitalWrite(IRQ_to_Teensy, 1);   // Ask teensy to receive an image
  while(!digitalRead(ACK_IN)){      // wait for acknowledgement
    delay(1);
    cnt++;
#if DEBUG
    if(cnt == 3000){   // after this many ms, print a warning
      debugPrint("Teensy seems to be busy... ");
    }
#endif
    _assertCritical(cnt < TEENSY_COMM_TIMEOUT_ms, "requestTeensy: waiting for Teensy IMAGE DATA acknowledge in time (" + String(TEENSY_COMM_TIMEOUT_ms/1000) + " seconds).", false);    // after tot ms this will cause the condition to fail
  }
  digitalWrite(IRQ_to_Teensy, 0);   // Clear image-push request
}

void pushBuffer(uint8_t* data, uint32_t len){
  for(uint32_t p = 0; p < len; p++){
    channel.write(data[p]);
  }
}

// Brief:
// @param char*: the char array pointer to be sent, if message is set pending using the function "sendRequestStatus"
// @returns: true if the message has been sent, false if Teensy is not ready to listen (so message is still pending)
bool sendQueue(char* message){
  digitalWrite(IRQ_to_Teensy, 1);     // Ask teensy to receive a message
  if(digitalRead(ACK_IN)){            // wait for acknowledgement; FREERUNNING
    sendRequestStatus(CLEAR);      // Clear message presence
    digitalWrite(IRQ_to_Teensy, 0);   // Clear pin status
    channel.print(message);           // send message content
    debugPrintln("Message sent!");
    return true;
  }
  return false;
}


bool sendRequestStatus(bool READonly){
  if(READonly == READ){
    return sendRequestStatus(READ, 0);
  }

  debugPrintln("ACCESS ERROR!");
  return 0;
}

bool sendRequestStatus(bool read_WRITE, bool send_req_value){
  static bool send_request = false;
  if(read_WRITE == READ){
    return send_request;
  }
  else if(read_WRITE == WRITE){
    send_request = send_req_value;
  }
  return false;
}




void flashLED(){
  digitalWrite(LED, HIGH);
  delay(2);
  digitalWrite(LED, LOW);
}



inline void debugPrint(String text){
#if DEBUG
  Serial.print(text);
#endif
}
inline void debugPrintln(String text){
#if DEBUG
  Serial.println(text);
#endif
}






// eof
