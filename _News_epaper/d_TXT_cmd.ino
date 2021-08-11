
/*	@brief: given the filename of a .txt list, it counts the number of lines in it.
	@param: the string name of the list
	@returns: the number of lines
*/
uint16_t getListLength(String list_name){

  char fn_arr[ARRAY_CHARS];
  File list_file;
  uint16_t line_count = 0;
  
  filename = list_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  list_file = SD.open(fn_arr);
  
  while(list_file.available()){
    String line = list_file.readStringUntil('\n');
    line_count++;
    assert(line_count > 0, "getListLength");
  }
  
  list_file.close();
  
  //debugPrintln("\nCounted " + String(line_count) + " lines.");
  
  return line_count;
}


/*	@brief: given the filename of the playlist, it fills in the various information needed
			to "play" the next file, to be used from subsequent blocks.
	@param: the filename string of the playlist
	@param: the length of the playlist, in order to restart when it gets exhausted
	@param: reference to the next block type
	@param: reference to the number of the first index, for animations
	@param: reference to the number of the last index, for animations
	@param: reference to the number of image sequence repetitions, for annimations
	@param: reference to the wait period, when the command is a "wait:" type
	@returns: the name of the file to be played back.
*/
String fetchNextBlock(String list_name, uint16_t loop_length, int8_t& type, uint16_t& frame_st, uint16_t& frame_end, uint16_t& repeat, int16_t& wait_period_sec){
  static uint16_t curr_line = 0;
  char fn_arr[ARRAY_CHARS];
  File list_file;
  uint16_t line_count = 0;
  String nextfile_name;

  filename = list_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  list_file = SD.open(fn_arr);

  while(list_file.available()){
    String line = list_file.readStringUntil('\n');
    
    if(line_count == curr_line){
      
      int8_t symbol = line.indexOf(ANIMAT_char);
      if(symbol != -1){   //  an animation type
        type = ANIMAT_t;
        nextfile_name = line.substring(0, symbol);    // return the filename, bounded before the symbol and the number
        
        int8_t dash = line.indexOf('-');
        String str_start = line.substring(symbol+1, dash);
        int8_t cross = line.indexOf('x');
        String str_end = line.substring(dash+1, cross);
        String str_repeat = line.substring(cross+1, line.length()-1);
        frame_st = str_start.toInt();
        frame_end = str_end.toInt();
        repeat = str_repeat.toInt();

        //debugPrintln(frame_st);
        //debugPrintln(frame_end);
        //debugPrintln(repeat);
        break;
      }

      symbol = line.indexOf(WAIT_char);
      if(symbol != -1){   // a wait command
        type = WAIT_t;
        String str_wait = line.substring(symbol+1, line.length()-1);   // parse the wait period to the caller
        wait_period_sec = str_wait.toInt();
        break;
      }

      symbol = line.indexOf(STOP_char);
      if(symbol != -1){   // a stop command
        type = STOP_t;
        break;
      }

      // if none of the above commands was verified, the only thing left is an image type (not identified by any symbol)
      type = IMAGE_t;
      nextfile_name = line.substring(0, line.length()-1);   // parse the filename to the caller
    }

    line_count++;
    assert(line_count > 0, "fetchNextBlock");
  }

  curr_line = (curr_line+1) % loop_length;    // advance playlist

  list_file.close();
  return nextfile_name;
}


/*	@brief: helping function used to check if an image sequence conversion request is present
	@param: the filename required to start the conversion (found in z_setup.h)
	@returns: TRUE if the conversion has been requested.
*/
bool checkConversionRequest(String file_name){
  if(checkFileExists(file_name)){
    debugPrintln("Conversion command found!");
    return true;
  }
  return false;
}


/*	@brief: if the conversion was requested, after the assertion, it deletes the conversion file so that
			it won't reconvert the sequences if the uC is rebooted.
	@param: the filename required to start the conversion (found in z_setup.h)
*/
void setConversionDone(String file_name){
  char fn_arr[ARRAY_CHARS];
  
  filename = file_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  
  if(SD.exists(fn_arr)){    // check anyway, just in case
    SD.remove(fn_arr);
  }
  else{
    assert(ERROR_ASSERT, "tried removing conversion command, but not present.");
  }
  debugPrintln("Conversion command cleared.");
}


/*	@brief: Checks if the filename of a specific animation requires a conversion.
			Since it wouldn't make much sense to reconvert a perfectly fine file,
			only when a chosen animation index is missing, a reconversion will be 
			performed.
	@param: the filename of the animation to be checked
	@param: the index to be checked, in order to decide if the animation requires
			a conversion or not.
	@returns: TRUE if the conversion is needed.
*/
bool checkConversionNeeded(String file_name, uint16_t frame_number_check){
  filename = file_name + frame_number_check + ".h";
  return (1 - checkFileExists(filename));
}


/*	@brief: used to append a string to the end of a list; mainly used when taking note
			of the photos taken by the ESP32, but also for the debugging log functionality.
	@param: the name of the file where to save the next line of text.
	@param: the string of text to be appended at the end of the file.
*/
void appendToList(String list_name, String text_string){
  char fn_arr[ARRAY_CHARS];
  File list_file;
  
  filename = list_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  list_file = SD.open(fn_arr, FILE_WRITE);

  list_file.println(text_string);

  list_file.close();
}


/*	@brief: given the name of a .txt list and the number of a line, it returns the text that can be found at that line.
	@param: the list name from which get the line of text
	@param: the line number to be read.
	@returns: the content of the list at that line, or "-1" if the list doesn't have that line.
*/
String getListLine(String list_name, uint16_t line_number){
  char fn_arr[ARRAY_CHARS];
  File list_file;
  uint16_t line_count = 0;
  
  filename = list_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  list_file = SD.open(fn_arr);
  
  while(list_file.available()){
    String line = list_file.readStringUntil('\n');
    
    if(line_count == line_number){
      list_file.close();
      return line.substring(0, line.length()-1);
    }
    line_count++;
    assert(line_count > 0, "getListLine");
  }

  list_file.close();
  return "-1";
}


/*	@brief: used to read "specifically" the photo.txt list; this function will read the last line
			of the list, fetch the photo index number and return it so that the next photo can be 
			saved with an increased index. E.g.: if the last photo is called MAG23.bmp , this 
			function will return "23".
	@param: the list name (strictly the list of the photos)
	@returns: the current last index of the photo
*/
uint16_t getLastIndex(String list_name){
  uint16_t llen = getListLength(list_name);
  String last_fname = getListLine(list_name, llen - 1);
  String last_ind_str = last_fname.substring(bmp_prefix.length(), last_fname.length()-4);		// -4 in order to ignore the ".bmp" extension
  uint16_t last_ind = last_ind_str.toInt();
  return last_ind;
}


/*	@brief: Function used to check if a particular file exists on the SD card; mainly used in
			order not to try to read a playlist/photo list file which is not present.
	@param: the name of the file to be checked.
	@returns: TRUE if the file is present.
*/
bool checkFileExists(String file_name){
  char fn_arr[ARRAY_CHARS];
  filename = file_name;
  filename.toCharArray(fn_arr, file_name.length() + 1);
  
  if(SD.exists(fn_arr)){
    return true;
  }
  return false;
}



// eof
