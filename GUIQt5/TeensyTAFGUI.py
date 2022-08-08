
import sys
from PyQt5.QtWidgets import QApplication,QDialog,QSizeGrip,QMessageBox
from PyQt5 import QtCore, QtGui, uic, QtWidgets
from collections import deque
import serial as serial
from numpy import array        
 
qtCreatorFile = "Gui.ui" # Enter file here.
import GlobalVars

 
Ui_MainWindow, QtBaseClass = uic.loadUiType(qtCreatorFile)

def GetTeensyPorts():
    import GlobalVars    
    import sys, serial
    import pyaudio as pa
    from serial.tools import list_ports
    import pdb

    TeensyPort = (list_ports.grep("16c0"))

    i=0;
    temp=[]
    ui.Teensy_Com_ComboBox.clear();
    for p in TeensyPort:                              
               temp = p[0]
               #print(temp)
               ui.Teensy_Com_ComboBox.insertItem(0,str(temp))


    if (len(temp)>0):
        GlobalVars.CurrentPort=(temp)
    else:
        print("No Teensy Serial Device")
   
    inputdevices = 0
    
    pya = pa.PyAudio()
    info = pya.get_host_api_info_by_index(0)
    DeviceList = info.get('deviceCount')
    
    
    ui.Teensy_USB_ComboBox.disconnect();
    ui.Teensy_USB_ComboBox.clear();
    #for each audio device, determine if is an input or an output and add it to the appropriate list and dictionary
    for i in range (0,DeviceList):
        #print(pya.get_device_info_by_host_api_device_index(0,i).get('maxInputChannels'))
        if pya.get_device_info_by_host_api_device_index(0,i).get('maxInputChannels')>1:
             print(pya.get_device_info_by_host_api_device_index(0,i).get('name'))
            #if ((pya.get_device_info_by_host_api_device_index(0,i).get('name').find("Te"))!=-1):
             ui.Teensy_USB_ComboBox.insertItem(20,str(pya.get_device_info_by_host_api_device_index(0,i).get('name')))
             inputdevices+=1
 

    GlobalVars.NumAudioDevices=inputdevices
    ui.Teensy_USB_ComboBox.currentIndexChanged.connect(SetAudioIn)
    GlobalVars.AudioDeviceName=ui.Teensy_USB_ComboBox.currentText();
        
    pya.terminate()    
        
    if (len(temp)>0):
        GlobalVars.CurrentPort=(temp)
    else:
        print("No Teensy Detected")

def SetAudioIn():
    import GlobalVars
    GlobalVars.inputdeviceindex=ui.Teensy_USB_ComboBox.currentIndex();
    GlobalVars.AudioDeviceName=ui.Teensy_USB_ComboBox.currentText();
    print(GlobalVars.inputdeviceindex);
    
    
def startButtonPressed():
    import GlobalVars
    import Functions
    import serial
    #import numpy as np
    from numpy import histogram, array, arange
    import sys
    import os
    import time
    from Functions import setAllButtons
    from Functions import TriggeredRecordAudio
    import pdb

    QueSize=250;
    GlobalVars.meanFF=0;
    TemplateCurrent=True;
    templateOverPlot=0;
    GlobalVars.FF = deque(maxlen=QueSize)
    GlobalVars.HIT = deque(maxlen=QueSize)
    GlobalVars.DP = deque(maxlen=QueSize)
    GlobalVars.RMS = deque(maxlen=QueSize)
    GlobalVars.PrePostRatio = deque(maxlen=QueSize);
    GlobalVars.PreTemplateCounter = 0;
    GlobalVars.PostTemplateCounter = 0;
    GlobalVars.UpDateValues(ui);

    try:
        GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),9600)
    except:
        messageBox=QMessageBox()
        messageBox.setWindowTitle("Serial Error")
        messageBox.setText("Serial Port Already Open?")
        messageBox.setFixedSize(500,200);
        messageBox.show()
        setAllButtons(ui,True);
        ui.stopButton.setEnabled(False);
        ui.startButton.setEnabled(True);
    
    GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)

    date=time.localtime();

    RemoveAppendix=str(GlobalVars.SavePath);
    RemoveAppendix.strip('.TAFlog')
    BaseFileName=RemoveAppendix+str(date[0])+','+str(date[1])+','+str(date[2])+','+str(date[3])+','+str(int(time.time()))

    #Set up output files for the full run (Config, and full experiment Log- file by file version are implemented in TriggeredRecordAudio
    configfilename=BaseFileName+'.TAFcfg'
    GlobalVars.saveConfig(configfilename)                   #Save Config,
    setAllButtons(ui,False);
    ui.stopButton.setEnabled(True);
    GlobalVars.ser.flush()        
    GlobalVars.SendAllToTeensy()
    GlobalVars.ser.write(str.encode('SET PLOTMAGS 0;'))
    GlobalVars.ser.write(str.encode('SET PreTemplate ' + str(ui.PreTemplatecheckBox.isChecked()) + ';'))
    GlobalVars.ser.write(str.encode('START;'))
    GlobalVars.isRunning=True
    GlobalVars.ser.flush();
    
    TriggeredRecordAudio(ui,app);
    

   
    
