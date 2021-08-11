
/*
 // Requires definition of object: SnoozeBlock config_teensy35(alarm);
 // and of object: SnoozeAlarm  alarm;
inline void setSleepTimer_sec(int32_t tot_sec){
  if(tot_sec < 0){
    assert(ERROR_ASSERT, "tried loading a negative value in RTC timer.");
    return;
  }
  uint8_t hours = tot_sec / 3600;
  uint8_t mins = (tot_sec / 60) % 60;
  uint8_t secs = tot_sec % 60;
  alarm.setRtcTimer(hours, mins, secs);   // hour, min, sec
}
*/

inline void initSleep(){
  setSleepTimer_ms(1000);    // set sleep timer to 1 sec
}

inline void setSleepTimer_ms(int32_t tot_ms){
  if(tot_ms < 0){
    assert(ERROR_ASSERT, "tried loading a negative value in Low Power timer.");
    return;
  }
  
  #if defined(__IMXRT1062__)    // Teensy 4.0
  timer.setTimer(tot_ms / 1000);// seconds
  #else
    timer.setTimer(tot_ms);// milliseconds
  #endif
}

inline void sleepTeensy(){
  Snooze.sleep(config_teensy35);
}



// eof
