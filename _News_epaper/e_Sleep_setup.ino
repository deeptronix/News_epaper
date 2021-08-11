
/*	@brief: Sets the sleep timer to 1000 milliseconds, fixed in SW because other timing hypothesis are based on this behaviour
*/
inline void initSleep(){
  setSleepTimer_ms(1000);    // set sleep timer to 1 sec
}


/*	@brief: loads the timer value in the sleep timer function
	@param: the amount of ms to sleep for, once the sleep function is called.
*/
inline void setSleepTimer_ms(int32_t tot_ms){
  if(tot_ms < 0){
    assert(ERROR_ASSERT, "tried loading a negative value in Low Power timer.");
  }
  else{
	  // depending on the underlying architecture, sets the correct timer value
	#if defined(__IMXRT1062__)    	// Teensy 4.0
	timer.setTimer(tot_ms / 1000);	// seconds
	#else
	timer.setTimer(tot_ms);		// milliseconds
	#endif
  }
}


/*	@brief: Puts the processor to sleep (low power mode); the time is set by loading the
			timer with the appropriate duration beforehand (using the previous functions).
*/
inline void sleepTeensy(){
  Snooze.sleep(config_teensy35);
}



// eof