def stopButtonPressed():
    import GlobalVars
    from Functions import setAllButtons
    
    GlobalVars.isRunning=False;
    GlobalVars.ser.flush();
    GlobalVars.ser.write(str.encode('STOP;'));
    GlobalVars.ser.close();
    setAllButtons(ui,True);
    ui.stopButton.setEnabled(False);
    ui.startButton.setEnabled(True);
    
    
def updateTemplate():
    import GlobalVars    
    from configparser import SafeConfigParser
    from numpy import arange, array
    import serial
    import pdb

    loadfilename = (QtWidgets.QFileDialog.getOpenFileName(ui,'Open Template File',GlobalVars.DirPath,'*.TMPLT'))
    GlobalVars.DirPath = QtCore.QFileInfo(loadfilename[0]).path();
    loadfilename=loadfilename[0].replace('/','\\') #.replace('/','\\')):

    parser=SafeConfigParser()
    if not parser.read(str(loadfilename)):
        raise(IOError, 'cannot load')
    
    newTemplate=parser.get('template','GlobalVars.template');
    print(newTemplate)
    newTemplate=newTemplate.replace('[','')
    newTemplate=newTemplate.replace(']','')
    
    GlobalVars.template= array(newTemplate.split(','), dtype=float)

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
        
def uploadtemplatepreButtonPressed():
    import GlobalVars   
    from configparser import SafeConfigParser
    from numpy import arange, array
    import serial
    import pdb

    loadfilename = (QtWidgets.QFileDialog.getOpenFileName(ui,'Open Template File',GlobalVars.DirPath,'*.TMPLT'))
    GlobalVars.DirPath = QtCore.QFileInfo(loadfilename[0]).path();
    loadfilename=loadfilename[0].replace('/','\\') #.replace('/','\\')):

    parser=SafeConfigParser()
    if not parser.read(str(loadfilename)): 
        raise(IOError, 'cannot load')
    
    newTemplate=parser.get('template','GlobalVars.template');
    print(newTemplate)
    newTemplate=newTemplate.replace('[','')
    newTemplate=newTemplate.replace(']','')
    
    GlobalVars.templatepre= array(newTemplate.split(','), dtype=float)

    ui.pretemplateView.clear();
    ui.pretemplateView.plot(GlobalVars.templatepre,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000)

    if (GlobalVars.isRunning==1):
        GlobalVars.ser.write(str.encode('SET PRETEMPLATE ' + newTemplate +';'))
    else:        
        GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
        GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
        GlobalVars.ser.flush()    
        GlobalVars.ser.write(str.encode('SET PRETEMPLATE ' + newTemplate +';'))
        GlobalVars.ser.close();
        

