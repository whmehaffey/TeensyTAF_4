

global isRunning
global CurrentPort
global FreqTHRESH
global MinFF
global DPTHRESH
global MaxFF
global HitDIR
global AMP
global sampleBin
global SavePath
global FFT
global SamplingRate
global WN_ON
global upDateThreshold
global DirFlag
global UpDateThresholdPercent
global CatchTrialPercent
global NumAudioDevices
global ser
global buffertime
global threshold
global path
global filename
global lastFFT
global FFAveraging
global FullExpOutputFileName
global TrialExpOutputFileName
global MagsOutFileName
global Gain
global AudioAmp
global AMPMAX
global isPaused
global DPTHRESHPRE
global templatepre
global PreTemplateFlag
global PreDelayMax;
global PreDelayMin;
global DPCountPre;
global AllMagsFile;
global PreMatchVal;


def loadConfig(loadfilename):
    from configparser import ConfigParser
    import os
    import GlobalVars
    import serial
    import pdb;
    from numpy import arange,array

    parser = ConfigParser()
    loadfilename=loadfilename.replace('/','\\')   

    if not parser.read(str(loadfilename)): #.replace('/','\\')):
        raise(IOError, 'cannot load')
        
    GlobalVars.MaxFF=float(parser.get('main','GlobalVars.MaxFF'))
    GlobalVars.MinFF=float(parser.get('main','GlobalVars.MinFF'))
    GlobalVars.AMP=float(parser.get('main','GlobalVars.AMP'))
    GlobalVars.DPTHRESH=float(parser.get('main','GlobalVars.DPTHRESH'))
    GlobalVars.FreqTHRESH=float(parser.get('main','GlobalVars.FreqTHRESH'))
    GlobalVars.SavePath=str(parser.get('main','GlobalVars.SavePath'))
    GlobalVars.WN_ON=bool(parser.getboolean('main','GlobalVars.WN_ON'))
    GlobalVars.HitDIR=int(parser.get('main','GlobalVars.HitDIR'))
    GlobalVars.DirFlag=bool(parser.getboolean('main','GlobalVars.DirFlag'))    
    GlobalVars.upDateThreshold=bool(parser.getboolean('main','GlobalVars.upDateThreshold'))
    GlobalVars.UpDateThresholdPercent=float(parser.get('main','UpDateThresholdPercent'))
    GlobalVars.CatchTrialPercent=int(parser.get('main','CatchTrialPercent'))


    try: #These weren't always there, so load if possible.  
        GlobalVars.FFAveraging=int(parser.get('main','FFAveraging'))
        GlobalVars.SamplingRate=int(parser.get('main','SamplingRate'))
        GlobalVars.Gain=float(parser.get('main','Gain'))
        GlobalVars.AudioAmp=float(parser.get('main','AudioAmp'));
        GlobalVars.AMPMAX=float(parser.get('main','Amp_Max'));
        GlobalVars.DPCount=int(parser.get('main','DPMatchDuration'))
        GlobalVars.AMPCount=int(parser.get('main','AmplitudeMatchDuration'))
        GlobalVars.PreDelayMin=int(parser.get('main','DelayMin'))
        GlobalVars.PreDelayMax=int(parser.get('main','DelayMax'))
        GlobalVars.DPCountPre=int(parser.get('main','PRE_DUR_MATCH_TEMPLATE'));
        GlobalVars.DPTHRESHPRE=float(parser.get('main','DPTHRESHPRE'));
        GlobalVars.PreTemplateFlag=bool(parser.get('main','PreFlagChecked'));
      #  pdb.set_trace();
        newTemplatepre=parser.get('template','GlobalVars.templatepre');

        GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
        GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
        GlobalVars.ser.flush()
    
        GlobalVars.ser.write(str.encode('SET PRETEMPLATE ' + newTemplatepre +';'))
        print('SET PRETEMPLATE ' + newTemplatepre +';')
        GlobalVars.ser.close();

        newTemplatepre=newTemplatepre.replace('[','')
        newTemplatepre=newTemplatepre.replace(']','')        
        GlobalVars.templatepre= array(newTemplatepre.split(','), dtype=float)        
        GlobalVars.AudioDeviceName=str(parser.get('main','AudioDeviceName'))
        GlobalVars.AudioDeviceName=GlobalVars.AudioDeviceName.replace('"','');
        
    except Exception as e:
        print(e)
        print('Old Config File');
      
    newTemplate=parser.get('template','GlobalVars.template');

    GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
    GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
    GlobalVars.ser.flush()
    
    GlobalVars.ser.write(str.encode('SET TEMPLATE ' + newTemplate +';'))
    print('SET TEMPLATE ' + newTemplate +';')
    GlobalVars.ser.close();

    newTemplate=newTemplate.replace('[','')
    newTemplate=newTemplate.replace(']','')        
    GlobalVars.template= array(newTemplate.split(','), dtype=float)


