
uint8_t header_400x300[HEADER_LENGTH] = {     // extracted from an actual 400x300 bitmap file; not compatible with other resolutions!
  0x42, 0x4D, 0x76, 0x7E, 0x05, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
  0x90, 0x01, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

String savePhotoBMP(uint8_t* img, String bmp_prefix, String list_name, uint16_t last_index){

  uint8_t out_values[8];
  uint8_t r, g, b;
  uint8_t pixel;
  char fn_arr[ARRAY_CHARS];
  File bmp_file;
  
  filename = bmp_prefix + String(last_index + 1) + ".bmp";
  filename.toCharArray(fn_arr, filename.length() + 1);
  bmp_file = SD.open(fn_arr, FILE_WRITE);

  for(uint8_t i = 0; i < HEADER_LENGTH; i++){
    bmp_file.write(header_400x300[i]);
  }
  for(uint32_t p = 0; p < ((ep_width/8)*ep_height); p++){
    pixel = img[p];
    img_conv.bool8bitsTo8Bytes(pixel, out_values);
    for(uint8_t e = 0; e < 8; e++){
      convert.colorBoolTo888(out_values[e], r, g, b);
      bmp_file.write(r);
      bmp_file.write(g);
      bmp_file.write(b);
    }
  }

  bmp_file.close();

  return (filename.toUpperCase());    // limitation of native SD library; the saved filename will always be uppercase; hence, return an appropriately converted filename.
}


void animationCompress(String curr_filename, uint16_t frame_number_start, uint16_t total_frames){
  char fn_arr[ARRAY_CHARS];

  uint8_t temp_buffer[ep_width*ep_height];

  for(uint16_t frame_number = frame_number_start; frame_number <= total_frames; frame_number++){
    filename = curr_filename + String(frame_number) + ".bmp";
    filename.toCharArray(fn_arr, filename.length() + 1);
    
    bmpLoad(fn_arr, 0, 0, temp_buffer);
    //debugPrintln("Image successfully loaded into RAM.");

    anim_preload.ANIMATION_DITHER_TYPE(temp_buffer);
    //debugPrint("Dithered, ");
    img_conv.gray256To8bits(temp_buffer, ep_width, ep_height);
    //debugPrint("downconverted. Saving to SD card... ");

    uint32_t size = EP_SIZE/8;
    saveHeaderFile(temp_buffer, size, curr_filename, frame_number);
    //debugPrintln(" Done!");
  }
}

void saveHeaderFile(uint8_t* temp_buffer, uint32_t size, String curr_filename, uint16_t frame_number){
  char fn_arr[ARRAY_CHARS];
  
  filename = curr_filename + String(frame_number) + ".h";
  filename.toCharArray(fn_arr, filename.length() + 1);
  File head_file = SD.open(fn_arr, FILE_WRITE);
  //debugPrint("(file created; saving image data... ");

  for(uint32_t p = 0; p < size; p++){
    head_file.write(temp_buffer[p]);
  }

  //debugPrint("done; closing file)");
  head_file.close();
}


void fileLoad(char* fn_arr, uint8_t* out_buffer, uint32_t size){
  File compressed_file = SD.open(fn_arr);
  for(uint32_t i = 0; i < size; i++){
    out_buffer[i] = compressed_file.read();
  }
}



// eof