def loadConfig_ButtonPressed():
    import os
    import GlobalVars       
            
    loadfilename = (QtWidgets.QFileDialog.getOpenFileName(ui,'Open Config File', GlobalVars.DirPath,'*.TAFcfg'))
    GlobalVars.DirPath = QtCore.QFileInfo(loadfilename[0]).path();
    GlobalVars.loadConfig(loadfilename[0])
    GlobalVars.UpDateValues(ui)
    
    GlobalVars.ser=serial.Serial(str(GlobalVars.CurrentPort),115200)
    GlobalVars.ser.set_buffer_size(rx_size = 50000, tx_size = 50000)
    GlobalVars.ser.flush()        
    GlobalVars.SendAllToTeensy();
    GlobalVars.ser.close();    

def saveConfig_ButtonPressed():
    import GlobalVars
    savefilename = (QtWidgets.QFileDialog.getSaveFileName(ui,'Save Config File', GlobalVars.DirPath, '.TAFcfg'))
    GlobalVars.DirPath = QtCore.QFileInfo(savefilename[0]).path();
    GlobalVars.saveConfig(savefilename[0])


def SaveFileButtonPressed():
    import os
    import GlobalVars

    GlobalVars.SavePath = QtWidgets.QFileDialog.getSaveFileName(ui,'Save File', GlobalVars.DirPath, '')
    GlobalVars.SavePath = GlobalVars.SavePath[0]  # get str from tuple
    #GlobalVars.SavePath = GlobalVars.SavePath[1:-1]  # remove quotation marks from str
    GlobalVars.DirPath = QtCore.QFileInfo(GlobalVars.SavePath).path();
    ui.SaveFilePathLabel.setText(GlobalVars.SavePath)
    

def editAMP():
    import GlobalVars

    GlobalVars.AMP=float(ui.editMINAMP.text());    
    if GlobalVars.isRunning:            
        GlobalVars.ser.write(str.encode('SET AMP_THRESHOLD ' + str(GlobalVars.AMP) + ';'))
     
def editAMPMax():
    import GlobalVars

    GlobalVars.AMPMAX=float(ui.editMAXAMP.text());
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET AMP_MAX ' + str(GlobalVars.AMPMAX) + ';'))

def editDPTHRESH():
    import GlobalVars

    GlobalVars.DPTHRESH=float(ui.editDP_Thresh.text());
    if GlobalVars.isRunning:            
        GlobalVars.ser.write(str.encode('SET DPTHRESH ' + str(GlobalVars.DPTHRESH) + ';'))
        
def editDPTHRESHPrePressed():
     import GlobalVars

     GlobalVars.DPTHRESHPRE=float(ui.editDP_ThreshPre.text());
     if GlobalVars.isRunning:            
        GlobalVars.ser.write(str.encode('SET PREDPTHRESH ' + str(GlobalVars.DPTHRESHPRE) + ';'))
        
def editFFMAX():
    import GlobalVars

    GlobalVars.MaxFF=float(ui.editMAXFF.text());
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET FF_MAX ' + str(GlobalVars.MaxFF) + ';'))

def editFFMIN():
    import GlobalVars

    GlobalVars.MinFF=float(ui.editMINFF.text());
    if GlobalVars.isRunning:        
        GlobalVars.ser.write(str.encode('SET FF_MIN ' + str(GlobalVars.MinFF) + ';'))
                                 
def editFFreqThresh():
    import GlobalVars
    import pyqtgraph as pg
    
    GlobalVars.FreqTHRESH=float(ui.editFREQ_THRESH.text());
    
    ui.FFMonitorPlotTrials.addLine(x=None, y=GlobalVars.FreqTHRESH,pen=pg.mkPen('r', width=1))
    ui.FFMonitorPlot.addLine(x=GlobalVars.FreqTHRESH, y=None,pen=pg.mkPen('r', width=1))
    
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET FREQTHRESH ' + str(GlobalVars.FreqTHRESH) + ';'))

def WN_OnPressed():
    import GlobalVars

    GlobalVars.WN_ON=ui.WN_OncheckBox.isChecked();
    if GlobalVars.isRunning:        
        GlobalVars.ser.write(str.encode('SET PLAYWN ' + str(int(GlobalVars.WN_ON)) + ';'))

