//Main Loop  should run every ~0.7ms for 32 samples updating a 256pt FFT. 

void loop() {

  //float timea = micros();

   if (isRunning) {

  // Get samples and calculate RMS. ~
  //Serial.println(SamplesAvailable);
    if (SamplesAvailable) {    
      getsamples(); // not perfect, but I can't point to a volatile float, and I don't want to make the samples non-volatile.
      arm_rms_q15(fftbuffer, FFT_SIZE*2, &RUNNING_AMP); //RMS of the buffer. Since there's a 'complex' component of all zeros, it still works fine.     
    
    
        
        if (RUNNING_AMP > AMP_THRESHOLD) { 
          AboveThresh++;      
        }
        else {
          AboveThresh = 0;    
          fft_threshcounter_pre = 0; // reset the FFT_threshold counter.
          fft_threshcounter = 0;
        }
    
        if (RUNNING_AMP > AMP_MAX){ // implement max amplitude. 
          //AboveThresh = 0;        
        }

    if (AboveThresh > AMPLITUDE_THRESHOLD_DURATION) {   // e.g. has there been sound? 
    
           if (PLOTMAGS==true) {
             CalculateFFT(); 
             export_normalized_mags();   
           }

           if (PLOTMAGS==false) {
            
             CalculateFFT(); // Galculate the FFT, results are cast to the magnitudes variable.                               
        
             if ((TESTPRETEMPLATE==true) && (delaytime > MAXDELAY)) { // compare to PRE template;
                dp = ScaleAndCompareToTemplateMSE(templtpre);    
              
                if ((dp <= PREDPTHRESH)) { //// Are we above the template match threshold?
                      fft_threshcounter_pre++; // if we are, increment the counter so we can decide how many sequenctial template matches we went before we trigger.            
                }
                else {
                      fft_threshcounter_pre = 0; // reset the FFT_threshold counter.
                }              
               if (fft_threshcounter_pre >= PRE_DUR_MATCH_TEMPLATE) { // Did we find the pre-template?
                     delaytime=0;   // reset our timer in milliseconds
                     Serial.print("PRE:");
                     Serial.print(dp);
                     Serial.print(",");
                     export_normalized_mags();   
                     fft_threshcounter_pre=0; // restart FFT counter for the other process                                              
               }
              }
              
            if ((TESTPRETEMPLATE==false) || ((delaytime > MINDELAY) && (delaytime < MAXDELAY))) { // if one template, then go, if two, wait for the between times from the pre-match
                dp = ScaleAndCompareToTemplate(templt);   
                if ((dp >= DPTHRESH)) { //// Are we above the template match threshold?
                      fft_threshcounter++; // if we are, increment the counter so we can decide how many sequenctial template matches we went before we trigger.            
                    }
                else {
                      fft_threshcounter = 0; // reset the FFT_threshold counter.
                     }                         
  
                if (fft_threshcounter >= DUR_MATCH_TEMPLATE) { ////////// How long has the template match been? If it's been enough samples above the FFT match threshold, then trigger        
                     fft_threshcounter_pre=0;
                     FF = CalculateFF(AVERAGING);                                           
                     delaytime=millis()+MAXDELAY; // Wait for Pre again.
                if ((FF < FF_MAX) && (FF > FF_MIN)) { // If the result isn't gibberish (e.g. as long as the FF returned is inside of the window it should be in, the peak isn't the first or last or anything stupid. 
          
                  if ((FREQDIR==0) && (FF<FREQTHRESH)) { /////// Are we below the Frequency Trials? Upshifts... 
                          Serial.println("Below Hit");
                          play_wn();
                          delay(50);         // optional for slow syllables that you don't want to hit twice.
                  } // 
                  else if ((FREQDIR==1) && (FF>FREQTHRESH)) { /////// Are we above the Frequency Trials forDownshifts...
                          Serial.println("Above Hit");
                          play_wn();
                          delay(50);         // optional for slow syllables that you don't want to hit twice.
                  } // 
                  else { // good trial;
                         HIT=3;
                         Serial.println("Escaped!");
                         play_wn();
                         delay(50);
                 } // end of HIT directions.
                } // end of sanity check for peak FF.
              } //end of template match triggered portions
           }// End of Real Template portion
         } // end of PlotMags portion
        } // end of amplitude triggered portion
  
    } //end samples available
  } // end is running

} // end Main Loop  
