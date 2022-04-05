
////////////////////////////////////////////////////////////////////////////////
// FFT/SAMPLING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////


void play_wn() {
  
  PlayBackCounter = 0;
  digitalWrite(POWER_LED_PIN,HIGH);

  if (PLAYWN==0) { // If WN is off, code everything as a catch trial. 
    HIT=0;
  }
  
  if (HIT==0) {    
    Serial.println("Catch!");     
    delay(20); //20ms delay between WN and trigOUt timing due to audio latency issues
    analogWrite(9, 255); 
    delay(25);  
    analogWrite(9, 0);    
    delay(25);    
  }
  
  if (HIT==1) {    
    Serial.println("Hit!");    
    WNSynth.amplitude(1.0);    
    delay(20);    //20ms delay between WN and trigOUt timing due to audio latency issues
    analogWrite(9, 255);    
    delay(30);
    WNSynth.amplitude(0);          
    
    delay(20);
    analogWrite(9, 0);
    
  }
  
  if (HIT==3) { 
    Serial.println("Escaped!");
    delay(20); //20ms delay between WN and trigOUt timing due to audio latency issues
    analogWrite(9, 255);
    delay(50); 
    analogWrite(9, 0);
 }
 
 /// Send everything else out!
 
  digitalWrite(POWER_LED_PIN, LOW);        // And power everything else down too.           
  
  Serial.print("FF ");
  Serial.print(FF);
  Serial.print(','); // print FF to serial so it can be written out. 
  Serial.print(dp);
  Serial.print(',');  
  Serial.print(HIT);         // Output Catch (0) or Hit (1) trial.
  Serial.print(',');  
  Serial.println(RUNNING_AMP);
  
  export_normalized_mags();
    
  ThreshRand = random(1, 100); // choose a random variable

  if (ThreshRand > CATCHPERCENT) { //Decide if the next trial will be a hit, or a catch. PERCENTHITs is actually Catch Fraction. 
    HIT = 1;
  }
  else {
    HIT = 0;
  }

}

//////////////////
// Calculated Peak FF index
//////////////////

// Calcutes the FFT, in this case 32bit floating point (given 12-14 bit data, possibly overkill, but int16 is definately a smoother FFT, especially once normalized, and it's nice to have everything be floats for convenience. 
void CalculateFFT() {
  
    arm_cfft_radix4_instance_q15 fft_inst; // create structure on ARM processor
    arm_cfft_radix4_init_q15(&fft_inst, FFT_SIZE, 0, 1); // initialize,
    HanningWindowFFTBuffer();    // 256 point Hanning window. 
    
    if (status == 0){
          arm_cfft_radix4_q15(&fft_inst,fftbuffer); // and calculate the FFT. - Calculated in place, fftbuffer is now the mangitudes
    }
   
    // Calculate magnitude of complex numbers output by the FFT.
    //arm_cmplx_mag_squared_q15(fftbuffer, magnitudes, FFT_SIZE); // and get them back out. Used for template matches. raw fftbuffer used for FF calc 
    //arm_shift_q15(magnitudes,8, magnitudes, FFT_SIZE);  // bit shift to correct for what happened during the FFT calculation 
    
    int32_Mags();    // calculate the magnitudes, no longer using the ARM for this in order to get higher resolution.  
  //}
  
}

float ScaleAndCompareToTemplate(float CurrTemplt[]) {

  float32_t floatmags[TEMPLT_SIZE]; 
  float32_t scaledmags[TEMPLT_SIZE];   
  float32_t scalefactor;
  u_int32_t peakmagidx;
  float dotprod;
  
  arm_q15_to_float(magnitudes,floatmags,TEMPLT_SIZE);
 // for (int i=0; i < TEMPLT_SIZE; i++) {        // move to floating point so the template is normalized to 1. 
  //  floatmags[i]=(float)magnitudes[i];
 // }
  arm_max_f32(floatmags,TEMPLT_SIZE,&scalefactor,&peakmagidx);//find the max; 
  scalefactor=1/scalefactor; // for vector multiplation below 
  arm_scale_f32(floatmags, scalefactor, scaledmags, TEMPLT_SIZE); // and normalize.  

  scaledmags[0]=0; // Get rid of the first few 100-300 Hz depding n samp freq
  scaledmags[1]=0;
  scaledmags[2]=0;
  
  arm_dot_prod_f32(scaledmags, CurrTemplt, TEMPLT_SIZE, &dotprod); // and calculate the dot product 

  return dotprod;
    
}

