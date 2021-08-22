# espI2sClockGen
A signal generator using AD9833 and esp8266

## Features
- Signal generation from 0.1Hz to 5MHz
- Sine, Triangle and square waves
- Amplitude set by digital potentiometer (0-127) and switched attenuators (0, -20dB, -26dB, -40dB)
- Can playback files holding sequence of frequencies and amplitudes
- Web based GUI allowing control from PC, phone, tablet
- Wifi Management to allow easy initial router set up
- OTA software update

## Build and use
- Install MD_AD9833 library
- Install BaseSupport library (https://github.com/roberttidey/BaseSupport)
- Needs WifiManager library
- Edit passwords in BaseConfig.h
- Compile and upload in Arduino environment
- Set up wifi network management by connecting to AP and browsing to 192.168.4.1
- upload basic set of files from data folder using STA ip/upload
- further uploads can then be done using ip/edit
- normal interface is at ip/



