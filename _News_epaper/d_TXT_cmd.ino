
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
  }
  
  list_file.close();
  
  //debugPrintln("\nCounted " + String(line_count) + " lines.");
  
  return line_count;
}


String fetchNextBlock(String list_name, uint16_t loop_length, int8_t& type, uint16_t& frame_st, uint16_t& frame_end, uint16_t& repeat, int16_t& wait_period){
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
    //debugPrintln(line);
    
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
        wait_period = str_wait.toInt();
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
  }

  curr_line = (curr_line+1) % loop_length;    // advance playlist

  list_file.close();
  return nextfile_name;
}


bool checkConversionRequest(String file_name){
  char fn_arr[ARRAY_CHARS];
  
  filename = file_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  
  if(SD.exists(fn_arr)){
    debugPrintln("Conversion command found!");
    return true;
  }
  return false;
}

void setConversionDone(String file_name){
  char fn_arr[ARRAY_CHARS];
  
  filename = file_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  
  if(SD.exists(fn_arr)){    // check anyway, just in case
    SD.remove(fn_arr);
  }
  debugPrintln("Conversion command cleared.");
}


bool checkConversionNeeded(String file_name, uint16_t frame_number_start){
  char fn_arr[ARRAY_CHARS];
  
  filename = file_name + frame_number_start + ".h";
  filename.toCharArray(fn_arr, filename.length() + 1);
  
  if(SD.exists(fn_arr)){    // if file already exists, do not perform conversion
    return false;
  }
  return true;
}


void appendToList(String list_name, String file_name){
  char fn_arr[ARRAY_CHARS];
  File list_file;
  
  filename = list_name;
  filename.toCharArray(fn_arr, filename.length() + 1);
  list_file = SD.open(fn_arr, FILE_WRITE);

  list_file.println(file_name);

  list_file.close();
}

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
  }

  list_file.close();
  return "-1";
}




// eof
