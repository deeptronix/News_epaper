
#define _max(val_a, val_b)  ((val_a > val_b)?  val_a : val_b)
#define _min(val_a, val_b)  ((val_a < val_b)?  val_a : val_b)

#define debug_filename "debug.txt"

#if FINALIZE_DELIVERY   // The parameters used on THE FINAL PRODUCT
  // EPD paramters:
  const uint16_t ep_width = 400, ep_height = 300;
  #define EP_SIZE (ep_width*ep_height)
  const uint8_t STARTUP_CLEAR_CY = 2; // needed to clear the previous image, which might have forced pixels
  #define NIGHT_DEGHOST_CYCLES 1      // cycles per iteration during the night
  #define MIN_DEGHOST_OFFSET 2
  #define ABS_MAX_DEGH_CYCLES 5
  #define AFTER_PHOTO_DEGHOST_CYCLES 1
  #define DEGHOST_FORMULA  _min(((_max(repeat, MIN_DEGHOST_OFFSET) * total_frames) / 12), ABS_MAX_DEGH_CYCLES)
  
  // Communication parameters & pin definitions
  #define BAUD 57600   // Pins: TX=1,  RX=0
  #define TRIGGER_OUT 29
  #define IRQ_receive 30
  #define channel Serial1
  #define MSG_LENGTH 5
  #define MOTION_MSG "MOVMT"
  #define LOWLIGHT_MSG "NIGHT"
  #define GOODLIGHT_MSG "GDDAY"
  
  
  // Execution flow parameters:
  const String playlist = "playlist.txt";    // .txt file that lists every image, animation and pause to be executed sequentially (like a playlist)
  const String RECONVERT = "convert.txt";    // if this file is found on the SD card, it will perform the dithering+byte alignment and save each header file of all animations; it will then delete this file
  #define ANIMAT_char '$'
  #define WAIT_char ':'
  #define STOP_char '.'
  #define SLOWDOWN_PLAYBACK_ms 0
  #define DEFAULT_SLEEP_TIMEOUT_sec 30    // this will be overwritten by the playlist "wait:_" value
  #define PHOTO_HOLD_PERIOD_sec 900     // 15 minutes pause for the taken photo
  #define NIGHT_INSTR_PERIOD_sec 2880   // instructions (deghosting) period during the night: 48 minutes, which is 10 times in 8 hours. This can still be interrupted every second by an incoming message
  
  
  // Static images parameters
  #define HEADER_LENGTH 54
  #define BUFFPIXEL 80
  const String photo_list_name = "photos.txt";      // .txt file that lists every available photo taken
  const String bmp_prefix = "magic";    // prefix used to save new photos taken
  #define MAGIC_PROBABILITY 60
  
  
  // Appearance parameters:
  #define ANIMATION_DITHER_TYPE AtkinsonDither
  #define IMAGES_DITHER_TYPE AtkinsonDither


#else   // The parameters used ONLY FOR TESTING
  // EPD paramters:
  const uint16_t ep_width = 400, ep_height = 300;
  #define EP_SIZE (ep_width*ep_height)
  const uint8_t STARTUP_CLEAR_CY = 2; // needed to clear the previous image, which might have forced pixels
  #define NIGHT_DEGHOST_CYCLES 1      // cycles per iteration during the night
  #define MIN_DEGHOST_OFFSET 2
  #define ABS_MAX_DEGH_CYCLES 5
  #define AFTER_PHOTO_DEGHOST_CYCLES 1
  #define DEGHOST_FORMULA  _min(((_max(repeat, MIN_DEGHOST_OFFSET) * total_frames) / 12), ABS_MAX_DEGH_CYCLES)
  
  // Communication parameters & pin definitions
  #define BAUD 38400   // Pins: TX=1,  RX=0
  #define TRIGGER_OUT 29
  #define IRQ_receive 30
  #define channel Serial1
  #define MSG_LENGTH 5
  #define MOTION_MSG "MOVMT"
  #define LOWLIGHT_MSG "NIGHT"
  #define GOODLIGHT_MSG "GDDAY"
  
  
  // Execution flow parameters:
  const String playlist = "playlist.txt";    // .txt file that lists every image, animation and pause to be executed sequentially (like a playlist)
  const String RECONVERT = "convert.txt";    // if this file is found on the SD card, it will perform the dithering+byte alignment and save each header file of all animations; it will then delete this file
  #define ANIMAT_char '$'
  #define WAIT_char ':'
  #define STOP_char '.'
  #define SLOWDOWN_PLAYBACK_ms 0
  #define DEFAULT_SLEEP_TIMEOUT_sec 30    // this will be overwritten by the playlist "wait:_" value
  #define PHOTO_HOLD_PERIOD_sec 30      // pause for the taken photo
  #define NIGHT_INSTR_PERIOD_sec 2880   // instructions (deghosting) period during the night: 48 minutes, which is 10 times in 8 hours. This can still be interrupted every second by an incoming message
  
  
  // Static images parameters
  #define HEADER_LENGTH 54
  #define BUFFPIXEL 80
  const String photo_list_name = "photos.txt";      // .txt file that lists every available photo taken
  const String bmp_prefix = "magic";    // prefix used to save new photos taken
  #define MAGIC_PROBABILITY 20
  
  
  // Appearance parameters:
  #define ANIMATION_DITHER_TYPE AtkinsonDither  //PersonalFilterDither
  #define IMAGES_DITHER_TYPE AtkinsonDither

#endif


// FSM constants for playlist execution; actual values are not relevant, as long as they are >= 0.
#define ANIMAT_t 0    // play an image sequence (from playlist)
#define IMAGE_t 1     // display a static image (from playlist)
#define MAGIC_t 2     // get the photo from ESP32
#define WAIT_t 3      // wait for some time (from playlist)
#define STOP_t 4      // full stop (from playlist); can only be woken up by movement detected by the ESP32
#define SLEEP_t 5     // get to sleep, as instructed by ESP32 (low light)
#define WAKE_t 6      // wake up from sleep, as instructed by ESP32 (enough light)
#define UNDEF_t 99


// Other uncathegorized constants
#define ARRAY_CHARS 20    // SD supports a maximum of 8+3 characters for filename+.+extension



// EOF
