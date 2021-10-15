// This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 2.0 Generic License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/2.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.


#define PERFORMANCE_PROFILING false
#define DEBUG true            // when set to false, it will disable every verbose Serial.print (or SD.print)
#define LOGGING_TO_SD true    // this command will only work when DEBUG is defined TRUE

#include "z_Setup.h"

#include <SPI.h>
#include <SD.h>
#include <epd4in2.h>  // E-paper main library
#include <Entropy.h>
#include <Snooze.h>

#include <Dither.h>
#include <ImageManipulation.h>

Epd epd;

SnoozeTimer timer;
SnoozeBlock config_teensy35(timer);

ImgManip img_conv;
Dither static_dt(ep_width, ep_height);
Dither anim_preload(ep_width, ep_height);

String filename;    // global since it's used also inside some helping functions
int16_t loop_length;   // global since this value is evaluated inside the setup() section, and then used in the loop

#if PERFORMANCE_PROFILING
uint32_t q;
#endif

void setup() {
#if DEBUG
  Serial.begin(115200);
  delay(2000);
#endif

  Entropy.Initialize();

  initSleep();
  
  configureComm();

  if (epd.Init(7, 4) != 0) {
    assertCritical(ERROR_ASSERT, "e-Paper init failed!");
  }

  deghost(STARTUP_CLEAR_CY);

  epd.Sleep();
  delay(200);
  epd.Wake(7, 4);

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println(F("failed!"));
    epd.Sleep();
    while (1) {
      delay(1);
    }
  }
  debugPrintln(F("OK."));

  assertCritical(checkFileExists(playlist), "playlist file not found!");
  loop_length = getListLength(playlist) - 1;    // the very last line is just to indicate the file is over (an underscore). So do not count it.
  assertCritical(loop_length > 0, "setup: loop_length < 0 or expected future division by 0");

  bool conversion_requested = checkConversionRequest(RECONVERT);

  if (conversion_requested) {
    epd.Sleep();

    uint16_t frame_number_start = 1;
    uint16_t frame_number_end = 1;
    uint16_t repeat;
    int16_t wait_period_sec = -1;
    int8_t type = UNDEF_t;

    for (uint16_t j = 0; j < loop_length; j++) {
      String curr_filename = fetchNextBlock(playlist, loop_length, type, frame_number_start, frame_number_end, repeat, wait_period_sec);
      debugPrint(("Currently fetching file: " + curr_filename + " - "));
      if (type == ANIMAT_t) {
        bool conversion_needed = checkConversionNeeded(curr_filename, frame_number_start);
        if (conversion_needed) {
          animationCompress(curr_filename, frame_number_start, frame_number_end);
          debugPrintln("done!");
        }
        else {
          debugPrintln("conversion not needed.");
        }
      }
      else  debugPrintln("not an animation. Checking next one...");
    }
    debugPrintln("Finished!");
    setConversionDone(RECONVERT);

    epd.Wake(7, 4);
  }
  else{
    debugPrintln("Conversion not needed.");
  }

}