float ScaleAndCompareToTemplateMSE(float CurrTemplt[]) {

  float32_t floatmags[TEMPLT_SIZE]; 
  float32_t scaledmags[TEMPLT_SIZE];
  float32_t submags[TEMPLT_SIZE];   
  float32_t scalefactor;
  u_int32_t peakmagidx;
//  float dotprod;
  float MSE;
  
  for (int i=0; i < TEMPLT_SIZE; i++) {        // move to floating point so the template is normalized to 1. 
    floatmags[i]=(float)magnitudes[i];
  }
  //arm_q15_to_float(magnitudes,floatmags,TEMPLT_SIZE);
  arm_max_f32(floatmags,TEMPLT_SIZE,&scalefactor,&peakmagidx);//find the max; 
  scalefactor=1/scalefactor; // for vector multiplation below 
  arm_scale_f32(floatmags, scalefactor, scaledmags, TEMPLT_SIZE); // and normalize.  

  scaledmags[0]=0; // Get rid of the first few 100-300 Hz depding n samp freq
  scaledmags[1]=0;
  scaledmags[2]=0;

  arm_sub_f32(scaledmags, CurrTemplt, submags, TEMPLT_SIZE); //Subtract
  arm_power_f32(submags, TEMPLT_SIZE, &MSE); // sum of squares
//  arm_sqrt_f32 (dotprod, &MSE); // and square root for MSE;
  

  return MSE;
    
}


////////////////////////////////////////////////////////////////////////////////
// SAMPLING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
  
// Get the last FFT_SIZE samples, and format them for the FFT. 
void getsamples() {
  
  int currentidx = sampleCounter - (FFT_SIZE - 1); // Go back FFT_Size from where we are now. (array starts at 0, so 0-1023 for 1024 samples).
    
//Full Samples
  for (int i = 0; i <= ((FFT_SIZE*2)-1); i = i + 2) { // (2/DOWNSAMPLE)) { // go through, get the last FFT_Size samples, and put them into a structure for ARM FFT calculation. Go by 4 down downsample by half. (FFT_SIZE*2-1) give 255, and gets rid of a possible integer overflow loop warning, but doesn't matter. 
    fftbuffer[i] = samples[currentidx + (i/2)]; //Samples in real idx, FFT interleaved with 0s, downsampled;
    fftbuffer[(i/2) + 1] = 0; // Sampled in place, so reset every time.
  } 
  SamplesAvailable=false;
  
}

  
// This is called at the sampling frequency in order to continously sample, and write to a buffer.
// for whatever reason, I couldn't get a simple circular buffer to work- but this seems to be stable and functional. 
// might have been using memcpy() rather than memmove to deal w/ circ buffer- would explain the occasioanl wrong samples. 

void samplingCallback() {
 
  // Read from the ADC and store the sample data   
  while (QueueIn.available() >=1) {  /////////Check the Audio Queue This will keep blocks of audio during playback- long (e.g. syllable) playbacks should not exceed the AudioMemory.        

    
        memcpy(&audio, QueueIn.readBuffer(), AUDIO_BLOCK_SAMPLES*2); // move samples into audio[], set to 16 to up the latency. 
        QueueIn.freeBuffer(); // clear buffer;   
        memcpy(QueueOutSound.getBuffer(), &audio, AUDIO_BLOCK_SAMPLES*2); // send out to USB      
        QueueOutSound.playBuffer();          
          
    if (SAMPLE_RATE_HZ==44100) { //use every sample
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {                          
          samples[sampleCounter] = audio[i]; // May need subtraction here, depending on whether the filter on the audio shield is disabled- if the highpass is one, it will be zero centered, but might have noise at high frequencies.      
       //   Serial.println(samples[sampleCounter]);
          sampleCounter++;          
        }        

    }
    
    if (SAMPLE_RATE_HZ==22050) { //Take the average of every 2 at 44.1 KHz.        
        for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i=i+2) {    // Get Downsampled Audio.                       
          samples[(sampleCounter+(i/2))] = ((audio[i]/2) + (audio[(i+1)]/2)); //May need subtraction here, depending on whether the filter on the audio shield is disabled- if the highpass is on, it will be zero centered, but might have noise at high frequencies.                         
          //Serial.println(samples[(sampleCounter+(i/2))]);
        } 
        sampleCounter=sampleCounter+(AUDIO_BLOCK_SAMPLES/2);                 
   // }
      
    }
      
  if (sampleCounter >= (BUFFER_SIZE-FFT_SIZE)) {    /// Is the buffer so full ?           
    for (int i = 0; i < (FFT_SIZE); i++) { /// Rewrite the first however many for continuity. This seems to work just fine, as long at it's fast.
       samples[i] = samples[i + ((BUFFER_SIZE)-(FFT_SIZE*2))]; /// move the end to the start, and continue writing from there.                        
    }
    sampleCounter = (FFT_SIZE); // set samplecounter to the end of the moved parts.       
  }
  
  SamplesAvailable=true; // There are new samples available;
  
 } // end QueueIn.available
} // end samplingCallback;

