# PyCBS
Teensy v4 (pjrc.com)
https://www.pjrc.com/store/teensy40.html
and Teensy 4 Audio Shield
https://www.pjrc.com/store/teensy3_audio.html

with a small 100hz-20khz microphone



based Target Audio Feedback (TAF) system. 

Interface PCB @
https://oshpark.com/shared_projects/7dglGlFl

Parts list for Interface PCB is in .xls file included.

You will also need a stereo audio card- if your computer is mono, there are stereo USB cards available.
such as:
https://www.amazon.com/StarTech-com-Sound-SPDIF-Digital-Stereo/dp/B00F7120TQ/
or
https://www.amazon.com/VAlinks-External-Surround-Recording-Compatible/dp/B075F5VYG7/



Includes python GUI. 

Using:
pyserial
pyaudio (use https://www.lfd.uci.edu/~gohlke/pythonlibs/#pyaudio	with ALSA support. )
PyQtGraph
numpy

PyQt4:
https://www.lfd.uci.edu/~gohlke/pythonlibs/#pyqt4


or PyQt5 (use GUIQt5)

pip -m install pyqt5



From the original documentation of the orignal CBS (Catch Bird Singing, by David Perkel), 

"Also, I am not a programmer, so I don't want to hear complaints.  I know it
isn't written well.  However, I would love to hear suggestions, even things
that may seem obvious."
