
/*  @brief: this function is called if an image sequence recompression is requested.
			It simply dithers and byte-aligns each frame of an animation, then saves
			it as .h header file onto the SD card. The performance gain over a basic
			BMP image is quite substantial, about 24 times faster.
			Also, since the image is already dithered, it does not need to be done on 
			the fly during playback.
	@param: the filename as a string of characters of the prefix of the animation to be
			compressed.
	@param: the start frame number, from which the animation begins (frame included).
	@param: the last frame number, where the animation ends (frame included).
*/
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


/*  @brief: helping function used to save an array of data as a .h header file onto the SD card.
	@param: the buffer of data, loaded in RAM, to be saved.
	@param: the size of the array to be written, in bytes.
	@param: the filename prefix given to the SD card .h file.
	@param: since this is used for animations, a frame number follows the name prefix.
*/
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



/*  @brief: loads a .h file from the SD card to a RAM buffer on the uC, as is.
	@param: the filename name of the .h file, as a char array pointer.
	@param: the RAM buffer where we write the content read from the SD card file.
	@param: the size of the file (and buffer), in bytes.
*/
void fileLoad(char* fn_arr, uint8_t* out_buffer, uint32_t size){
  File compressed_file = SD.open(fn_arr);
  for(uint32_t i = 0; i < size; i++){
    out_buffer[i] = compressed_file.read();
  }
  compressed_file.close();
}



// eof
