
// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel). Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.

Dither convert;

void bmpLoad(char *filename, int x, int y, uint8_t* im_buffer) {

  File     bmpFile;
  int16_t  bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint32_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  uint16_t w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;
  uint32_t imgidx = 0;

  if((x >= ep_width) || (y >= ep_height)){
    return;
  }

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');
  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL){
    Serial.println(F("File not found"));
    return;
  }
  

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
     uint32_t fsize = read32(bmpFile);
    Serial.print(F("File size: "));
    Serial.println(fsize);
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    //Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    uint16_t head_sz = read32(bmpFile);
    Serial.print(F("Header size: ")); Serial.println(head_sz);
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    noteBmpSize(bmpWidth, bmpHeight);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)){ // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= ep_width){
          w = ep_width  - x;
        }
        if((y+h-1) >= ep_height){
          h = ep_height - y;
        }

        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          
          if(flip){ // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          }
          else{     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          }
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++){ // For each column...
            // Time to read more pixel data?
            
            if (buffidx >= sizeof(sdbuffer)){ // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to 8-bit array format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            im_buffer[imgidx++] = convert.color888ToGray256(r, g, b);
            
          } // end pixel
        } // end scanline
      } // end
    }
  }

  bmpFile.close();
  if(!goodBmp){
    Serial.println(F("BMP format not recognized."));
  }
}


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



// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}



void noteBmpSize(uint16_t w, uint16_t h){
  _BmpSize(w, h, true);
}

uint16_t readBmpWidth(){
  uint16_t w, h;
  _BmpSize(w, h, false);
  return w;
}

uint16_t readBmpHeight(){
  uint16_t w, h;
  _BmpSize(w, h, false);
  return h;
}

void _BmpSize(uint16_t &w, uint16_t &h, bool write){
  static uint16_t widt;
  static uint16_t heig;
  if(write){
    widt = w;
    heig = h;
  }
  else{
    w = widt;
    h = heig;
  }
}

// EOF
