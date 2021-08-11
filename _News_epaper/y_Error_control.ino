
#define RESTART_ADDR 0xE000ED0C
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))


/*	@brief: writes the registers that cause the Teensy to restart
*/
inline void TeensyRestart(){
  WRITE_RESTART(0x5FA0004);
}


/*	@brief: tests a condition and logs an error if it's false.
	@param: the test to be checked
	@param: the string to be appended to the error message
*/
void assert(bool test, String error_source){
  if(test == false){
    debugPrint("\n * * * ERROR FOUND AT ");
    debugPrintln(error_source);
  }
}


/*	@brief: tests a condition; if it's false, it logs an error and reboots the board (the error was critical).
	@param: the test to be checked
	@param: the string to be appended to the error message
*/
void assertCritical(bool test, String error_source){
  if(test == false){
    debugPrint("\n * * * CRITICAL ERROR FOUND AT ");
    debugPrintln(error_source);
    debugPrintln("Forcing uC to restart.");
    delay(10);
    TeensyRestart();
  }
}


// eof
