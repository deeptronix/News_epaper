
#define RESTART_ADDR 0xE000ED0C
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))


inline void TeensyRestart(){
  WRITE_RESTART(0x5FA0004);
}

void assert(bool test, String error_source){
  if(test == false){
    debugPrint("\n * * * ERROR FOUND AT ");
    debugPrintln(error_source);
  }
}

void assertCritical(bool test, String error_source){
  if(test == false){
    debugPrint("\n * * * CRITICAL ERROR FOUND AT ");
    debugPrintln(error_source);
    debugPrintln("Forcing uC to restart.");
    delay(10);
    TeensyRestart();
  }
}