void loop() {

  uint8_t out_buffer[EP_SIZE];

  // execution flow variables
  static bool load_next_file = true;
  static bool enable_stop = true;
  static bool enable_sleep_init = true;

  // parameters parsed and fetched from .txt file instructions
  static uint16_t frame_number_start = 1;
  static uint16_t frame_number_end = 1;
  static uint16_t repeat;
  static int16_t wait_period_sec = -1;
  static int8_t type = UNDEF_t;   // what state of the FSM to be in

  // animation / static images variables
  static String curr_filename = "";
  static uint16_t frame_number = 0, cycle_cnt = 0;
  static bool force_strong_LUTs = true;


  if (pollESP()) {  // a request from EPS has come through
    if (load_next_file  ||  type == WAIT_t  ||  type == STOP_t  ||  type == SLEEP_t) { // leave executing file playing (unless it's a delay or a stop from playlist); as soon as it's over ( load_next_file ), interrupt just before a new loading section
      if(type == STOP_t){
        epd.Wake(7, 4);
      }
      
      if (wait_period_sec >= 0) { // if command or photo taken during a WAIT_t
        wait_period_sec = -1;     // force wait period to -1, so that we go wake the EPD, deghost, and in the next cycle fetch the photo
      }
      else{    // once wait_period_sec = -1 (forced to), get the photo (or command)
        type = MAGIC_t;   // load "photo" in the FSM state
        load_next_file = false;   // disable next file loading: it's time to get the photo!
      }
    }
  }


  if (load_next_file  &&  type != SLEEP_t  &&  type != WAKE_t) {  // in order to keep executing playlist, the FSM must not be in sleep, nor wake states
    curr_filename = fetchNextBlock(playlist, loop_length, type, frame_number_start, frame_number_end, repeat, wait_period_sec);
    debugPrint("    Currently seeking and about to play file: ");
    if(curr_filename != ""){
      debugPrintln(curr_filename);
    }
    else{
      debugPrintln("_a command_");
    }
    load_next_file = false;
  }


  switch (type) {
    case ANIMAT_t: {
        static bool anim_init = true;
        if (anim_init) {
          // This state will set a high refresh rate to play videos. At the end, it will restore the 50Hz rate.
          // Unfortunately, the display is likely to be in sleep mode, so we first need to wake it up and deghost the last image.
          epd.Reset();
          epd.Init(7, 4);
          deghost(AFTER_PHOTO_DEGHOST_CYCLES);

          epd.Reset();
          epd.Init();   // set high speed refresh rate
          anim_init = false;
        }

        char fn_arr[ARRAY_CHARS];
        filename = curr_filename + String(frame_number + frame_number_start) + ".h";
        filename.toCharArray(fn_arr, filename.length() + 1);
        frame_number = (frame_number + 1) % (frame_number_end - frame_number_start + 1);

#if PERFORMANCE_PROFILING
        uint32_t q = millis();
#endif
        fileLoad(fn_arr, out_buffer, (EP_SIZE / 8));

#if PERFORMANCE_PROFILING
        debugPrintln("              Image loading took " + String(millis() - q) + "ms.");
#endif

#if PERFORMANCE_PROFILING
        q = millis();
#endif
        epd.WaitUntilIdle();

#if PERFORMANCE_PROFILING
        if (millis() - q > 0)  debugPrintln("              EPD halted the execution for another " + String(millis() - q) + "ms.");
        else  debugPrintln("              EPD didn't slow down the execution (bottleneck is elsewhere).");
#endif

#if (SLOWDOWN_PLAYBACK_ms > 0)
        debugPrintln("Another " + String(SLOWDOWN_PLAYBACK_ms) + "ms will be added to slow down playback.");
        delay(SLOWDOWN_PLAYBACK_ms);
#endif

        epd.SetPartialWindow(out_buffer, 0, 0, ep_width, ep_height);
        epd.DisplayFrameQuickAndHealthy(force_strong_LUTs);
        force_strong_LUTs = false;

        if(frame_number == 0){
          cycle_cnt++;
          if(cycle_cnt >= repeat){

            epd.Reset();
            epd.Init(7, 4); // restore slow refresh rate of 50Hz

            deghost(DEGHOST_FORMULA(repeat, frame_number_end, frame_number_start));   // use deghosting cycle value much higher than the one used for photos
            epd.WaitUntilIdle();

            load_next_file = true;
            force_strong_LUTs = true;
            anim_init = true;
            cycle_cnt = 0;
          }
        }

      } break;

    case IMAGE_t: {
        deghost(AFTER_PHOTO_DEGHOST_CYCLES);

        static uint8_t magic_probability_dynamic = MAGIC_PROBABILITY;
        uint8_t random_percent =  Entropy.random(0, 100);
        bool chosen_magic_photo = (magic_probability_dynamic > random_percent);
        uint16_t list_length = getListLength(photo_list_name);
        
        debugPrint("Chose photo from ");
        if (chosen_magic_photo  &&  list_length > 0) {
          uint16_t random_line =  Entropy.random(0, list_length);
          filename = getListLine(photo_list_name, random_line);    // get the random photo name from the list
          magic_probability_dynamic /= 2;    // This time we chose a photo; now, reduce the probability of getting a magic photo next time
          debugPrint("Magic Photo list: ");
        }
        else {  // load the usual photo file from the playlist
          filename = curr_filename + ".bmp";
          magic_probability_dynamic = MAGIC_PROBABILITY;    // restore the probability of getting a magic photo next time
          debugPrint("playlist: ");
        }
        debugPrintln(filename);

        char fn_arr[ARRAY_CHARS];
        filename.toCharArray(fn_arr, filename.length() + 1);

        bmpLoad(fn_arr, 0, 0, out_buffer);

        static_dt.IMAGES_DITHER_TYPE(out_buffer);

        img_conv.gray256To8bits(out_buffer, ep_width, ep_height);
        epd.SetPartialWindow(out_buffer, 0, 0, ep_width, ep_height);
        epd.DisplayFrame();
        epd.WaitUntilIdle();

        load_next_file = true;
      } break;

    case MAGIC_t: {
        static int32_t photo_hold_sec = PHOTO_HOLD_PERIOD_sec;

        static bool begin_photo = true;
        if (begin_photo) {
          memset(out_buffer, 0, (EP_SIZE / 8)); // flush the old buffer (needed as sometimes we don't quite get the entire image data...)

          debugPrint("Request from ESP32...");
          ackESP();   // give the ESP32 the acknowledgement
          uint32_t buff_size = pullBuffer(out_buffer, (EP_SIZE / 8));
          debugPrintln("received data: " + String(buff_size) + " bytes.");

          if (buff_size < (3 * MSG_LENGTH)) { // if less than 3*MSG_LENGTH chars have come through, suspect a message might have been received instead of the photo data
            String incom_msg = getFirstChars(out_buffer, MSG_LENGTH);
            debugPrint("Message received: ");
            debugPrint(incom_msg);
            debugPrint("; ");

            String low_msg = LOWLIGHT_MSG;
            if (incom_msg.equals(low_msg)) {
              debugPrint("correctly interpreted as: ");
              debugPrintln(LOWLIGHT_MSG);
              type = SLEEP_t;
            }

            String good_msg = String(GOODLIGHT_MSG);
            if (incom_msg.equals(good_msg)) {
              debugPrint("correctly interpreted as: ");
              debugPrintln(GOODLIGHT_MSG);
              type = WAKE_t;
            }

            String motion_msg = String(MOTION_MSG);
            if (incom_msg.equals(motion_msg)) {
              debugPrint("correctly interpreted as: ");
              debugPrintln(MOTION_MSG);
              type = WAKE_t;
            }

            if(type == MAGIC_t){
              assert(ERROR_ASSERT, "Message NOT CORRECTLY interpreted.");
              debugPrintln("Defaulting to STOP_t.");
              type = STOP_t;
            }

            // assignments that will be skipped because of the next break
            photo_hold_sec = PHOTO_HOLD_PERIOD_sec;   // restore waiting period
            begin_photo = true;

            break;
          }

          deghost(AFTER_PHOTO_DEGHOST_CYCLES);

          epd.WaitUntilIdle();
          epd.SetPartialWindow(out_buffer, 0, 0, ep_width, ep_height);
          epd.DisplayFrame();

          uint16_t last_curr_index = getLastIndex(photo_list_name);
          String photo_name = savePhotoBMP(out_buffer, bmp_prefix, photo_list_name, last_curr_index);
          appendToList(photo_list_name, photo_name);
          debugPrintln("Photo saved to SD card.");

          debugPrintln("Sleeping for " + String(photo_hold_sec) + " seconds (" + String(photo_hold_sec/60) + " minutes); fixed in SW, same as the other MCU.\n");
          delay(50);

          epd.WaitUntilIdle();
          epd.Sleep();

          begin_photo = false;
        }
        
        photo_hold_sec--;
        if (photo_hold_sec >= 0) {
           sleepTeensy();
           delay(1);
        }
        else{
          // DO NOT deghost here; this state might (often) be called right before a wait; it's not good to wait with no image to see...
          epd.Wake(7, 4);
          epd.WaitUntilIdle();
  
          photo_hold_sec = PHOTO_HOLD_PERIOD_sec;   // restore waiting period
          begin_photo = true;
          load_next_file = true;
        }
      } break;

    case WAIT_t: {
        static bool begin_wait = true;
        if (begin_wait) {
          debugPrintln("Sleeping for " + String(wait_period_sec) + " seconds.");
          delay(50);

          epd.Sleep();
          begin_wait = false;
        }

        wait_period_sec--;
        if (wait_period_sec >= 0) {
          sleepTeensy();
          delay(1);
        }
        else{
          epd.Wake(7, 4);
          begin_wait = true;    // enable next waiting session
          load_next_file = true;
        }
      } break;

    case STOP_t: {
        if (enable_stop) {
          epd.Sleep();
          debugPrintln("Stopping, as instructed by playlist.");
          enable_stop = false;
        }
        sleepTeensy();
        delay(1);
        
      } break;

    case SLEEP_t: {
        static int32_t sleep_timer_sec = NIGHT_INSTR_PERIOD_sec;

        if (enable_sleep_init) {
          debugPrintln("Sleeping, as asked by ESP32 (lowlight condtion). " + String(sleep_timer_sec / 60) + " minutes to go before the first deghosting cycle, if uninterrupted.");
          wait_period_sec = -1;   // set to -1 so that the ESP32 can interrupt if needed; not an elegant/intuitive solution, but it works.
          enable_sleep_init = false;
        }

        sleep_timer_sec--;
        if (sleep_timer_sec >= 0) {
          sleepTeensy();
          delay(1);
        }
        else {
          debugPrint("Cycle completed; ");
          deghost(NIGHT_DEGHOST_CYCLES);
          epd.WaitUntilIdle();

          epd.Sleep();

          sleep_timer_sec = NIGHT_INSTR_PERIOD_sec;   // reload timer value
        }
      } break;

    case WAKE_t: {
        debugPrintln("Resuming playlist, as instructed by ESP32.");

        epd.Reset();
        epd.Init(7, 4);
        epd.WaitUntilIdle();

        type = UNDEF_t;   // restart from old point in playlist
        
        enable_stop = true;
        enable_sleep_init = true;
        load_next_file = true;

      } break;

    default: {
        assert(ERROR_ASSERT, "Defaulting to undefined case.");
        deghost(AFTER_PHOTO_DEGHOST_CYCLES);    // not knowing where coming from, assume EPD has to be deghosted
      }
  }


}




// EOF
