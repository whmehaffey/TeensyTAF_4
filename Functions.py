from PyQt5 import QtGui, QtCore, QtWidgets

import pyaudio 
import wave
import audioop
from collections import deque
import os
import time
import math
import GlobalVars
import copy

CHUNK = 2048 # CHUNKS of bytes to read each time from mic
FORMAT = pyaudio.paInt16 #this is the standard wav data format (16bit little endian)
CHANNELS = 2# number of channels
RATE = 44100# sampling frequency
MIN_DUR=3 #minimum duration in seconds
MAX_DUR=30 #max dur in seconds

def GetSerialData(ui): #TAFLogfile,SaveFile):

    import sys
    import os
    import time 
    import serial
    import GlobalVars
    import numpy as np
    from numpy import histogram, array, arange 
    import pyqtgraph as pg
    import pdb

    while GlobalVars.ser.in_waiting > 0:
                
            line=GlobalVars.ser.readline(GlobalVars.ser.inWaiting()).decode('ascii');
            #print(line);
            line=line.strip('\n\r');            
            
            try:
                
                if (line[0:3]=="FF "):      #We've found the correct start
                    print(line)
                    magsline=GlobalVars.ser.readline(GlobalVars.ser.inWaiting()).decode('ascii');
                    magsline=magsline.strip('\n\r');
                    line=line.lstrip("FF ") #get rid of it
                    splitline=line.split(',')                    
                    GlobalVars.lastFFT= np.array(magsline.split(','), dtype=float)
                    
                    GlobalVars.PostTemplateCounter = GlobalVars.PostTemplateCounter +1;
                    GlobalVars.FF.append(float(splitline[0]))                    
                    GlobalVars.DP.append(float(splitline[1]))
                    GlobalVars.HIT.append(float(splitline[2]))
                    GlobalVars.RMS.append(float(splitline[3]))
                    #GlobalVars.PostTemplateCounter = GlobalVars.PostTemplateCounter +1;
                    
                    #Write to full Log file, and to individual song files.
                    date=time.localtime();
                    timestr=str(time.time())+','+str(date[0])+','+str(date[1])+','+str(date[2])+','+str(date[3])
                    paramsstr=str(GlobalVars.HitDIR)+','+str(GlobalVars.WN_ON)+','+str(GlobalVars.DirFlag)+','+str(GlobalVars.FreqTHRESH)+','+str(GlobalVars.CatchTrialPercent)
                    GlobalVars.FullExpOutputFileName.write(line +','+paramsstr+','+timestr+'\n');                    
                    GlobalVars.TrialExpOutputFileName.write(line +','+paramsstr+','+timestr+'\n');
                    GlobalVars.AllMagsFile.write(magsline+'\n');
                    GlobalVars.MagsOutFileName.write(magsline+'\n');
                    ui.MagsfromTeensy.clear()
                    ui.MagsfromTeensy.plot(GlobalVars.template,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000,pen=pg.mkPen('r', width=1))    
                    ui.MagsfromTeensy.plot(GlobalVars.lastFFT,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)
                    ui.DP_Val.setText("Match Value: "+str(GlobalVars.DP[-1]));
                    ui.FF_Val.setText("FF Estimate: "+str(GlobalVars.FF[-1]));

                    
                    ui.RMS_Val.setText("RMS: "+str(GlobalVars.RMS[-1]));                    

                    if ((len(GlobalVars.FF)>5)):
                                y,x=histogram(GlobalVars.FF,bins=25)
                                ui.FFMonitorPlot.clear()
                                ui.FFMonitorPlot.plot(x,y,stepMode=True,fillLevel=None, brush=(0,0,255,150))
                                ui.FFMonitorPlot.setXRange(GlobalVars.MinFF,GlobalVars.MaxFF,padding=0);
                                ui.FFMonitorPlotTrials.clear()
                                ui.FFMonitorPlotTrials.plot(arange(len(GlobalVars.FF)),GlobalVars.FF, symbol='o');
                                ui.FFMonitorPlot.addLine(x=GlobalVars.FreqTHRESH, y=None,pen=pg.mkPen('r', width=1))
                                ui.FFMonitorPlotTrials.addLine(x=None, y=GlobalVars.FreqTHRESH,pen=pg.mkPen('r', width=1))
                                GlobalVars.meanFF=sum(GlobalVars.FF)/len(GlobalVars.FF);   
                                ui.CurrentMeanFFLabel.setText("Current Avg FF: "+str(GlobalVars.meanFF));
                                
                                if GlobalVars.meanFF>0:
                                    ui.FFMonitorPlot.addLine(x=GlobalVars.meanFF, y=None) #, pen=mkPen('r', width=3))
                                    ui.FFMonitorPlotTrials.addLine(x=None, y=GlobalVars.meanFF) #,pen=mkPen('r', width=3))            
                                
                                y,x=histogram(GlobalVars.DP);
                                ui.DPHistGraph.clear()
                                ui.DPHistGraph.plot(x,y,stepMode=True,fillLevel=None, brush=(0,0,255,150))
                                ui.DPvsFFGraph.clear()
                                ui.DPvsFFGraph.plot(GlobalVars.DP,GlobalVars.FF,pen=None, symbol='o');
                                ui.RMSvsFFGraph.clear()
                                ui.RMSvsFFGraph.plot(GlobalVars.RMS,GlobalVars.FF,pen=None, symbol='o');

                    return True  # We got new info. 
                elif (line[0:4]=="PRE:"):                    
                    line=line.lstrip("PRE:") #get rid of it
                    lastFFT= np.array(line.split(','), dtype=float)
                    tempd=float(lastFFT[0])
                    PreMag=lastFFT[1:129];

                    ui.pretemplateView.clear()
                    ui.pretemplateView.plot(GlobalVars.templatepre,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000,pen=pg.mkPen('r', width=1))    
                    ui.pretemplateView.plot(PreMag,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)
                    ui.DP_ValPre.setText("Match Value: "+str(tempd));
                    GlobalVars.PreMatchVal=tempd;
            except IndexError as e:
            #   print("Bad Alignment")
               print(e)
            #   pdb.set_trace();
               return False # something dind't work. 

    return False
