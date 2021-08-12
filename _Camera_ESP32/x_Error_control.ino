
/*	@brief: tests a condition and logs an error if it's false. By default, tells Teensy via Serial
			the error codename.
	@param: the test to be checked
	@param: the string to be appended to the error message
*/
void _assert(bool test, String error_source){
  return _assert(test, error_source, true);
}


/*	@brief: tests a condition and logs an error if it's false. By default, tells Teensy via Serial
			the error codename.
	@param: the test to be checked
	@param: the string to be appended to the error message
	@param: whether or not to communicate to Teensy via serial the error codename.
*/
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


/*	@brief: tests a condition; if it's false, it logs an error and reboots the board (the error was critical).
	@param: the test to be checked
	@param: the string to be appended to the error message
*/
void _assertCritical(bool test, String error_source){
  _assertCritical(test, error_source, true);
}


/*	@brief: tests a condition; if it's false, it logs an error and reboots the board (the error was critical).
	@param: the test to be checked
	@param: the string to be appended to the error message
	@param: whether or not to communicate to Teensy via serial the error codename.
*/
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
