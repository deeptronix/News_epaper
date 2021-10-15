// © 2021 by Deep Tronix. Visit my website @ rebrand.ly/deeptronix
// This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 2.0 Generic License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/2.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.


// WARNING! Make sure that you have selected ESP32 Wrover Module from the compiler boards.
// Other settings, crucial and not, are:
// - Flash Freq. = 800MHz
// - partition scheme = Huge APP (3MB No OTA/1MB SPIFFS)
// - Core debug level = Verbose

#define PERFORMANCE_PROFILE false     // when set true, it will print how much time the loop has taken; useful for debugging and power analysis
#define DEBUG true      // when set true, it will enable every Serial.print

#include "z_Setup.h"
#include "esp_camera.h"
#include "camera_pins.h"
#include "fd_forward.h"

#include <Dither.h>
#include <ImageManipulation.h>

Dither photo_dt(((wid * MUL)/DIV), ((heig * MUL)/DIV));
ImgManip img_conv;

// holds the MTMN configuration settings for face detection; filled in c_Camera -> initMTMN();
mtmn_config_t mtmn_config = {0};

char message[MSG_LENGTH];

void setup(){
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  debugPrintln("");

  initComm();

  initCamera();

  initMTMN();

  debugPrint("ESP32 init over; startup delay (" + String(STARTUP_DELAY_sec) + "s)... ");
  delay(STARTUP_DELAY_sec*1000);
  debugPrintln("over.");
  
}

