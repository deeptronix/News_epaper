
void _assert(bool test, String error_source){
  return _assert(test, error_source, true);
}

void _assert(bool test, String error_source, bool tell_teensy){
  if(test == false){
    debugPrint("\n * * * ERROR FOUND AT ");
    debugPrintln(error_source);

    if(tell_teensy){    // if the problem IS the communication with Teensy, don't try to send it a message
      sendRequestStatus(PENDING);
      strcpy(message, STD_ERROR_MSG);
    }
  }
}

void _assertCritical(bool test, String error_source){
  _assertCritical(test, error_source, true);
}

void _assertCritical(bool test, String error_source, bool tell_teensy){
  if(test == false){
    debugPrint("\n * * * CRITICAL ERROR FOUND AT ");
    debugPrintln(error_source);
    debugPrintln("Forcing uC to restart.");

    if(tell_teensy){    // if the problem IS the communication with Teensy, don't try to send it a message
      sendRequestStatus(PENDING);
      strcpy(message, CRT_ERROR_MSG);
      while(!sendQueue(message)){
        delay(10);
      }
    }
    
    ESP.restart();
  }
}