def BufferTimeSpinBoxChanged():
    import GlobalVars
    GlobalVars.buffertime=ui.BufferTimeSpinBox.value();


def upDateThresholdButtonPressed():
    import GlobalVars

    GlobalVars.upDateThreshold=ui.upDateThresholdCheckBox.isChecked()
    
    if GlobalVars.upDateThreshold:
            ui.ThresholdUpdateThresholdspinBox.setEnabled(True)
    else:
            ui.ThresholdUpdateThresholdspinBox.setEnabled(False)
    

def updateDirFlag():
    import GlobalVars
    GlobalVars.DirFlag=ui.DirFlagCheckBox.isChecked()

    if (GlobalVars.isRunning):
        GlobalVars.ser.write(str.encode('SET ISDIR ' + str(int(GlobalVars.DirFlag)) + ';'))

def SetLineInGain():
    import GlobalVars
    GlobalVars.Gain=ui.LineInDoubleSpinBox.value()
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET LINE_IN_LEVEL ' + str(GlobalVars.Gain) + ';'))

def HitAboveButtonPressed():
    import GlobalVars    
    
    GlobalVars.HitDIR=1
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET FREQDIR ' + str(int(GlobalVars.HitDIR)) + ';'))

def HitBelowButtonPressed():
    import GlobalVars
    
    GlobalVars.HitDIR=0
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET FREQDIR ' + str(int(GlobalVars.HitDIR)) + ';'))

def updateThreshholdPercent():
    import GlobalVars
    GlobalVars.UpDateThresholdPercent=ui.ThresholdUpdateThresholdspinBox.value()/100;

def setAveraging():
    import GlobalVars
    GlobalVars.FFAveraging=ui.AveragingSpinBox.value()
    if GlobalVars.isRunning: 
        GlobalVars.ser.write(str.encode('SET AVERAGING ' + str(GlobalVars.FFAveraging) + ';'))

def setSampleRate():
    import GlobalVars    # Off when running....
    GlobalVars.SamplingRate= int(ui.SampleRateComboBox.currentText()) # so always updated
    GlobalVars.sampleBin=(GlobalVars.SamplingRate/2)/(GlobalVars.FFT/2);
    print(GlobalVars.sampleBin)
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET SAMPLE_RATE_HZ ' + str(GlobalVars.SamplingRate) + ';'))
    
def updateCatchPercent():
    
    import GlobalVars
    GlobalVars.CatchTrialPercent=ui.CatchPercentspinBox.value()
    
    if GlobalVars.isRunning: 
       GlobalVars.ser.write(str.encode('SET CATCHPERCENT ' + str(GlobalVars.CatchTrialPercent) + ';'))
    

def updateAMPCount():
    import GlobalVars
    GlobalVars.AMPCount=ui.AmpCountSpinBox.value()
    
    if GlobalVars.isRunning: 
       GlobalVars.ser.write(str.encode('SET AMPLITUDE_THRESHOLD_DURATION ' + str(GlobalVars.AMPCount) + ';'))
    
def updateDPCount():
    import GlobalVars
    GlobalVars.DPCount=ui.DPCountSpinBox.value()
    
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET DUR_MATCH_TEMPLATE ' + str(GlobalVars.DPCount) + ';'))

def updateDPCountSpinBoxPre():
    import GlobalVars
    GlobalVars.DPCountPre=ui.DPCountSpinBoxPre.value()
    
    if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET PRE_DUR_MATCH_TEMPLATE ' + str(GlobalVars.DPCountPre) + ';'))

def TrackFFButtonPressed():
    import GlobalVars
    import numpy
    
    if (len(GlobalVars.FF)>5):
        GlobalVars.meanFF=sum(GlobalVars.FF)/len(GlobalVars.FF);
    ui.TrackedFFLabel.setText("Tracked FF: "+str(GlobalVars.meanFF));

def editPreDelayMaxPressed():
    import GlobalVars
    GlobalVars.PreDelayMax=int(ui.editPreDelayMax.text())
      
    if GlobalVars.isRunning: 
       GlobalVars.ser.write(str.encode('SET MAXDELAYMAX ' + str(GlobalVars.PreDelayMax) + ';'))
    
