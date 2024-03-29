
void serialEvent() {
  // Process any incoming characters from the serial port  
  while (Serial.available() > 0) {
    char c = Serial.read();
    // Add any characters that aren't the end of a command (semicolon) to the input buffer.
    if (c != ';') {
      c = toupper(c);
      strncat(commandBuffer, &c, 1);
    }
    else
    {
      // Parse the command because an end of command token was encountered.
      Serial.println(commandBuffer);
      parseCommand(commandBuffer);


      // Clear the input buffer
      memset(commandBuffer, 0, sizeof(commandBuffer));
    }
  }
}

#define GET_AND_SET(variableName) \
  if (strstr(command, "GET " #variableName) != NULL) { \
    Serial.print(#variableName" "); \
    Serial.println(variableName); \
  } \
  else if (strstr(command, "SET " #variableName " ") != NULL) { \
    variableName = (typeof(variableName)) atof(command+(sizeof("SET " #variableName " ")-1)); \
    Serial.print(#variableName" "); \
    Serial.println(variableName); \
  }

#define GET_NOT_SET(variableName) \
  if (strstr(command, "GET " #variableName) != NULL) { \
    Serial.print(#variableName" "); \
    Serial.println(variableName); \
  }

void GET_MAG(char* command) {
  if (strstr(command, "GET MAGS") != NULL) {
    Serial.print("MAG ");
    export_normalized_mags(); //export_mags();
  }
}


void GET_AND_SET_TEMPLATE(char* command) {

  if (strstr(command, "GET TEMPLATE") != NULL) {
    Serial.print("TEMPLATE ");
    export_template(templt);
  }
  else if (strstr(command, "SET TEMPLATE") != NULL) {
    import_template(command);
  }
}



void GET_AND_SET_PRETEMPLATE(char* command) {

  if (strstr(command, "GET PRETEMPLATE") != NULL) {
    Serial.print("PRETEMPLATE ");
    export_template(templtpre);
  }
  else if (strstr(command, "SET PRETEMPLATE") != NULL) {
    import_templatepre(command);
  }
}


void START_AND_STOP(char* command) {
  if (strstr(command, "STOP") != NULL) {
    samplingStop();      
    isRunning=false;
    Serial.println("Stopped");
  }
  else if (strstr(command, "START") != NULL) {
    samplingBegin();
    isRunning=true;
    delay(250); ///////////wait to fill the buffer....
    Serial.println("Started...");
  }
}



void parseCommand(char* command) {
  
  GET_AND_SET(FF_MAX);
  GET_AND_SET(FF_MIN);
  GET_AND_SET(FREQTHRESH);
  GET_AND_SET(DPTHRESH);
  GET_AND_SET(TESTPRETEMPLATE);
  GET_AND_SET(PREDPTHRESH);
  GET_AND_SET(TIME_BETWEEN_TEMPLATES);
  GET_AND_SET(AVERAGING);
  GET_AND_SET(AMP_THRESHOLD);
  GET_AND_SET(CATCHPERCENT);
  GET_AND_SET(FREQDIR);
  GET_AND_SET(MINDELAY);
  GET_AND_SET(MAXDELAY);
  GET_AND_SET(PLOTMAGS);
  GET_AND_SET(AMP_MAX);
  GET_AND_SET(PLAYWN);
  GET_AND_SET(AMP_MAX);
  GET_AND_SET(LINE_IN_LEVEL);
  GET_AND_SET(DUR_MATCH_TEMPLATE)
  GET_AND_SET(PRE_DUR_MATCH_TEMPLATE);
  GET_AND_SET(AMPLITUDE_THRESHOLD_DURATION);  
  GET_AND_SET(SAMPLE_RATE_HZ)
  GET_AND_SET(AVERAGING)
  GET_NOT_SET(FFT_SIZE)  
  GET_NOT_SET(RUNNING_AMP);   
  GET_NOT_SET(AUDIO_BLOCK_SAMPLES);
  
  GET_AND_SET_TEMPLATE(command);
  GET_AND_SET_PRETEMPLATE(command);  
  GET_MAG(command);
  START_AND_STOP(command);

  // Update Max/Min Range for FFTs and/or Sampling Rates.
  float FFT_Bin = ((float(SAMPLE_RATE_HZ)/ 2) / (float(FFT_SIZE) / 2)); // Correct for changed sampling rate now. Cast to float for (tiny bit) better precision. 
  FF_maxIDX = (int)ceil((FF_MAX / FFT_Bin))+1; //float to int! ////////// Indexes for finding peak frequency after template math. Offset for Parabolic interpolation. 
  FF_minIDX = (int)floor((FF_MIN / FFT_Bin))-1;                  /////// Lower bound for detection. -1 because of offset issues, make sure we include that peak + 1 bin for interpolation. 
  audioShield.micGain(LINE_IN_LEVEL);  

}

void export_template(float CurrTemplt[]) {

 Serial.print(CurrTemplt[0]);
  for (int i = 1; i <= TEMPLT_SIZE - 1; i++) {
    Serial.print(',');
    Serial.print(CurrTemplt[i]);
  }
  Serial.println();
}

void import_template(char* input) {

  float NewTemplate[TEMPLT_SIZE];
  String crap = input;

  crap.remove(0, 14); // strip out instructions;

  int CharNum = int(crap.length());
  
  int FFT_Counter = 0;


  // SET TEMPLATE 24442,27174,3810,27401,18971,2926,8355,16406,28725,28947,4728,29118,28715,14561,24008,4257,12653,27472,23766,28785,19672,1071,25474,28020,20362,22732,22294,11767,19664,5136,21181,955,8308,1385,2914,24704,20845,9513,28507,1033,13162,11447,22966,23856,5606,14693,13368,19389,21281,22641,8281,20391,19653,4878,3570,14951,28792,10212,17558,6714,22538,7653,15179,20972,26727,28779,16416,4159,4479,7725,25222,7628,24429,7306,27878,10500,5898,7533,18481,14199,10550,24925,17558,16492,27516,8575,22716,22612,11413,17035,2276,1619,15924,23375,28020,3897,17065,14082,357,10114,4865,23829,9336,15856,4969,18059,7889,19622,20676,22445,13516,2515,6869,27400,4571,24775,16150,29884,2345,13280,3200,28857,139,23247,24519,26061,2533,11993,7796,24002,12942,27319,5455,7914,4366,4082,26079,17391,16496,4349,25591,18662,10529,15397,12054,2279,7197,3700,5517,7199,12518,1490,27081,28344,14726,14678,10132,27002,11077,3336,23408,11692,7251,12117,2894,3959,28262,28684,17256,1793,7043,10595,24636,462,1291,5070,19473,21952,19432,13528,16410,8890,22341,5669,20603,5505,11055,18769,23407,2434,27882,23271,14604,13076,13404,9190,15255,15323,24529,23845,19330,11358,24347,15985,10522,28170,26278,16505,18674,17611,6232,9037,14128,6915,25329,5843,6778,5121,6830,13071,9333,27701,12906,5544,27146,29392,13166,3334,7742,12262,17847,7866,18085,21336,6652,3523,8900,9563,12725,15236,2565,7874,24030,877,27866,21910,14658,17356,7119,13765,28893,16404,15634,6948,14667,18722,20374,11865,11023,29639,1132,26555,27399,23886,2961,7856,10061,20392,4097,21637,3203,19613,14825,23372,21451,27112,26728,10025;SET TEMPLATE 24442,27174,3810,27401,18971,2926,8355,16406,28725,28947,4728,29118,28715,14561,24008,4257,12653,27472,23766,28785,19672,1071,25474,28020,20362,22732,22294,11767,19664,5136,21181,955,8308,1385,2914,24704,20845,9513,28507,1033,13162,11447,22966,23856,5606,14693,13368,19389,21281,22641,8281,20391,19653,4878,3570,14951,28792,10212,17558,6714,22538,7653,15179,20972,26727,28779,16416,4159,4479,7725,25222,7628,24429,7306,27878,10500,5898,7533,18481,14199,10550,24925,17558,16492,27516,8575,22716,22612,11413,17035,2276,1619,15924,23375,28020,3897,17065,14082,357,10114,4865,23829,9336,15856,4969,18059,7889,19622,20676,22445,13516,2515,6869,27400,4571,24775,16150,29884,2345,13280,3200,28857,139,23247,24519,26061,2533,11993,7796,24002,12942,27319,5455,7914,4366,4082,26079,17391,16496,4349,25591,18662,10529,15397,12054,2279,7197,3700,5517,7199,12518,1490,27081,28344,14726,14678,10132,27002,11077,3336,23408,11692,7251,12117,2894,3959,28262,28684,17256,1793,7043,10595,24636,462,1291,5070,19473,21952,19432,13528,16410,8890,22341,5669,20603,5505,11055,18769,23407,2434,27882,23271,14604,13076,13404,9190,15255,15323,24529,23845,19330,11358,24347,15985,10522,28170,26278,16505,18674,17611,6232,9037,14128,6915,25329,5843,6778,5121,6830,13071,9333,27701,12906,5544,27146,29392,13166,3334,7742,12262,17847,7866,18085,21336,6652,3523,8900,9563,12725,15236,2565,7874,24030,877,27866,21910,14658,17356,7119,13765,28893,16404,15634,6948,14667,18722,20374,11865,11023,29639,1132,26555,27399,23886,2961,7856,10061,20392,4097,21637,3203,19613,14825,23372,21451,27112,26728,10025,20962,5934,916,22322,15001,14398,27142,18296,18530,25783,24165,17302,5488,7198,26595,860,14697,5038,29360,21381,15014;
  String floatString = String("");

  for (int i = 0; i < CharNum + 1; i++) { //for however many characters sent

    if (crap[i] == ',') {  //check for comma delimiter
    
      NewTemplate[FFT_Counter] = float(floatString.toFloat());
      floatString = (""); //reset!
      FFT_Counter += 1;
    }
    if (crap[i] != ',') { //otherwise add character to string.
      floatString = (floatString + crap[i]); // append!
    }


  }
  NewTemplate[FFT_Counter] = floatString.toFloat();
  //NewTemplate[0]=0; // Never count 0Hz info. 0Hz is always bad. 

  if (FFT_Counter == TEMPLT_SIZE-1) {
    memcpy(templt, NewTemplate, sizeof(templt));
    Serial.println("Copied");
    //export_template();
  }
    else if (FFT_Counter != TEMPLT_SIZE) {
    Serial.println("Bad Template");
  }
  
}

void import_templatepre(char* input) {

  float NewTemplate[TEMPLT_SIZE];
  String crap = input;

  crap.remove(0, 14); // strip out instructions;

  int CharNum = int(crap.length());
  
  int FFT_Counter = 0;


  // SET TEMPLATE 24442,27174,3810,27401,18971,2926,8355,16406,28725,28947,4728,29118,28715,14561,24008,4257,12653,27472,23766,28785,19672,1071,25474,28020,20362,22732,22294,11767,19664,5136,21181,955,8308,1385,2914,24704,20845,9513,28507,1033,13162,11447,22966,23856,5606,14693,13368,19389,21281,22641,8281,20391,19653,4878,3570,14951,28792,10212,17558,6714,22538,7653,15179,20972,26727,28779,16416,4159,4479,7725,25222,7628,24429,7306,27878,10500,5898,7533,18481,14199,10550,24925,17558,16492,27516,8575,22716,22612,11413,17035,2276,1619,15924,23375,28020,3897,17065,14082,357,10114,4865,23829,9336,15856,4969,18059,7889,19622,20676,22445,13516,2515,6869,27400,4571,24775,16150,29884,2345,13280,3200,28857,139,23247,24519,26061,2533,11993,7796,24002,12942,27319,5455,7914,4366,4082,26079,17391,16496,4349,25591,18662,10529,15397,12054,2279,7197,3700,5517,7199,12518,1490,27081,28344,14726,14678,10132,27002,11077,3336,23408,11692,7251,12117,2894,3959,28262,28684,17256,1793,7043,10595,24636,462,1291,5070,19473,21952,19432,13528,16410,8890,22341,5669,20603,5505,11055,18769,23407,2434,27882,23271,14604,13076,13404,9190,15255,15323,24529,23845,19330,11358,24347,15985,10522,28170,26278,16505,18674,17611,6232,9037,14128,6915,25329,5843,6778,5121,6830,13071,9333,27701,12906,5544,27146,29392,13166,3334,7742,12262,17847,7866,18085,21336,6652,3523,8900,9563,12725,15236,2565,7874,24030,877,27866,21910,14658,17356,7119,13765,28893,16404,15634,6948,14667,18722,20374,11865,11023,29639,1132,26555,27399,23886,2961,7856,10061,20392,4097,21637,3203,19613,14825,23372,21451,27112,26728,10025;SET TEMPLATE 24442,27174,3810,27401,18971,2926,8355,16406,28725,28947,4728,29118,28715,14561,24008,4257,12653,27472,23766,28785,19672,1071,25474,28020,20362,22732,22294,11767,19664,5136,21181,955,8308,1385,2914,24704,20845,9513,28507,1033,13162,11447,22966,23856,5606,14693,13368,19389,21281,22641,8281,20391,19653,4878,3570,14951,28792,10212,17558,6714,22538,7653,15179,20972,26727,28779,16416,4159,4479,7725,25222,7628,24429,7306,27878,10500,5898,7533,18481,14199,10550,24925,17558,16492,27516,8575,22716,22612,11413,17035,2276,1619,15924,23375,28020,3897,17065,14082,357,10114,4865,23829,9336,15856,4969,18059,7889,19622,20676,22445,13516,2515,6869,27400,4571,24775,16150,29884,2345,13280,3200,28857,139,23247,24519,26061,2533,11993,7796,24002,12942,27319,5455,7914,4366,4082,26079,17391,16496,4349,25591,18662,10529,15397,12054,2279,7197,3700,5517,7199,12518,1490,27081,28344,14726,14678,10132,27002,11077,3336,23408,11692,7251,12117,2894,3959,28262,28684,17256,1793,7043,10595,24636,462,1291,5070,19473,21952,19432,13528,16410,8890,22341,5669,20603,5505,11055,18769,23407,2434,27882,23271,14604,13076,13404,9190,15255,15323,24529,23845,19330,11358,24347,15985,10522,28170,26278,16505,18674,17611,6232,9037,14128,6915,25329,5843,6778,5121,6830,13071,9333,27701,12906,5544,27146,29392,13166,3334,7742,12262,17847,7866,18085,21336,6652,3523,8900,9563,12725,15236,2565,7874,24030,877,27866,21910,14658,17356,7119,13765,28893,16404,15634,6948,14667,18722,20374,11865,11023,29639,1132,26555,27399,23886,2961,7856,10061,20392,4097,21637,3203,19613,14825,23372,21451,27112,26728,10025,20962,5934,916,22322,15001,14398,27142,18296,18530,25783,24165,17302,5488,7198,26595,860,14697,5038,29360,21381,15014;
  String floatString = String("");

  for (int i = 0; i < CharNum + 1; i++) { //for however many characters sent

    if (crap[i] == ',') {  //check for comma delimiter
    
      NewTemplate[FFT_Counter] = float(floatString.toFloat());
      floatString = (""); //reset!
      FFT_Counter += 1;
    }
    if (crap[i] != ',') { //otherwise add character to string.
      floatString = (floatString + crap[i]); // append!
    }


  }
  NewTemplate[FFT_Counter] = floatString.toFloat();
  //NewTemplate[0]=; // Never count 0Hz info. 0Hz is always bad. 

  if (FFT_Counter == TEMPLT_SIZE-1) {
    memcpy(templtpre, NewTemplate, sizeof(templt));
    Serial.println("Copied");
    //export_template();
  }
    else if (FFT_Counter != TEMPLT_SIZE) {
    Serial.println("Bad Template");
  }
  
}
//////////////////////////////////////////////////////////////////////////////////
//// UTILITY FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////
void export_mags() {

      int16_t scalefactor;
      uint32_t peakmagidx;

      arm_max_q15(magnitudes,FFT_SIZE,&scalefactor,&peakmagidx);//find the max; 
      scalefactor=1/scalefactor; // for vector multiplation below 
      
      Serial.print(magnitudes[0]);                    
            
      for (int i=1; i<=TEMPLT_SIZE; i++) {

            Serial.print(',');
            Serial.print(magnitudes[i]); 
       }

   Serial.println();    
     
}


void export_normalized_mags() {
  
  float32_t floatmags[TEMPLT_SIZE]; 
  float32_t scaledmags[TEMPLT_SIZE];    
  float32_t scalefactor;
  uint32_t peakmagidx;
  float32_t dotprod;

  for (int i=0; i<TEMPLT_SIZE; i++) {        // move to floating point so the template is normalized to 1. 
    floatmags[i]=(float)magnitudes[i];
  }
  
  arm_max_f32(floatmags,TEMPLT_SIZE,&scalefactor,&peakmagidx);//find the max; 
  scalefactor=1/scalefactor; // for vector multiplation below 
  arm_scale_f32(floatmags, scalefactor, scaledmags, TEMPLT_SIZE); // and normalize.
  
  if (FFT_SIZE==256){ // Get rid of Low F for normalization later
      for (int i=0; i <3; i++) {
        scaledmags[i]=0; 
      }
        
    }
  if (FFT_SIZE==1024) { // Get rid of Low F for normalization later
      for (int i=0; i <10; i++) {
        scaledmags[i]=0; 
      }
    }  

  
  arm_dot_prod_f32(scaledmags, templt, TEMPLT_SIZE, &dotprod); // and calculate the dot product
  Serial.print(scaledmags[0]);       
      
      for (int i=1; i<=TEMPLT_SIZE-1; i++) {            
            Serial.print(',');
            Serial.print(scaledmags[i]); 
       }
       
       Serial.println();   
  }