#    else:

def shiftTemplateDown(ui):
    import GlobalVars
    import pdb
    from numpy import roll, arange, array2string
    
    GlobalVars.template=roll(GlobalVars.template,-1)
    GlobalVars.template[0]=-5;
    GlobalVars.template[127]=0;

    newTemplate=str(GlobalVars.template)
    newTemplate=newTemplate.replace('[','')
    newTemplate=newTemplate.replace(']','')

    ui.MagsfromTeensy.clear();
    ui.MagsfromTeensy.plot(GlobalVars.template,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)

    ui.PrePostRatioPlot.clear();

    if (GlobalVars.isRunning==1):
        GlobalVars.ser.write(str.encode('SET TEMPLATE ' + newTemplate +';'))
    else:        
        GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
        GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
        GlobalVars.ser.flush()    
        GlobalVars.ser.write(str.encode('SET TEMPLATE ' + newTemplate +';'))                
        GlobalVars.ser.close();

def shiftTemplateUp(ui):
    import GlobalVars
    from numpy import roll, arange
    import pdb
    
    GlobalVars.template=roll(GlobalVars.template,1)
    GlobalVars.template[0]=-5;
    GlobalVars.template[127]=0;

    newTemplate=str(GlobalVars.template)
    newTemplate=newTemplate.replace('[','')
    newTemplate=newTemplate.replace(']','')

    ui.MagsfromTeensy.clear();
    ui.MagsfromTeensy.plot(GlobalVars.template,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)    

    if (GlobalVars.isRunning==1):
        GlobalVars.ser.write(str.encode('SET TEMPLATE ' + newTemplate +';'))
    else:        
        GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
        GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
        GlobalVars.ser.flush()    
        GlobalVars.ser.write(str.encode('SET TEMPLATE ' + newTemplate +';'))                
        GlobalVars.ser.close();    

def setAllButtons(ui,state):
        ui.startButton.setEnabled(state)
        ui.stopButton.setEnabled(state)
        ui.serialScan.setEnabled(state)
        ui.Teensy_Com_ComboBox.setEnabled(state)
        ui.actionLoad_Config.setEnabled(state)
        ui.actionSave_Config.setEnabled(state)
        ui.FileAndPath_PushButton.setEnabled(state)
        ui.ThresholdUpdateThresholdspinBox.setEnabled(state)
        ui.CatchPercentspinBox.setEnabled(state)
        ui.SampleRateComboBox.setEnabled(state)
        ui.Teensy_USB_ComboBox.setEnabled(state);


