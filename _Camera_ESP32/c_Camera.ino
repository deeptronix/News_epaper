
bool detectFaces(camera_fb_t* frame, uint8_t* stored_fb){
  bool face_detected = 0;

  stored_fb = frame->buf;

  // scale down the photo
  img_conv.scale(stored_fb, wid, heig, MUL_FD, DIV_FD);
  uint16_t out_w = img_conv.getNewWidth();
  uint16_t out_h = img_conv.getNewHeight();
  
  // crop and center the scaled photo
  uint16_t horiz_offset = (out_h - 240)/2;
  uint16_t vert_offset = (out_w - 320)/2;
  img_conv.offsetAndCrop(stored_fb, out_w, out_h, 320, 240, horiz_offset, vert_offset);
  uint32_t len = (320*240);

  //dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);   // 3: # of color channels
  
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, 320, 240, 3);   // 3: # of color channels
  fmt2rgb888(stored_fb, len, frame->format, image_matrix->item);
  esp_camera_fb_return(frame);

  box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
  
  if (boxes != NULL) {
    face_detected = true;
 
    free(boxes->score);
    free(boxes->box);
    free(boxes->landmark);
    free(boxes);
  }

  dl_matrix3du_free(image_matrix);

  return face_detected;
}


uint16_t detectMotion(camera_fb_t* frame, uint8_t* stored_fb){
  uint32_t index;
  uint32_t motion_cnt = 0;
  uint8_t pixel, old_pixel, abs_diff;

  stored_fb = frame->buf;
  esp_camera_fb_return(frame);
  
  img_conv.scale(stored_fb, wid, heig, MUL_MD, DIV_MD);
  uint16_t out_w = img_conv.getNewWidth();
  uint16_t out_h = img_conv.getNewHeight();

  static uint8_t* old_buff = (uint8_t*)heap_caps_malloc((out_w*out_h), MALLOC_CAP_SPIRAM);
  _assert(old_buff != NULL, "detectMotion: allocation of 'old_buff'");

  for(uint16_t y = 0; y < out_h; y++){
    for(uint16_t x = 0; x < out_w; x++){
      index = x + y*out_w;
      
      pixel = stored_fb[index];
      old_pixel = old_buff[index];

      abs_diff = abs(pixel - old_pixel);
      if(abs_diff > PIXEL_MOTION_THRESHOLD)   motion_cnt++;
      
      old_buff[index] = pixel;   // update old buffer
    }
  }

  motion_cnt = (100*motion_cnt) / (out_w*out_h);    // convert to percentage
  return motion_cnt;  
}


uint16_t getLightAmount(camera_fb_t* frame, uint8_t* stored_fb){

  stored_fb = frame->buf;
  esp_camera_fb_return(frame);
      
  uint8_t pixel;
  uint32_t orig_ind = 0;
  uint16_t x_orig, y_orig;
  uint32_t lightness = 0;

  uint16_t mul = MUL_MD;
  uint16_t div = DIV_MD;

  uint16_t out_w = img_conv.getNewDimension(wid, mul, div);
  uint16_t out_h = img_conv.getNewDimension(heig, mul, div);
  
  for(uint32_t y = 0; y < out_h; y++){
    for(uint32_t x = 0; x < out_w; x++){

      x_orig = round((float)(x*div)/mul);
      y_orig = round((float)(y*div)/mul);
      orig_ind = x_orig + y_orig*wid;
      
      pixel = stored_fb[orig_ind];   // read pixel value
      
      if(pixel > PIXEL_LIGHT_THRESHOLD){
        lightness++;
      }
    }
  }

  return ((100*lightness)/(out_w*out_h));
}



void cameraAutoAdjust(uint8_t cycles){
  camera_fb_t* frame;
  for(uint8_t c = 0; c < cycles; c++){
    // fetch camera image, in order to update brightness auto adjustment
    frame = esp_camera_fb_get();
    esp_camera_fb_return(frame);
    delay(50);
  }
}


void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  
  config.frame_size = FRAMESIZE;
  config.jpeg_quality = 1;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK){
    Serial.printf("Camera init failed with error 0x%x", err);
    _assertCritical(ERROR_ASSERT, "Camera init failed with error 0x%x" + String(err));
  }
  else  debugPrintln("Camera initialized.");
}



void initMTMN(){
  mtmn_config = mtmn_init_config();
  mtmn_config.p_threshold.score = 0.55;
  mtmn_config.r_threshold.score = 0.45;
  mtmn_config.o_threshold.score = 0.4;
  mtmn_config.r_threshold.candidate_number = 8;
  mtmn_config.o_threshold.candidate_number = 2;
}





// eof
