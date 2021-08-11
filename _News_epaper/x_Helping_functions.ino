
/*	@brief: performs EPD deghosting; as many cycles, as indicated by its argument. A small delay is
			introduced in order to allow the particles to completely settle to their state.
	@param: the number of cycles to perform (note: only in range 0 - 255!)
*/
void deghost(uint8_t cycles){
  epd.ClearFrame();		// clear the internal EPD image buffer
  for(uint8_t k = 0; k < cycles; k++){
    epd.DisplayFrame();		// updating the display deghosts it
    debugPrintln("Display deghosted (iter. " + String(k + 1) + " out of " + String(cycles) + ").");
    delay(300);
    epd.WaitUntilIdle();	// repeat only after the EPD is back online
  }
}


/*	@brief: configures the communication with the ESP32, which is an 8n1 RS232. Also, 2 additional
			pins are used to receive a request from the ESP32 (IRQ_receive), and give back an
			acknwoledge (TRIGGER_OUT).
*/
inline void configureComm(){
  channel.begin(BAUD);
  pinMode(TRIGGER_OUT, OUTPUT);
  pinMode(IRQ_receive, INPUT);
}


/*	@brief: reads the state of the IRQ_receive pin, to see if the ESP32 is communicating something.
	@returns: the state of the pin, active high.
*/
inline bool pollESP(){
  return digitalRead(IRQ_receive);
}


/*	@brief: after an incoming request has been asserted, give back the ESP an acknowledge by raising
			the TRIGGER_OUT pin, until the ESP clears the request.
			If this takes too much time, the ESP32 must be in a fault condition.
*/
void ackESP(){
  digitalWrite(TRIGGER_OUT, 1);
  uint16_t cnt = 0;
  while(digitalRead(IRQ_receive)){
    delay(1);
    cnt++;
    assert(cnt > 0, "ackESP");		// this condition will be violated after 65535ms, i.e. on an OVF
  }
  digitalWrite(TRIGGER_OUT, 0);
}


/*	@brief: after an ESP32 request for image sending, pull the data through the serial connection
			until no more data come through.
	@param: the pointer to the image buffer to be filled with the incoming data
	@returns: the number of bytes received (for a 300x400 image, we expect 15000 bytes since
			the image itself is already dithered and byte-aligned).
*/
int32_t pullBuffer(uint8_t* buf){
  int32_t len = 0;
  bool repeat = false;
  
  do{
    while(channel.available() > 0){
      buf[len++] = channel.read();
      assert(len > 0, "pullBuffer");
    }
    delay(10);   // if channel is emptied, wait a little to see if new data comes through
    if(channel.available() > 0){
      repeat = true;
    }
    else{
      repeat = false;
    }
  }while(repeat);

  channel.flush();

  return len;
}


/*	@brief: used to crop an incoming message, to see if it matches any of the ones known.
	@param: the pointer to the incoming data, to be cropped
	@param: the length of a standard message, fixed in z_Setup.h
	@returns: the message cropped to its first "len" characters
*/
String getFirstChars(uint8_t* data, uint8_t len){
  String msg = "";
  for(uint8_t p = 0; p < len; p++){
    msg += (char)data[p];
  }

  return msg;
}


/*	@brief: substitutes the std serial.print() in order to decide whether to print to serial or 
			logged to a .txt debug file on the SD card.
	@param: the text to be printed
*/
inline void debugPrint(String text){
  #if DEBUG
    #if LOGGING_TO_SD
    String debug_name = debug_filename;
    char db_arr[ARRAY_CHARS];
    debug_name.toCharArray(db_arr, debug_name.length() + 1);
    File file = SD.open(db_arr, FILE_WRITE);
  
    file.print(text);
    file.close();
  
    #else
    Serial.print(text);
    #endif
  #endif
}


/*	@brief: substitutes the std serial.println() in order to decide whether to print to serial or 
			logged to a .txt debug file on the SD card.
	@param: the text to be printed
*/
inline void debugPrintln(String text){
  #if DEBUG
    #if LOGGING_TO_SD
    String debug_name = debug_filename;
    char db_arr[ARRAY_CHARS];
    debug_name.toCharArray(db_arr, debug_name.length() + 1);
    File file = SD.open(db_arr, FILE_WRITE);
  
    file.println(text);
    file.close();
  
    #else
    Serial.println(text);
    #endif
  #endif
}


/*	@brief: substitutes the std serial.println() when just a CR char is needed (overloading function).
*/
inline void debugPrintln(){
  #if DEBUG
  debugPrintln("");
  #endif
}



// eof
