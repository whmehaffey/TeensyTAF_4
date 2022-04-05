import GlobalVars;

QueSize=4;
GlobalVars.upDateThreshold=True;
GlobalVars.FF=[2000,2020,2040,1980]
GlobalVars.UpDateThresholdPercent=0.75
GlobalVars.HitDIR=1;
isUpdated=True;
QueCnt=0

if ((len(GlobalVars.FF)==QueSize) and isUpdated and GlobalVars.upDateThreshold): #Is Cue Full?
            SortedFFs=sorted(GlobalVars.FF)
            QueCnt=QueCnt+1;
            #pdb.set_trace();
            if (GlobalVars.HitDIR==1) and (QueCnt>50):  # Hit Above
                percentileidx=int(QueSize*(1-(GlobalVars.UpDateThresholdPercent)))
                print(percentileidx);
                Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx+1])/2
                if GlobalVars.FreqTHRESH<Threshold: #ramp only upwards
                    GlobalVars.FreqTHRESH=int(Threshold);
                GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
                ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
                print('Threshold :' + str(Threshold));
                QueCnt=0;
                
            if (GlobalVars.HitDIR==0) and (QueCnt>50):   # Hit Below
                percentileidx=int(QueSize*(GlobalVars.UpDateThresholdPercent))
                print(percentileidx);                
                Threshold=(SortedFFs[percentileidx]+SortedFFs[percentileidx+1])/2
                if GlobalVars.FreqTHRESH<Threshold: #Ramp only downwards
                    GlobalVars.FreqTHRESH=int(Threshold);
                GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
                ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
                print('Threshold :' + str(Threshold));
                QueCnt=0;