def editPreDelayMinPressed():
    import GlobalVars
    GlobalVars.PreDelayMin=int(ui.editPreDelayMin.text())
    
    if GlobalVars.isRunning:
       GlobalVars.ser.write(str.encode('SET MINDELAY ' + str(GlobalVars.PreDelayMin) + ';'))
       

def updatePausing():
    import GlobalVars
    import os


    if (ui.isPausedCheckBox.isChecked()):
        ui.ListeningTextBox.setText('<span style="color:yellow">PAUSED</span>')
        GlobalVars.ser.write(str.encode('STOP;'))            
    else:
        GlobalVars.ser.write(str.encode('START;'))                        
        ui.ListeningTextBox.setText('<span style="color:green">quiet</span>')

def shiftTemplateDownButtonPressed():
    from Functions import shiftTemplateDown
    shiftTemplateDown(ui)
    
def shiftTemplateUpButtonPressed():
    from Functions import shiftTemplateUp
    shiftTemplateUp(ui)        

def PreTemplatecheckBoxChecked():     
     if GlobalVars.isRunning:
        GlobalVars.ser.write(str.encode('SET PRETEMPLATE ' + str(ui.PreTemplatecheckBox.isChecked()) + ';'))
     GlobalVars.PreTemplateFlag=ui.PreTemplatecheckBox.isChecked();

def editUSBThreshold():
     import GlobalVars;
     GlobalVars.AudioAmp=int(ui.editAUDIOAMPTHRESHOLD.text())


    
