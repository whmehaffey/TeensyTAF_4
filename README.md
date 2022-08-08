# TeensyTAF 4
Teensy v4 (pjrc.com)
https://www.pjrc.com/store/teensy40.html
and Teensy 4 Audio Shield
https://www.pjrc.com/store/teensy3_audio.html

with a small 100hz-(10-20)khz microphone based Target Audio Feedback (TAF) system. 
https://www.digikey.com/en/products/detail/cui-devices/CMEJ-4618-42-L010/10253438


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

PyQt5 (use GUIQt5)

pip -m install pyqt5


Without the PCB, 

Basically, wire up the audio output jack with the Audio Channel (Left from the Teensy Audio Shield) going to the left channel, Pin 9 on the Teensy going to the right, and the audio ground (next to the Left channel) going to the ground pin.
Then, for another (preferably mono, but you could also just just one (or both) channels of a stereo jack) audio jack, run the 'Right' channel off the audio shield to your amplifier.
Once you have that, plug the audio channel into the Line-In on the USB sound card, and see if you see sound (once you've uploaded the sketch to the Teensy).
I often use Audacity for this (https://www.audacityteam.org/) because you can stream audio live and see how it looks either as an oscillogram, or as a spectrogram. 

Either way you'll need to attach a microphone to Mic-In on the Teensy Audio Shield, something like:
https://www.digikey.com/en/products/detail/cui-devices/CMEJ-4618-42-L010/10253438
or
https://www.digikey.com/en/products/detail/cui-devices/CMC-6010-42L100/7398903

depending on species, since you may want higher than 10khz (or not)
From the original documentation of the orignal CBS (Catch Bird Singing, by David Perkel), 

"Also, I am not a programmer, so I don't want to hear complaints.  I know it
isn't written well.  However, I would love to hear suggestions, even things
that may seem obvious."