// Starts the timer. If you run things that conflict with this interrupt, you'll crash the processor.
void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  delaytime=0;   // reset the timer in case system has been on forever
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000 / REFRESH_RATE_HZ); ///// Probably not exactly necessary anymore. But Whatevs. 
}

// pretty self-evident.
void samplingStop() {
  samplingTimer.end();
}


void HanningWindowFFTBuffer() {
  
  int16_t *buf = (int16_t *)fftbuffer;
  const int16_t *win = (int16_t *)AudioWindowHanning256;;

  for (int i=0; i < 256; i=i+1) { // Every second one is true.... 
    int32_t val = *buf * *win++;
    *buf = val >> 15;
    buf += 2;
  }
}

/* INTERPOLATIN FUCNTIONS
// Interpolation modified: From:
//https://forum.pjrc.com/threads/36358-A-New-Accurate-FFT-Interpolator-for-Frequency-Estimation
// by 
// apparentrly similar to Jain, 1979. 
// Estimate off-center bing FFs by the ratio of the peak to the larger shoulder. 
*/

float FFTinterpHann256() {

    float ratio;
    float freq;
    float idx_corr;
    int16_t idx_max=0;
    float magpeak=0;

    float peakMag;


    idx_corr=0;          
    for (int i=FF_minIDX; i<FF_maxIDX; i++) {
          if (magnitudes[i]>magpeak) {
              idx_max=i;
              magpeak=float(magnitudes[i]);            
          }          
    }
    
   if (magnitudes[idx_max-1] == magnitudes[idx_max+1]) {
         idx_corr = peakMag;
   }
   
   if (magnitudes[idx_max-1] > magnitudes[idx_max+1]) {
          ratio=(float)((sqrt(magpeak))/(sqrt(magnitudes[idx_max-1])));
          //ratio=(peakMag)/PrePeak;
          idx_corr = (float)(idx_max - (2.0 - ratio)/(1.0+ratio));
   
   }
   if (magnitudes[idx_max+1] > magnitudes[idx_max-1]) {
          ratio=(float)((sqrt(magpeak))/(sqrt(magnitudes[idx_max+1])));    
          idx_corr = (float)(idx_max + (2.0 - ratio)/(1.0+ratio));
   }
  
   freq = (idx_corr)*FFT_Bin;

   return freq;   
}



// Parabolic Interpolation for FF improvement.
  
float PinterP() {

  float InterP_FF;
  float interpidx;
  float magpeak=0;
  int idx_max;

  idx_max=0; 
  for (int i=FF_minIDX; i<FF_maxIDX; i++) {
          if (magnitudes[i]>magpeak) {  
              idx_max=i;
              magpeak=magnitudes[i];            
          }          
    }

  float a=  (float)sqrt(magnitudes[idx_max-1]);        
  float b=  (float)sqrt(magnitudes[idx_max]);        
  float c= (float)sqrt(magnitudes[idx_max+1]);        

  interpidx =(float)(0.5 * (((a - c) / (a - (2 * b) + c))));
  
  //idx = idx + 1;
  InterP_FF = ((idx_max) + interpidx) * (FFT_Bin);

  return InterP_FF;

}


float CalculateFF(int Average) {
  
      float FF[Average];  
      float FFPar;     
      float FFMean;

      FF[0]=PinterP();

      if (Average>1) {            
        for (int i=1; i<=Average-1; i++) {                            

            while (SamplesAvailable!=true) {              //If averging, include the next 16Ch block. 
              delayMicroseconds(15);                   
            }
            getsamples();  
            CalculateFFT();  
            dp = ScaleAndCompareToTemplate(templt);                 
            
            if (dp > DPTHRESH) { //make sure we still match?
                FFPar=PinterP();
                FF[i]=FFPar;
            }
            else {
                i=Average; // break the loop and fill in with the last good estimate; Mags will be bad tho.
                  for (int j=i; j<Average-1; j++) {
                     FF[i]=FFPar;  // Fill in with last good value. 
                  }
               }
            }

      }              

   //   arm_std_f32(FF,Average,&FFstd); 
      arm_mean_f32(FF,Average,&FFMean);

//             
     return FFMean;
}

////// Calculate Magnitudes, since the ARM is faster, but loses at least one bit of precision. 

void int32_Mags() {

  //  int imax = 0;
    
    for (int i=0; i < 128; i++) {
      uint32_t tmp = *((uint32_t *)fftbuffer + i);        // real & imag
      uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
      magnitudes[i]=sqrt_uint32(magsq);
    }

}
