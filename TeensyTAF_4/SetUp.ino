
////////////////////////////////////////////////////////////////////////////////
// MAIN SKETCH FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Set up serial port

  Serial.begin(9600); 

  //output LEDs for playback observations.
   pinMode(POWER_LED_PIN, OUTPUT);
   //pinMode(9, OUTPUT);
   digitalWrite(POWER_LED_PIN, HIGH);
   analogWrite(9, 0);
   

  /////////// Audio Shield Setup
   AudioMemory(60);      //allocate Int16 audio data blocks
   
   Serial.println("AudioBlock Samples");  
   Serial.print("is: ");
   Serial.println(AUDIO_BLOCK_SAMPLES);  

   while (AUDIO_BLOCK_SAMPLES>16) {
    //  Serial.println("AudioBlock Samples Incorrect");  
   }
      
  // Enable the audio shield, select input, and enable output
  audioShield.enable();                   //start the audio board
  audioShield.inputSelect(AUDIO_INPUT_MIC);       //choose line-in or mic-in  
  audioShield.micGain(LINE_IN_LEVEL);  
  audioShield.lineOutLevel(15, 15);        //level can be 13+ (13 biggest). 
  FilterHP.frequency(120); //fixed 100Hz HP filter. 
  FilterHPOut.frequency(250);
  FilterLP.frequency(12000);
  //audioShield.adcHighPassFilterDisable(); //reduces noise?  https://forum.pjrc.com/threads/27215-24-bit-audio-boards?p=78831&viewfull=1#post78831
  delaytime=0;
  QueueIn.begin();
  WNSynth.amplitude(0);


  FF_maxIDX = (int)ceil((FF_MAX / FFT_Bin))+1; //float to int! ////////// Indexes for finding peak frequency after template math. Offset for Parabolic interpolation. 
  FF_minIDX = (int)floor((FF_MIN / FFT_Bin));                  /////// Lower bound for detection. -1 because of offset issues, make sure we include that peak + 1 bin for interpolation. 
  
  // Begin sampling audio
  samplingBegin();
  isRunning=true;
  delay(250); ///////////wait to fill the buffer....

}
