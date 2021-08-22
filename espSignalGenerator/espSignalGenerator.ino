//signal generator using ads9833
// R.J.Tidey 07/07/2021

#define ESP8266
#include <arduino.h>
#include "BaseConfig.h"
#include "MD_AD9833.h"
#include <SPI.h>
#include <Wire.h>
#define DATA_EXT				".dat"


String playFile;
String tempCaptureFile = "tempCapture.dat";
int dataRecord = 0;
unsigned long dataStartTime;
unsigned long nextPlayTime;
int dataFileCount;
int playPosition = -1;

// Pins for SPI comm with the AD9833 IC
#define FSYNC 2	
//SDATA 13
//SCLK  14
#define ATTENUATE1 12
#define ATTENUATE2 15

#define DIGITALPOT_ADDR 0x2e
//#define I2C_D 4
//#define I2C_C 5

MD_AD9833	AD(FSYNC);  // Hardware SPI

void setMode(int mode) {
	switch(mode) {
		case 0: AD.setMode(MD_AD9833::MODE_OFF); break;
		case 1: AD.setMode(MD_AD9833::MODE_SINE); break;
		case 2: AD.setMode(MD_AD9833::MODE_SQUARE1); break;
		case 3: AD.setMode(MD_AD9833::MODE_SQUARE2); break;
		case 4: AD.setMode(MD_AD9833::MODE_TRIANGLE); break;
	}
}

void setAmplitude(int amplitude) {
	digitalWrite(ATTENUATE1, (amplitude & 0x100) ? 0 : 1);
	digitalWrite(ATTENUATE2, (amplitude & 0x200) ? 0 : 1);
	Wire.beginTransmission(DIGITALPOT_ADDR);
	Wire.write(0);
	Wire.write(amplitude & 0x7f);
	Wire.endTransmission();
}

//check to see if play a record, updates position in file
int processPlayRecord() {
	if((millis() - dataStartTime) > nextPlayTime) {
		if(playPosition == 0) {
			Serial.println("Playing data file " + playFile);
		}
		File f;
		String line;
		String pars;
		int j = 0;
		f = FILESYS.open("/" + playFile, "r");
		if(f) {
			f.seek(playPosition);
			if(f.available()) {
				line =f.readStringUntil('\n');
				line.replace("\r","");
				if(line.length() > 0 && line.charAt(0) != '#') {
					int i, val;
					int lastIndex = 0;
					int index = 0;
					for(j = 0; (j < 5) && (lastIndex >= 0); j++) {
						pars = "";
						index = line.indexOf(',', lastIndex);
						if(index > 0) {
							pars = line.substring(lastIndex, index);
							lastIndex = index + 1;
						} else {
							pars = line.substring(lastIndex);
							lastIndex = -1;
						}
						if(pars != "") {
							switch(j) {
								case 0: nextPlayTime = pars.toInt(); break;
								case 1: AD.setFrequency(MD_AD9833::CHAN_0, pars.toFloat()); break;
								case 2: AD.setPhase(MD_AD9833::CHAN_0, pars.toInt()); break;
								case 3: setMode(pars.toInt()); break;
								case 4: setAmplitude(pars.toInt()); break;
							}
						}
					}
				}
				playPosition = f.position();
				f.close();
			} else {
				playPosition = -1;
				f.close();
			}
		} else {
			playPosition = -1;
		}
	} else {
		return playPosition;
	}
}

//action getdataFiles
void handleGetDataFiles() {
	String fileList;
	String filename;
	Dir dir = FILESYS.openDir("/");
	int i;
	dataFileCount = 0;
	while (dir.next()) {
		filename = dir.fileName();
		i = filename.indexOf(DATA_EXT);
		if(i > 0 &&  i == (filename.length()-4)) {
			fileList += filename.substring(1) + "<BR>";
			dataFileCount++;
		}
	}
	server.send(200, "text/html", fileList);
}

void handleSetFreq() {
	float freq = server.arg("freq").toFloat();
	AD.setFrequency(MD_AD9833::CHAN_0, freq);
	server.send(200, "text/html", "Done");
}

void handleSetPhase() {
	int phase = server.arg("phase").toInt();
	AD.setPhase(MD_AD9833::CHAN_0, phase);
	server.send(200, "text/html", "Done");
}

void handleSetMode() {
	int mode = server.arg("mode").toInt();
	if(mode == -1) {
		setMode(0);
	} else if(mode) {
		setMode(mode);
	}
	server.send(200, "text/html", "Done");
}

void handleSetAmplitude() {
	int amplitude = server.arg("amplitude").toInt();
	if(amplitude == -1) {
		setAmplitude(0);
	} else if(amplitude) {
		setAmplitude(amplitude);
	}
	server.send(200, "text/html", "Done");
}

void handleSetAll() {
	float freq = server.arg("freq").toFloat();
	AD.setFrequency(MD_AD9833::CHAN_0, freq);
	int phase = server.arg("phase").toInt();
	if(phase) AD.setPhase(MD_AD9833::CHAN_0, phase);
	int mode = server.arg("mode").toInt();
	if(mode == -1) {
		setMode(0);
	} else if(mode) {
		setMode(mode);
	}
	int amplitude = server.arg("amplitude").toInt();
	if(amplitude == -1) {
		setAmplitude(0);
	} else if(amplitude) {
		setAmplitude(amplitude);
	}
	server.send(200, "text/html", "Done");
}

void handlePlayDataFile() {
	playFile = server.arg("playfile");
	playPosition = 0;
	dataStartTime = millis();
	nextPlayTime = 0;
	server.send(200, "text/html", "Done");
}

void setupStart() {
}

// add any extra webServer handlers here
void extraHandlers() {
	server.on("/setfreq", handleSetFreq);
	server.on("/setphase", handleSetPhase);
	server.on("/setmode", handleSetMode);
	server.on("/setamplitude", handleSetAmplitude);
	server.on("/setall", handleSetAll);
	server.on("/getdatafiles", handleGetDataFiles);
	server.on("/playdatafile", handlePlayDataFile);
}

void setupEnd() {
	AD.begin();
	Wire.begin();
	digitalWrite(ATTENUATE1, 0);
	digitalWrite(ATTENUATE2, 0);
	pinMode(ATTENUATE1, OUTPUT);
	pinMode(ATTENUATE2, OUTPUT);
	setAmplitude(0x33f); //50%
}

void loop() {
	server.handleClient();
	wifiConnect(1);
	if(playPosition >= 0) {
		processPlayRecord();
	}
	delaymSec(10);
}