class MainWindow(QtWidgets.QMainWindow, Ui_MainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        Ui_MainWindow.__init__(self)
        self.setupUi(self)

        from numpy import arange, array, zeros
        
        import GlobalVars;
        import os;
        import pyqtgraph as pg
       
        
        GlobalVars.isRunning=False;
        
        GlobalVars.buffertime=3        
        GlobalVars.DirPath=os.path.normcase('.\\')
        GlobalVars.filename='birdname'

        #Initialize Teensy Variables
        GlobalVars.FFT=256; #int(self.NFFTcomboBox.currentText())
        GlobalVars.SamplingRate=int(self.SampleRateComboBox.currentText())
        GlobalVars.inputdeviceindex=0;
        GlobalVars.FFAveraging=3;
        GlobalVars.FreqTHRESH=0
        GlobalVars.MinFF=0
        GlobalVars.MaxFF=0
        GlobalVars.DPTHRESH=5
        GlobalVars.HitDIR=0
        GlobalVars.AMP=1200
        GlobalVars.AMPMAX=6000;
        GlobalVars.AudioAmp=900
        GlobalVars.SavePath=".//BirdName"
        GlobalVars.WN_ON=0
        GlobalVars.upDateThreshold=False
        GlobalVars.DirFlag=0
        GlobalVars.UpDateThresholdPercent=0.75
        GlobalVars.CatchTrialPercent=10
        GlobalVars.meanFF=0;
        GlobalVars.Gain=55;
        GlobalVars.AMPCount=5
        GlobalVars.DPCount=3;
        GlobalVars.isPaused=False;
        GlobalVars.PreTemplateFlag=False;
        GlobalVars.DPCountPre=2;
        GlobalVars.PreDelayMax=100;
        GlobalVars.PreDelayMin=10;
        GlobalVars.DPCountPre=2;
        GlobalVars.DPTHRESHPRE=2;
        
        
        GlobalVars.sampleBin=(GlobalVars.SamplingRate/2)/(GlobalVars.FFT/2);
        GlobalVars.template=(zeros(GlobalVars.FFT//2));        
        GlobalVars.templatepre=(zeros(GlobalVars.FFT//2)); 

        
        self.pretemplateView.plot(GlobalVars.templatepre,arange(0,GlobalVars.sampleBin*(GlobalVars.FFT/2),GlobalVars.sampleBin)/1000,pen=pg.mkPen('r', width=1))
        self.MagsfromTeensy.plot(GlobalVars.template,(arange(0,GlobalVars.sampleBin*128,GlobalVars.sampleBin))/1000,pen=pg.mkPen('r', width=1))

        #Turn of some buttons at initialization
        self.NFFTcomboBox.setEnabled(False);
        self.SampleRateComboBox.setEnabled(True);
        self.stopButton.setEnabled(False)

        #Connect all the buttons to their functions;
        self.serialScan.clicked.connect(GetTeensyPorts);
        self.startButton.clicked.connect(startButtonPressed);    
        self.stopButton.clicked.connect(stopButtonPressed);
        self.actionLoad_Config.triggered.connect(loadConfig_ButtonPressed);
        self.actionSave_Config.triggered.connect(saveConfig_ButtonPressed);
        self.editMINAMP.editingFinished.connect(editAMP);
        self.editMAXAMP.editingFinished.connect(editAMPMax);    
        self.editMINFF.editingFinished.connect(editFFMIN);
        self.editMAXFF.editingFinished.connect(editFFMAX);
        self.editFREQ_THRESH.editingFinished.connect(editFFreqThresh);
        self.editDP_Thresh.editingFinished.connect(editDPTHRESH);
        self.FileAndPath_PushButton.clicked.connect(SaveFileButtonPressed)
        self.WN_OncheckBox.clicked.connect(WN_OnPressed)
        self.HitAboveButton.clicked.connect(HitAboveButtonPressed)
        self.HitBelowButton.clicked.connect(HitBelowButtonPressed)
        self.uploadtemplateButton.clicked.connect(updateTemplate)
        
        self.DirFlagCheckBox.clicked.connect(updateDirFlag)
        self.upDateThresholdCheckBox.clicked.connect(upDateThresholdButtonPressed)
        self.ThresholdUpdateThresholdspinBox.valueChanged.connect(updateThreshholdPercent)
        self.DPCountSpinBox.valueChanged.connect(updateDPCount)
        self.DPCountSpinBoxPre.valueChanged.connect(updateDPCountSpinBoxPre)
        self.AmpCountSpinBox.valueChanged.connect(updateAMPCount)
        self.CatchPercentspinBox.valueChanged.connect(updateCatchPercent)
        self.ThresholdUpdateThresholdspinBox.setEnabled(False)
        self.BufferTimeSpinBox.valueChanged.connect(BufferTimeSpinBoxChanged)
        #self.MagsfromTeensy.setYLink(self.pretemplateView)
        self.AveragingSpinBox.valueChanged.connect(setAveraging)
        self.Teensy_USB_ComboBox.currentIndexChanged.connect(SetAudioIn)
        self.LineInDoubleSpinBox.valueChanged.connect(SetLineInGain)
        self.SampleRateComboBox.currentIndexChanged.connect(setSampleRate)
        self.TrackFFButton.clicked.connect(TrackFFButtonPressed);
        self.DP_Val.setText("Match Value: "+str(0));
        self.isPausedCheckBox.clicked.connect(updatePausing);
        self.templateShiftDownButton.clicked.connect(shiftTemplateDownButtonPressed)
        self.templateShiftUpButton.clicked.connect(shiftTemplateUpButtonPressed)
        self.editAUDIOAMPTHRESHOLD.editingFinished.connect(editUSBThreshold);
        self.PreTemplatecheckBox.clicked.connect(PreTemplatecheckBoxChecked);
        self.uploadtemplatepreButton.clicked.connect(uploadtemplatepreButtonPressed);
        self.editPreDelayMin.editingFinished.connect(editPreDelayMinPressed);
        self.editPreDelayMax.editingFinished.connect(editPreDelayMaxPressed);
        self.editDP_ThreshPre.editingFinished.connect(editDPTHRESHPrePressed);
    
 
if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    ui = MainWindow()
    ui.show()
    GetTeensyPorts()
    GlobalVars.UpDateValues(ui);
    sys.exit(app.exec_())
    window.show()

    sys.exit(app.exec_())
