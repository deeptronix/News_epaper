
void deghost(uint8_t cycles){
  epd.ClearFrame();
  for(uint8_t k = 0; k < cycles; k++){
    epd.DisplayFrame();
    debugPrintln("Display deghosted (iter. " + String(k + 1) + " out of " + String(cycles) + ").");
    delay(500);
  }
}


void configureComm(){
  channel.begin(BAUD);
  pinMode(TRIGGER_OUT, OUTPUT);
  pinMode(IRQ_receive, INPUT);
}

bool pollESP(){
  return digitalRead(IRQ_receive);
}

void ackESP(){      // give the ESP32 an acknowledge
  digitalWrite(TRIGGER_OUT, 1);
  while(digitalRead(IRQ_receive)){ delay(1); }
  digitalWrite(TRIGGER_OUT, 0);
}

int32_t pullBuffer(uint8_t* buf, uint32_t len){
  int32_t i = 0;
  bool repeat = false;
  
  do{
    while(channel.available() > 0){
      buf[i++] = channel.read();
    }
    delay(5);   // 10 was too much.
    if(channel.available() > 0)  repeat = true;
    else  repeat = false;
  }while(repeat);

  channel.flush();

  return i;
}

String getFirstChars(uint8_t* data, uint8_t len){
  String msg = "";
  for(uint8_t p = 0; p < len; p++){
    msg += (char)data[p];
  }

  return msg;
}



inline void debugPrint(String text){
  #ifdef DEBUG
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

inline void debugPrintln(String text){
  #ifdef DEBUG
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

inline void debugPrintln(){
  #ifdef DEBUG
  debugPrintln("");
  #endif
}



// eof
