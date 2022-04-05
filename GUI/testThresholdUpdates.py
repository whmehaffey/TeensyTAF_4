import GlobalVars
GlobalVars.HitDIR=1;
QueCnt=50;
GlobalVars.UpDateThresholdPercent=0.75
GlobalVars.FF=[50,66,100,80,86,75,100,100,100,100,100,50,50,60]
GlobalVars.FreqTHRESH=90
QueSize=14;

SortedFFs=sorted(GlobalVars.FF) #Sort in ascending order
QueCnt=QueCnt+1;

      #                   QtGui.qApp.processEvents()
      ## HitDIR = 1 is 'hit above', HitDIR==0 is 'hit below'     

if (GlobalVars.HitDIR==1) and (QueCnt>50):  # Hit Above, Updated every 50 trials. 
    percentileidx=int(QueSize*(1-(GlobalVars.UpDateThresholdPercent))) # 1-% (e.g. 
    print(percentileidx);
 
    print(GlobalVars.FreqTHRESH)
    Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx+1])/2
    print('IDX Thresh: ' + str(Threshold))
    if GlobalVars.FreqTHRESH>Threshold: #ramp only down (hit above -> downshift)
        GlobalVars.FreqTHRESH=int(Threshold);
        print('upshift')
    #    GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
    #    ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
        print('Threshold :' + str(Threshold));
        QueCnt=0; #Restart Counter
    
if (GlobalVars.HitDIR==0) and (QueCnt>50):   # Hit Below, Updated every 50 trials. 
    percentileidx=int(QueSize*(GlobalVars.UpDateThresholdPercent))
    print(percentileidx);                
    Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx-1])/2
    print('IDX Thresh: ' + str(Threshold))
    print(GlobalVars.FreqTHRESH)
    print(GlobalVars.FreqTHRESH<int(Threshold))
    if (GlobalVars.FreqTHRESH<int(Threshold)): #Ramp only up (hit below->upshift)
        GlobalVars.FreqTHRESH=int(Threshold);
        print('updated')
        print('downshift')
    #    GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
     #   ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
        print('Threshold :' + str(Threshold));
        QueCnt=0; #Restart Counter