def TriggeredRecordAudio(ui,app):

     import GlobalVars
     import pdb
     import os
     from numpy import histogram, array, arange
     import sys     
     import time
     import pyqtgraph as pg
     
     QueSize=250;
     QueCnt=50;
     
    # ui.MagsfromTeensy.clear()
     MIN_DUR=(GlobalVars.buffertime)+0.1;
  
     
     
     SILENCE_LIMIT = 2;
     PREV_AUDIO = GlobalVars.buffertime;

     p = pyaudio.PyAudio()

     GlobalVars.AUDIO_CHANNELS=2;  

     stream=p.open(format=FORMAT,input_device_index=GlobalVars.inputdeviceindex,channels=CHANNELS,rate=RATE,
                   input=True,
                   frames_per_buffer=CHUNK)

    
     ui.ListeningTextBox.setText('<span style="color:green">quiet</span>')
     audio2send = []
     
     #cur_data = '' # current chunk of audio data
     rel = int(RATE/CHUNK)
     slid_win = deque(maxlen=SILENCE_LIMIT * rel) #amplitude threshold running buffer
     prev_audio = deque(maxlen=PREV_AUDIO * rel) #prepend audio running buffer
     perm_win = deque(maxlen=PREV_AUDIO*rel)

     started = False
     cur_data=stream.read(CHUNK)

     count=1;
     ui.FFMonitorPlot.clear()
     ui.FFMonitorPlotTrials.clear()
     ui.DPHistGraph.clear()     
     ui.DPvsFFGraph.clear()
     ui.RMSvsFFGraph.clear()  
    
     T=time.localtime()
     outtime=str("%02d"%T[0])+str("%02d"%T[1])+str("%02d"%T[2])+str("%02d"%T[3])+str("%02d"%T[4])+str("%02d"%T[5])
     BoutLogFileName = GlobalVars.SavePath+'_'+outtime+'Full.TAFLog'
     #GlobalVars.FullExpOutputFileName=open(BoutLogFileName,'a');
     AllMagsFileName= GlobalVars.SavePath+'_'+outtime+'AllMagsFull.TAFLog'

    
     while (GlobalVars.isRunning==1):
           
      
      try:
         cur_data = stream.read(CHUNK)
      except:
        # print(time.clock)
         stream.close()
         stream=p.open(format=FORMAT,input_device_index=GlobalVars.inputdeviceindex,channels=CHANNELS,rate=RATE,
                   input=True,
                   frames_per_buffer=CHUNK)
         cur_data = stream.read(CHUNK)
         #error=stream.Pa_GetLastHostErrorInfo()
  
      
      count=count+1
      if (count>5):
          count=0
          QtWidgets.qApp.processEvents()      
          QtGui.QGuiApplication.processEvents()
          
      #get new Audio
      thresh=audioop.tomono(cur_data,2,1,0)  # left channel  

      #RMS to thresholding          
      slid_win.append(audioop.rms(thresh, 2))
      rms = audioop.rms(thresh, 2)
      ui.RMSTextBox.setText(str(rms))
      perm_win.append(cur_data)      
      
      if (started):
          try:
              isUpdated=GetSerialData(ui);
          except:
              isUpdated=False;

      while (ui.isPausedCheckBox.isChecked()):
                         try: 
                             cur_data = stream.read(CHUNK) #keep reading but do nothing with the data            
                             GlobalVars.ser.flush()
                             QtGui.QGuiApplication.processEvents()
                         except:
                             print('Stopped while paused');
                                 
      
      ## HitDIR = 1 is 'hit above', HitDIR==0 is 'hit below'      
      if ((len(GlobalVars.FF)==QueSize) and isUpdated and GlobalVars.upDateThreshold): #Are we in update mode? Is the queue full?  
            SortedFFs=sorted(GlobalVars.FF) #Sort in ascending order
            QueCnt=QueCnt+1;
            #pdb.set_trace();
            if (GlobalVars.HitDIR==1) and (QueCnt>50):  # Hit Above, Updated every 50 trials. 
                percentileidx=int(QueSize*(1-(GlobalVars.UpDateThresholdPercent))) # 1-% (e.g. 
                #print(percentileidx);
                Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx+1])/2
                if GlobalVars.FreqTHRESH>Threshold: #ramp only down (hit above -> downshift)
                    GlobalVars.FreqTHRESH=int(Threshold);
                    GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
                    ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
                    print('Threshold :' + str(Threshold));
                QueCnt=0; #Restart Counter
                
            if (GlobalVars.HitDIR==0) and (QueCnt>50):   # Hit Below, Updated every 50 trials. 
                percentileidx=int(QueSize*(GlobalVars.UpDateThresholdPercent))
                print(percentileidx);                
                Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx-1])/2
                print('Threshold :' + str(Threshold));
                if GlobalVars.FreqTHRESH<Threshold: #Ramp only up (hit below->upshift)
                    GlobalVars.FreqTHRESH=int(Threshold);
                    #print('updated')
                    GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
                    ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
                    print('Threshold :' + str(Threshold));
                QueCnt=0; #Restart Counter

      isUpdated=False;


            
      if(sum([x > GlobalVars.AudioAmp for x in slid_win])>0 and len(audio2send)<MAX_DUR*rel):    
           if(not started):
                ui.ListeningTextBox.setText('<span style="color:red">singing</span>')
                started = True        
                #Set up Output File
                T=time.localtime()

                rootdir=QtCore.QFileInfo(GlobalVars.SavePath).path();
                rootfilename=QtCore.QFileInfo(GlobalVars.SavePath).fileName();

                DatePath='/'+str("%02d"%T[0])+'_'+str("%02d"%T[1])+'_'+str("%02d"%T[2])+'/'

                if not os.path.exists(os.path.dirname(rootdir+DatePath)):
                    try:
                        os.makedirs(os.path.dirname(rootdir+DatePath))
                    except:
                        print('File error- bad directory?')
                        
                outtime=str("%02d"%T[0])+str("%02d"%T[1])+str("%02d"%T[2])+str("%02d"%T[3])+str("%02d"%T[4])+str("%02d"%T[5])
                TrialLogFileName = rootdir+DatePath+rootfilename+'_'+outtime+'.TAF'
                MagFileName = rootdir+DatePath+rootfilename+'_'+outtime+'.TAFMAG'
                BaseFilename = rootdir+DatePath+rootfilename+'_'+outtime;
                #GlobalVars.SavePath+'_'+outtime+'AllMagsFull.TAFLog'

                GlobalVars.AllMagsFile=open(AllMagsFileName,'a'); #Full mags files to line up w/ full FF
                GlobalVars.FullExpOutputFileName=open(BoutLogFileName,'a');   
                GlobalVars.MagsOutFileName=open(MagFileName,'a'); #Trial by trial version for each .WAV file
                GlobalVars.TrialExpOutputFileName=open(TrialLogFileName,'a');                #Trial by trial version for each .WAV file
           audio2send.append(cur_data) # fucking indents. 
      elif (started is True and len(audio2send)>(MIN_DUR*rel)):
       print("Finished")
       filename = save_audio(list(prev_audio) + audio2send,BaseFilename)
       GlobalVars.TrialExpOutputFileName.close();
       GlobalVars.MagsOutFileName.close();
       GlobalVars.FullExpOutputFileName.close();
       GlobalVars.AllMagsFile.close()
       started = False
       slid_win = deque(maxlen=SILENCE_LIMIT * rel)
       prev_audio = copy.copy(perm_win)
       ui.ListeningTextBox.setText('<span style="color:green">quiet</span>')
       audio2send=[]
      elif (started is True):
       ui.ListeningTextBox.setText('too short')
       print("TooShort")
       started = False
       GlobalVars.TrialExpOutputFileName.close();
       GlobalVars.MagsOutFileName.close();
       GlobalVars.FullExpOutputFileName.close(); #close but do not delete
       GlobalVars.AllMagsFile.close()
       os.remove(MagFileName);  #get rid of empty files
       os.remove(TrialLogFileName);       
       slid_win = deque(maxlen=SILENCE_LIMIT * rel)
       prev_audio = copy.copy(perm_win)
       audio2send=[]
       ui.ListeningTextBox.setText('<span style="color:green">quiet</span>')       
      else:
       prev_audio.append(cur_data)
            
      
      if (ui.isPausedCheckBox.isChecked()): #Pause temporarily, close files, keep audiostream going
            audio2send=[]
            slid_win = deque(maxlen=SILENCE_LIMIT * rel)
            prev_audio = copy.copy(perm_win)
            
            
                
                 


     print("done recording")
     
     stream.close()
     GlobalVars.TrialExpOutputFileName.close();
     GlobalVars.MagsOutFileName.close();
     GlobalVars.FullExpOutputFileName.close();
     GlobalVars.AllMagsFile.close();
     
     p.terminate()

def save_audio(data,filename):
     import GlobalVars
        
     """ Saves mic data to  WAV file. Returns filename of saved
     file """


     data = b''.join(data) #Python 3 requires casting to bytes
     if (GlobalVars.DirFlag):
         filename=filename+'dir'
         
     wf = wave.open(filename + '.wav', 'wb')
     wf.setnchannels(CHANNELS);
     wf.setsampwidth(2)
     wf.setframerate(RATE) 
     wf.writeframes(data)
     wf.close()
     return filename + '.wav'
 

        
