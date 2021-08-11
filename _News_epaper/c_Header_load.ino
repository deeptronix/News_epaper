
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
  compressed_file.close();
}



// eof
