
#define _max(val_a, val_b)  ((val_a > val_b)?  val_a : val_b)
#define _min(val_a, val_b)  ((val_a < val_b)?  val_a : val_b)

#if LOGGING_TO_SD
#define debug_filename "debug.txt"
#endif

// EPD paramters:
const uint16_t ep_width = 400, ep_height = 300;
#define EP_SIZE (ep_width*ep_height)
#define STARTUP_CLEAR_CY 2             // needed to clear the image at startup, not knowing where it comes from
#define NIGHT_DEGHOST_CYCLES 1         // cycles per iteration during the night
#define AFTER_PHOTO_DEGHOST_CYCLES 1
#define MIN_DEGHOST_OFFSET 2           // used in the formula below
#define ABS_MAX_DEGH_CYCLES 5          // used in the formula below
#define DEGHOST_FORMULA(rep, fn_end, fn_st)  _min(((_max(rep, MIN_DEGHOST_OFFSET) * (fn_end - fn_st + 1)) / 12), ABS_MAX_DEGH_CYCLES)

// Communication parameters & pin definitions
#define BAUD 57600
#define TRIGGER_OUT 5
#define IRQ_receive 6
#define channel Serial1   // Pins: TX=1 (not used),  RX=0
#define MSG_LENGTH 5
#define MOTION_MSG "MOVMT"
#define LOWLIGHT_MSG "NIGHT"
#define GOODLIGHT_MSG "GDDAY"


// Execution flow parameters:
const String playlist = "playlist.txt";    // .txt file that lists every image, animation and pause to be executed sequentially (like a playlist)
const String RECONVERT = "convert.txt";    // if this file is found on the SD card, it will perform the dithering+byte alignment and save each header file 
                                           // of the animations which are missing the first frame (used in the playlist); it will then delete this file.
#define ANIMAT_char '$'
#define WAIT_char ':'
#define STOP_char '.'
#define SLOWDOWN_PLAYBACK_ms 0
#define DEFAULT_SLEEP_TIMEOUT_sec 30    // this will be overwritten by the playlist "wait:_" value
#define PHOTO_HOLD_PERIOD_sec 30      // 1800       // hold photo taken (inhibit playback) for a couple of minutes
#define NIGHT_INSTR_PERIOD_sec 4800   // instructions (deghosting) period during the night: 80 minutes, which is 6 times in 8 hours. This can still 
                                      // be interrupted every 1 sec by an incoming message


// Static images parameters
#define HEADER_LENGTH 54
#define BUFFPIXEL 80
const String photo_list_name = "photos.txt";    // .txt file that lists every available photo taken
const String bmp_prefix = "mag";  	            // prefix used to save new photos taken; the total number of chars must be less than 8, including the suffix number!
#define MAGIC_PROBABILITY 20    // 60


// Appearance parameters:
#define ANIMATION_DITHER_TYPE AtkinsonDither
#define IMAGES_DITHER_TYPE AtkinsonDither



// FSM constants for playlist execution; actual values are not relevant, as long as they are >= 0.
#define ANIMAT_t 0    // play an image sequence (from playlist)
#define IMAGE_t 1     // display a static image (from playlist)
#define MAGIC_t 2     // get the photo from ESP32
#define WAIT_t 3      // wait for some time (from playlist)
#define STOP_t 4      // full stop (from playlist); can only be woken up by movement detected by the ESP32
#define SLEEP_t 5     // get to sleep, as instructed by ESP32 (low light)
#define WAKE_t 6      // wake up from sleep, as instructed by ESP32 (enough light)
#define UNDEF_t 99


// Other uncathegorized constants and Macros
#define ARRAY_CHARS 20    // SD supports a maximum of 8.3 characters for filename.extension
#define ERROR_ASSERT 0
  // Wake ID not used, but available if needed:
// #define TIMER_WAKE_SOURCE 36
// #define RTC_ALARM_SOURCE 35



// EOF
