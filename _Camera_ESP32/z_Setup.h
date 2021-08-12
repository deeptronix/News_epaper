
// Pin definition and communication parameters
#define BAUD 57600
#define channel Serial2
#define RXD2 16   // not used, but definition needed
#define TXD2 15
#define ACK_IN 14
#define IRQ_to_Teensy 2
#define LED 4

  // Sent photo characteristics
const uint16_t ep_width = 400, ep_height = 300;
#define DITHERING_TYPE  AtkinsonDither

  // Select camera model and parameters
#define CAMERA_MODEL_AI_THINKER

#define FRAMESIZE  FRAMESIZE_VGA
#define wid 640
#define heig 480
#define IMAGE_SIZE (wid*heig)

// Make the output image height compatible with the (rotated) EP display; the width will be offset.
#define MUL 17
#define DIV 20

// For face detection, scale down and crop the image. At the end the image will be 320x240
#define MUL_FD 12
#define DIV_FD 20

// For motion detection, the image can be much smaller so that it's scanned faster; also, an OLD BUFFER must be always kept, so memory constraints are important
#define MUL_MD 1
#define DIV_MD 20


// Flow parameters
#define STARTUP_DELAY_sec 15    // give Teensy some time to deghost and start the playlist
#define MIN_IDLE_DELAY_ms 20
#define TEENSY_COMM_TIMEOUT_ms 120000   //  after this many ms the communication with Teensy is known to be broken, so we restart the ESP32

#define FACE_SEARCH_INTERVAL_SLOW_ms 4000   // when no motion is detected (and after FACE_SEARCH_STOP_ms), search for faces but not frequently
#define FACE_SEARCH_INTERVAL_FAST_ms 1000    // once motion is detected, start searching for faces quickly
#define FACE_SEARCH_STOP_ms 40000        // after this many ms, the face detection will again slow down in order to reduce false triggering
#define PHOTO_HOLD_PERIOD_sec 1800    // hold photo taken (inhibit new acquisition) for some time
#define FACES_PER_SESSION 1     // can be as low as 0; 1 face will always be needed in order to start detection loop
#define SESSION_FRAMES 15       // once a face is detected, search for this many times if a number of FACES_PER_SESSION faces is detected.

#define PIXEL_LIGHT_THRESHOLD 80    // if pixels have a lightness higher than this value (range is 0~255), it's considered sufficiently exposed to light
#define MIN_LIGHT_PERCENT 30    // if at least this percentage of pixels is above the aforementioned threshold, consider the room to be lit
#define LOWLIGHT_FRAME_CNT 3      // count this many frames before telling Teensy to go into "night" mode
#define LIGHT_SEARCH_INTERVAL_sec 20   // how often to check the state of room illumination

#define MOTION_SEARCH_INTERVAL_ms 200
#define PIXEL_MOTION_THRESHOLD 20     // if a pixel sees a difference of at least this much, with respect to the comparison image buffer, count it for motion detection. range is 0~255
#define MOTION_DETECT_PERCENT 20      // once above this percentage threshold, consider enough motion has happened in the frame
#define AFTERMOTION_INHIBIT_sec 3600     // once motion is triggered, how long to wait before telling Teensy to resume playback again (basically, an inhibition delay on motion detection)

  // The messages that can be sent to Teensy to communicate states to go in. Only exactly MSG_LENGTH characters messages are allowed!
#define MSG_LENGTH 5
#define MOTION_MSG "MOVMT"
#define LOWLIGHT_MSG "NIGHT"
#define GOODLIGHT_MSG "GDDAY"
#define STD_ERROR_MSG "STERR"
#define CRT_ERROR_MSG "CRERR"
#define TIMEOUT_MESSAGE_sec  (TEENSY_COMM_TIMEOUT_ms/1000)


// General Macros
#define READ 0
#define WRITE 1

#define PENDING WRITE, 1
#define CLEAR WRITE, 0

#define ERROR_ASSERT 0

// eof