def saveConfig(savefilename):
    from configparser import ConfigParser
    import os
    import GlobalVars
    from numpy import array
             
    SaveFile= open((savefilename),'w')
    
    parser = ConfigParser()
    
    parser.add_section('main')
    parser.add_section('template')
    parser.set('main','GlobalVars.MaxFF',str(GlobalVars.MaxFF))
    parser.set('main','GlobalVars.MinFF',str(GlobalVars.MinFF))
    parser.set('main','GlobalVars.AMP',str(GlobalVars.AMP))
    parser.set('main','GlobalVars.DPTHRESH',str(GlobalVars.DPTHRESH))
    parser.set('main','GlobalVars.FreqTHRESH',str(GlobalVars.FreqTHRESH))
    parser.set('main','GlobalVars.SavePath',str(GlobalVars.SavePath));
    parser.set('template','GlobalVars.template',str(list(GlobalVars.template)))
    parser.set('template','GlobalVars.templatepre',str(list(GlobalVars.templatepre)))
    parser.set('main','GlobalVars.WN_ON',str(GlobalVars.WN_ON));
    parser.set('main','GlobalVars.HitDIR',str(GlobalVars.HitDIR));
    parser.set('main','GlobalVars.upDateThreshold',str(GlobalVars.upDateThreshold));
    parser.set('main','GlobalVars.FFT',str(GlobalVars.FFT))
    parser.set('main','GlobalVars.SamplingRate',str(GlobalVars.SamplingRate))
    parser.set('main','GlobalVars.DirFlag',str(GlobalVars.DirFlag))
    parser.set('main','UpDateThresholdPercent',str(GlobalVars.UpDateThresholdPercent))    
    parser.set('main','CatchTrialPercent',str(GlobalVars.CatchTrialPercent))
    parser.set('main','FFAveraging',str(GlobalVars.FFAveraging))
    parser.set('main','SamplingRate',str(GlobalVars.SamplingRate))
    parser.set('main','Gain',str(GlobalVars.Gain))
    parser.set('main','AudioAmp',str(GlobalVars.AudioAmp))
    parser.set('main','Amp_Max',str(GlobalVars.AMPMAX))
    parser.set('main','DPMatchDuration',str(GlobalVars.DPCount))
    parser.set('main','AmplitudeMatchDuration',str(GlobalVars.AMPCount))

    parser.set('main','DelayMin',str(GlobalVars.PreDelayMin))
    parser.set('main','DelayMax',str(GlobalVars.PreDelayMax))
    parser.set('main','PRE_DUR_MATCH_TEMPLATE',str(GlobalVars.DPCountPre))
    parser.set('main','DPTHRESHPRE',str(GlobalVars.DPTHRESHPRE))
    parser.set('main','PreFlagChecked',str(GlobalVars.PreTemplateFlag));        
    parser.set('main','AudioDevicename',str('"'+GlobalVars.AudioDeviceName+'"'));
    
    parser.write(SaveFile)    
    SaveFile.close()

def UpDateValues(ui):
    import GlobalVars
    from numpy import arange
    
    ui.editMAXFF.setText((str(GlobalVars.MaxFF)))
    ui.editMINFF.setText((str(GlobalVars.MinFF)))
    ui.editDP_Thresh.setText((str(GlobalVars.DPTHRESH)))
    ui.editFREQ_THRESH.setText((str(GlobalVars.FreqTHRESH)))
    ui.editMINAMP.setText(str(GlobalVars.AMP))
    ui.editMAXAMP.setText(str(GlobalVars.AMPMAX))
    ui.editAUDIOAMPTHRESHOLD.setText(str(GlobalVars.AudioAmp))
    ui.SaveFilePathLabel.setText(GlobalVars.SavePath[0])
    ui.WN_OncheckBox.setChecked(GlobalVars.WN_ON)
    ui.upDateThresholdCheckBox.setChecked(bool(GlobalVars.upDateThreshold))
    ui.DirFlagCheckBox.setChecked(bool(GlobalVars.DirFlag))
    ui.pretemplateView.clear();
    ui.pretemplateView.plot(GlobalVars.templatepre,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin)/1000))
    ui.CatchPercentspinBox.setValue(int(GlobalVars.CatchTrialPercent))
    ui.ThresholdUpdateThresholdspinBox.setValue(int(GlobalVars.UpDateThresholdPercent*100))    
    ui.CatchPercentspinBox.setValue(GlobalVars.CatchTrialPercent)
    ui.ThresholdUpdateThresholdspinBox.setValue(int(GlobalVars.UpDateThresholdPercent*100))
    ui.AveragingSpinBox.setValue(GlobalVars.FFAveraging)
    ui.LineInDoubleSpinBox.setValue(float(GlobalVars.Gain))
    ui.DPCountSpinBox.setValue(int(GlobalVars.DPCount))
    ui.AmpCountSpinBox.setValue(GlobalVars.AMPCount)    
    ui.editDP_ThreshPre.setText(str(GlobalVars.DPTHRESHPRE));
    ui.DPCountSpinBoxPre.setValue(GlobalVars.DPCountPre);
    ui.editPreDelayMin.setText(str(GlobalVars.PreDelayMin));
    ui.editPreDelayMax.setText(str(GlobalVars.PreDelayMax));
    ui.PreTemplatecheckBox.setChecked(bool(GlobalVars.PreTemplateFlag));
    ui.MagsfromTeensy.clear();
    ui.MagsfromTeensy.plot(GlobalVars.template,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)
    ui.Teensy_USB_ComboBox.setCurrentText(GlobalVars.AudioDeviceName);
    

    if (GlobalVars.PreTemplateFlag):        
        ui.pretemplateView.clear();
        ui.pretemplateView.plot(GlobalVars.templatepre,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)


    index = ui.SampleRateComboBox.findText(str(GlobalVars.SamplingRate))
    ui.SampleRateComboBox.setCurrentIndex(index);    
    
    if (GlobalVars.HitDIR==1):
        ui.HitBelowButton.setChecked(True)
        ui.HitAboveButton.setChecked(False)
    if (GlobalVars.HitDIR==0):
        ui.HitAboveButton.setChecked(True)
        ui.HitBelowButton.setChecked(False)