void loop(){

  #if PERFORMANCE_PROFILE
  static uint32_t idle_time_ms = 0;
  #endif
  
  static uint8_t* stored_fb = (uint8_t*)heap_caps_malloc(IMAGE_SIZE, MALLOC_CAP_SPIRAM);
  _assert(stored_fb != NULL, "main: allocation of 'stored_fb'");

  // Timing variables
  static uint16_t face_interval_dynamic_ms = FACE_SEARCH_INTERVAL_SLOW_ms;
  static int32_t timer_face_ms = 0, timer_face_stop_ms = 0, timer_mot_ms = 0, timer_inh_ms = 0, timer_light_ms = 0;
  static uint32_t loop_time_ms;

  // FSM control
  static bool search_for_faces = false, detect_motion = false, check_light = false;
  static bool update_detection = true;
  static bool mot_inhibited = false;
  static bool in_sleep = false;

  static bool camera_begin = true;
  if(camera_begin){
    cameraAutoAdjust(3);
    camera_begin = false;
  }
  
  
#if PERFORMANCE_PROFILE
  uint32_t q = millis();
#endif

  // initialize a camera_frame type
  static camera_fb_t *frame;

  if(search_for_faces){
    debugPrint("Searching for faces...");
    frame = esp_camera_fb_get();
    if(detectFaces(frame, stored_fb) == false){
      debugPrintln("not found.");
    }
    else{
      debugPrintln("");
      debugPrint("Face detected. Currently at 0..");
      int8_t face_count = 0;
#if FACES_PER_SESSION > 0
      for(uint8_t i = 0; (i < SESSION_FRAMES  &&  face_count < FACES_PER_SESSION); i++){
        delay(50);
        camera_fb_t *frame = esp_camera_fb_get();
        if(detectFaces(frame, stored_fb)){
          face_count++;
          debugPrint(String(face_count) + "..");
        }
      }
#endif
      if(face_count >= FACES_PER_SESSION){
        memset(stored_fb, 0, IMAGE_SIZE);   // flush current image buffer: get a new one which has not been resized (by detectFaces)
        
        frame = esp_camera_fb_get();
        stored_fb = frame->buf;
        esp_camera_fb_return(frame);   // NEEDED; return the frame buffer back to the driver for reuse
        flashLED();
  
        debugPrint(" Confidently detected!\nScaling image ");
        img_conv.scale(stored_fb, wid, heig, MUL, DIV);
        uint16_t new_w = img_conv.getNewWidth();
        uint16_t new_h = img_conv.getNewHeight();
        debugPrint("(new size: " + String(new_w) + "x" + String(new_h) + "), ");

        debugPrint("dithering, ");
        photo_dt.DITHERING_TYPE(stored_fb);     // at the cost of taking slightly longer, preserve the border's correct appearance by dithering BEFORE cropping to the final size

        debugPrint("rotating ");
        static uint8_t* temp_buffer = (uint8_t*)heap_caps_malloc((new_w*new_h), MALLOC_CAP_SPIRAM);   // a second buffer is needed for rotation, since in-place rotation is very complicated to perform (and RAM space is available)
        _assert(temp_buffer != NULL, "main: allocation of 'temp_buffer'");
        img_conv.rotate90deg(stored_fb, temp_buffer, new_h, new_w);
        debugPrint("(new size: " + String(new_h) + "x" + String(new_w) + "), ");

        debugPrint("cropping ");
        uint16_t horiz_offset = (new_h - ep_width)/2;   // yes, it's correct: new_h is the frame height, but the image width.
        uint16_t vert_offset = (new_w - ep_height)/2;   // yes, it's correct: new_w is the frame width, but the image height.
        img_conv.offsetAndCrop(temp_buffer, new_h, new_w, ep_width, ep_height, horiz_offset, vert_offset);
        debugPrintln("(new size: " + String(ep_width) + "x" + String(ep_height) + " - portrait).");
        
        debugPrint("Preparing for communication: byte-to-bit alignment... ");
        img_conv.gray256To8bits(temp_buffer, ep_width, ep_height);
        debugPrintln("Done!");
        
        debugPrint("Asking Teensy for response...");
        sendRequestStatus(CLEAR);    // clear previous requests
        requestTeensy();
        debugPrint("acknowledged!\nSending buffer...");
        pushBuffer(temp_buffer, ((ep_width/8)*ep_height));
        debugPrintln("Image data sent.");
        
        debugPrintln("Sleeping for " + String(PHOTO_HOLD_PERIOD_sec) + " seconds (" + String(PHOTO_HOLD_PERIOD_sec/60) + " minutes); fixed in SW, same as the other MCU.\n");
        delay(PHOTO_HOLD_PERIOD_sec * 1000);
        
        // since the idling period after a photo is supposed to be quite long, better reset the motion inhibition timer, otherwise it will take too long for a new motion to get through
        detect_motion = true;
        update_detection = true;
        mot_inhibited = false;
        face_interval_dynamic_ms = FACE_SEARCH_INTERVAL_SLOW_ms;
      }
    }
    
    search_for_faces = false;
  }


  
  if(detect_motion){
    if(update_detection){
      frame = esp_camera_fb_get();
      detectMotion(frame, stored_fb);    // do a "useless" run to update the internal comparison buffer in order not to fire immediately once powered up
      update_detection = false;
    }
    
    frame = esp_camera_fb_get();
    uint8_t motion_percent = detectMotion(frame, stored_fb);
    debugPrintln("Motion amount: " + String(motion_percent) + "% of the frame.");
    
    if(motion_percent >= MOTION_DETECT_PERCENT){
      debugPrintln("Enough motion detected! Queueing to tell Teensy.");
      sendRequestStatus(PENDING);
      strcpy(message, MOTION_MSG);

      mot_inhibited = true;   // inhibit motion detection
      timer_inh_ms = 0;      // reset inhibition timer, to start counting
      debugPrintln("Motion detection inhibited for the next " + String(AFTERMOTION_INHIBIT_sec) + " seconds. Now searching for faces every " + String(FACE_SEARCH_INTERVAL_FAST_ms) + "ms.");
      face_interval_dynamic_ms = FACE_SEARCH_INTERVAL_FAST_ms;
      timer_face_stop_ms = 0;    // reset face search stop timer, so that it can start counting
    }
    
    detect_motion = false;
  }




  if(check_light){
    frame = esp_camera_fb_get();
    uint8_t light_percent = getLightAmount(frame, stored_fb);

    debugPrintln("Light measured: " + String(light_percent) + "% of the frame.");
  
    static uint8_t lowlight_cnt = 0;
    if(light_percent < MIN_LIGHT_PERCENT  &&  !in_sleep)  lowlight_cnt++;    // start the lowlight count, but keep still once sleeps starts
    else{
      if(light_percent > MIN_LIGHT_PERCENT)  lowlight_cnt = 0;   // reset the counter if light is once again detected
    }
    
    if(lowlight_cnt >= LOWLIGHT_FRAME_CNT  &&  !in_sleep){
      debugPrintln("It's night time! Going to sleep... Queueing to tell Teensy.");
      sendRequestStatus(PENDING);
      strcpy(message, LOWLIGHT_MSG);
      
      in_sleep = true;
    }
    if(lowlight_cnt == 0  &&  in_sleep){
      debugPrint("It's day time! Queueing to tell Teensy.");
      sendRequestStatus(PENDING);
      strcpy(message, GOODLIGHT_MSG);
      debugPrintln("Starting looking for faces...");
      
      update_detection = true;    // force motion buffer update after sleep
      in_sleep = false;
    }
    
    check_light = false;
  }



  

  loop_time_ms = millis() - loop_time_ms;
  uint32_t remaining_ms = loop_time_ms;
  if(loop_time_ms < MIN_IDLE_DELAY_ms){
    remaining_ms = MIN_IDLE_DELAY_ms - loop_time_ms;
    delay(remaining_ms);
  }
  
  timer_face_ms += remaining_ms;
  timer_face_stop_ms += remaining_ms;
  timer_mot_ms += remaining_ms;
  timer_inh_ms += remaining_ms;
  timer_light_ms += remaining_ms;

  loop_time_ms = millis();

  if(!in_sleep){
    if(timer_face_ms >= face_interval_dynamic_ms){
      timer_face_ms = 0;    // reset timer
      search_for_faces = true;
    }
    if(timer_face_stop_ms >= FACE_SEARCH_STOP_ms  &&  face_interval_dynamic_ms == FACE_SEARCH_INTERVAL_FAST_ms){
      // load slow face detection timer value.
      // This way this condition will be met only after FACE_SEARCH_STOP_ms from the FAST search of faces.
      face_interval_dynamic_ms = FACE_SEARCH_INTERVAL_SLOW_ms;
      
      debugPrintln("Loaded SLOW FD timer (" + String(FACE_SEARCH_INTERVAL_SLOW_ms) + "ms), since " + String(timer_face_stop_ms) + "ms have elapsed from the face search start.");
    }

    if(!mot_inhibited){    // if not inhibited, enable motion detection section
      if(timer_mot_ms >= MOTION_SEARCH_INTERVAL_ms){
        detect_motion = true;
        timer_mot_ms = 0;    // reset timer for when it will be inhibited
        face_interval_dynamic_ms = FACE_SEARCH_INTERVAL_SLOW_ms;   // reload slow face search interval
      }
    }
    else{
      if(timer_inh_ms >= (AFTERMOTION_INHIBIT_sec*1000)){
        mot_inhibited = false;      // re-enable motion detection
        update_detection = true;    // update comparison buffer
      }
    }
  }
  
  if(timer_light_ms >= (LIGHT_SEARCH_INTERVAL_sec*1000)){
    timer_light_ms = 0;    // reset timer
    check_light = true;
  }

  bool send_request = sendRequestStatus(READ);  // check if a message to be sent is pending
  if(send_request){
    static uint32_t timeout_message_ms = 0;
    bool sent = sendQueue(message);
    if(!sent){
      timeout_message_ms += remaining_ms;     // allow a maximum amount of time before "watchdog" is fired
      uint16_t timeout_sec = timeout_message_ms/1000;
      // _assertCritical(timeout_sec < TIMEOUT_MESSAGE_sec, "requestTeensy: waiting for Teensy MESSAGE acknowledge in time (" + String(timeout_sec) + " seconds).", false);
      
      // It may not be required such a drastic solution; a better idea could be to simply ignore and keep going:
      _assert(timeout_sec < TIMEOUT_MESSAGE_sec, "requestTeensy: waiting for Teensy MESSAGE acknowledge in time (" + String(timeout_sec) + " seconds).", false);
      
      if(timeout_sec > TIMEOUT_MESSAGE_sec){
        sendRequestStatus(CLEAR);    // clear the unsatisfied request
        timeout_message_ms = 0;     // reset "watchdog" timer
      }
    }
    else{
      timeout_message_ms = 0;     // reset "watchdog" timer
    }
  }
  

#if PERFORMANCE_PROFILE
  uint32_t elapsed_ms = millis() - q;
  if(elapsed_ms <= MIN_IDLE_DELAY_ms){
    if(!idle_time_ms){
      debugPrintln("Idling...");
    }
    idle_time_ms += remaining_ms;
  }
  else{
    debugPrintln("Last elapsed time: " + String(elapsed_ms) + "ms.");
    uint32_t total_elapsed_ms = idle_time_ms + elapsed_ms;
    if(total_elapsed_ms > 0){   // division by 0 is no good
      uint16_t CPU_occup_percent = 100*elapsed_ms / total_elapsed_ms;
      debugPrintln("Average current CPU occupation: " + String(CPU_occup_percent) + "%");
      idle_time_ms = 0;
    }
  }
#endif
  
}




// eof
