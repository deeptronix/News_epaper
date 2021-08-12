
/*	@brief: initializes the communication with the Teensy; sets up the 
			RS232 com port and sets pin port direction
*/
void initComm(){
  channel.begin(BAUD, SERIAL_8N1, RXD2, TXD2);
  pinMode(ACK_IN, INPUT);
  pinMode(IRQ_to_Teensy, OUTPUT);
  pinMode(LED, OUTPUT);
}


/*	@brief: once a photo has been taken, tries to interrupt Teensy and, upon ack, 
			unlocks the execution. IMAGE DATA SPECIFIC function! Will halt execution 
			until Teensy responses.
*/
void requestTeensy(){
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


/*	@brief: sends a buffer of data through the serial com port.
	@param: pointer to the data array
	@param: the array length
*/
void pushBuffer(uint8_t* data, uint32_t len){
  for(uint32_t p = 0; p < len; p++){
    channel.write(data[p]);
  }
}


/*  @brief: takes care of sending a message if it's pending; only after the message is sent, the request is cleared.
			Will not pause the execution, but instead just verify if Teensy has ack'd the request.
			In order to set a request, use the function "sendRequestStatus".
	@param: the char array pointer to be sent
    @returns: true if the message has been sent, false if Teensy is not ready to listen (so message is still pending)
*/
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


/*	@brief: sends or clears a request to Teensy by driving an IRQ pin. Overloading function for READ only access.
	@param: whether to read or write (write not allowed if value to be written is missing)
	@returns: true if a message is pending
*/
bool sendRequestStatus(bool READonly){
  if(READonly == READ){
    return sendRequestStatus(READ, 0);
  }

  _assert(ERROR_ASSERT, "ACCESS ERROR!");
  return false;
}


/*	@brief: sends or clears a request to Teensy by driving an IRQ pin. Main function for request read and write.
	@param: access type (use proper macros to make sense of it)
	@param: the value to be sent, if access is write
	@returns: true if a message is pending
*/
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



/*	@brief: toggles briefly the LED to tell a photo has been taken. Will be deprecated in final version
*/
void flashLED(){
  digitalWrite(LED, HIGH);
  delay(2);
  digitalWrite(LED, LOW);
}


/*	@brief: Overwrites Serial.print/ln in order to enable or disable them once debug output is turned off.
	@param: the text to be written on the serial port
*/
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