def SendAllToTeensy():
    import GlobalVars
    import numpy

    
    GlobalVars.ser.write(str.encode('SET FF_MIN ' + str(GlobalVars.MinFF) + ';'))
    GlobalVars.ser.write(str.encode('SET FF_MAX ' + str(GlobalVars.MaxFF) + ';'))
    GlobalVars.ser.write(str.encode('SET DPTHRESH ' + str(GlobalVars.DPTHRESH) + ';'))
    GlobalVars.ser.write(str.encode('SET AMP_THRESHOLD ' + str(GlobalVars.AMP) + ';'))
    GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))
    GlobalVars.ser.write(str.encode('SET FREQDIR ' + str(int(GlobalVars.HitDIR)) + ';'))
    GlobalVars.ser.write(str.encode('SET PLAYWN ' + str(int(GlobalVars.WN_ON)) + ';'))
    GlobalVars.ser.write(str.encode('SET ISDIR ' + str(int(GlobalVars.DirFlag)) + ';'))
    GlobalVars.ser.write(str.encode('SET CATCHPERCENT ' + str(GlobalVars.CatchTrialPercent) + ';'))
    GlobalVars.ser.write(str.encode('SET SAMPLE_RATE_HZ ' + str(GlobalVars.SamplingRate) + ';'))
    GlobalVars.ser.write(str.encode('SET AVERAGING ' + str(GlobalVars.FFAveraging) + ';'))
    GlobalVars.ser.write(str.encode('SET LINE_IN_LEVEL ' + str(GlobalVars.Gain) + ';'))
    GlobalVars.ser.write(str.encode('SET AMP_MAX ' + str(GlobalVars.AMPMAX) + ';'))
    GlobalVars.ser.write(str.encode('SET DUR_MATCH_TEMPLATE ' + str(GlobalVars.DPCount) + ';'))
    GlobalVars.ser.write(str.encode('SET AMPLITUDE_THRESHOLD_DURATION ' + str(GlobalVars.AMPCount) + ';'))    
    GlobalVars.ser.write(str.encode('SET MINDELAY ' + str(GlobalVars.PreDelayMin) + ';'))
    GlobalVars.ser.write(str.encode('SET MAXDELAY ' + str(GlobalVars.PreDelayMax) + ';'))
    GlobalVars.ser.write(str.encode('SET PRE_DUR_MATCH_TEMPLATE ' + str(GlobalVars.DPCountPre) + ';'))
    GlobalVars.ser.write(str.encode('SET PREDPTHRESH ' + str(GlobalVars.DPTHRESHPRE) + ';'));
    GlobalVars.ser.write(str.encode('SET TESTPRETEMPLATE' + str(GlobalVars.PreTemplateFlag) + ';'));
    
    
    print('SET FF_MIN ' + str(GlobalVars.MinFF) + ';')
    print('SET FF_MAX ' + str(GlobalVars.MaxFF) + ';')
    print('SET DPTHRESH ' + str(GlobalVars.DPTHRESH) + ';')
    print('SET AMP_THRESHOLD ' + str(GlobalVars.AMP) + ';')
    print('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';')
    print('SET FREQDIR ' + str((GlobalVars.HitDIR)) + ';')
    print('SET PLAYWN ' + str((GlobalVars.WN_ON)) + ';')
    print('SET ISDIR ' + str(int(GlobalVars.DirFlag)) + ';')
    print('SET PERCENTHITS ' + str(GlobalVars.CatchTrialPercent) + ';')
    print('SET SAMPLE_RATE_HZ ' + str(GlobalVars.SamplingRate) + ';')
    print('SET AVERAGING ' + str(GlobalVars.FFAveraging) + ';')
    print('SET DUR_MATCH_TEMPLATE_PRE ' + str(int(GlobalVars.DPCountPre)) + ';')
    print('SET PREDPTHRESH ' + str(GlobalVars.DPTHRESHPRE) + ';')
    print('SET MAXDELAY ' + str(GlobalVars.PreDelayMax) + ';')
    print('SET MINDELAY ' + str(GlobalVars.PreDelayMin) + ';')  
    


