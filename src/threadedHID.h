#pragma once
/****************************************************************************
 * Copyright (c) 2016 Zurich University of the Arts. All Rights Reserved.
 *
 * This file is part of sabreServer
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY [LICENSOR] "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * \file threadedHID.h
 
 * \author Jan Schacher
 * \author Sebastien Schiesser
 
 * \date 2 March 2016
 
 * \version 0.99
 
 */


/* --------------------------------------------------------------
 *     INCLUDES
 * -------------------------------------------------------------- */
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"
#include "ofxRawHID.h"

#include "sabreKeys.h"
#include "sabreAir.h"

/* --------------------------------------------------------------
 *     MACROS
 * -------------------------------------------------------------- */
/* OSC */
#define OSC_FRAMELENGTH 105
#define OSC_NUMSENDERS 4 // number of OSC sender to use in parallel

/* SABRe communication */
#define SABRE_MAXNUMMESSAGES 64 // maximum number messages to allocate
#define SABRE_STARTBYTE 0x41 // communication protocol start byte
#define SABRE_STOPBYTE 0x5A // communication protocol stop byte
#define SABRE_ADDRESSBYTE_LEFT 0xF0  // communication protocol address byte 1 (left hand)
#define SABRE_ADDRESSBYTE_RIGHT 0xF1  // communication protocol address byte 2 (right hand)
#define SABRE_ADDRESSBYTE_AIR 0xF2  // communication protocol address byte 3 (airMEMS)
#define SABRE_PATTERNLEN_LEFT 23 // number of bytes in a left hand message
#define SABRE_PATTERNLEN_RIGHT 42 // number of bytes in a right hand message
#define SABRE_PATTERNLEN_AIR 15 // number of bytes in a airMEMS message
#define SABRE_MAXPATTERNLEN SABRE_PATTERNLEN_RIGHT // maximum number of bytes in a SABRe message

/* SABRe IMU */
#define SABRE_IMU_ACCELXPOS 1 // position in the raw input array of the x acceleration value (IMU[0])
#define SABRE_IMU_ACCELYPOS 2 //       -               "                -                    (IMU[1])
#define SABRE_IMU_ACCELZPOS 0 //       -               "                -                    (IMU[2])
#define SABRE_IMU_ACCELXSIGN 0 // flags to determine if the IMU axes are built in reverse direction to the SABRe coordinates
#define SABRE_IMU_ACCELYSIGN 1
#define SABRE_IMU_ACCELZSIGN 1
#define SABRE_IMU_REFRESHFACTOR 0.2 // Ponderation factor to reduce jittering on heading & tile values
#define SABRE_IMU_ANGLEOFFSET 59 // set offset angle (in ¡) to get the heading 0/±180 along the player/neck/bell direction

/* SABRe keys & buttons */
#define SERVER_FILTERCHANGE // comment out in order to build without the redundancy check
#define SERVER_CALIBRATEOFFSET 15 // value to add/remove to calibrated max/min to avoid key flattering at rest
#define SABRE_INSTRUMENTNR 3 /* instrument identification for correct button parsing
                                #1 -> first ICST instrument that went to Graz
                                #2 -> Matthias' instrument
                                #3 -> second ICST instrument */

/* Application */
#define THREAD_STOPSLEEP_US 10000 // sleep time (us) after thread stop to avoid error messages


/* --------------------------------------------------------------
 *     CLASS DECLARATIONS
 * -------------------------------------------------------------- */
class threadedHID : public ofThread
{
public:
	threadedHID();
	~threadedHID();
	
	void start();
	void stop();
	void threadedFunction();
    void HIDparse();
	void parseLeft();
	void parseRight();
	void parseIMU();
	void parseAir();
	void calcKeycode();
	void calcHeadingTilt();
	void sendOSC(int ID, bool resetFlags);
	void calcLastSender();

	ofxRawHID rawHID; ///< wrapping class to the hidapi functions
	unsigned char hidBuf[64];

	ofxXmlSettings	XML; ///< XML settings variables
	string str1;

	/// OSCsender (!!) variables
	ofxOscSender sender[OSC_NUMSENDERS]; // the OSC senders...
    bool senderActive[OSC_NUMSENDERS]; // flag for active/non-active senders
    int senderMode[OSC_NUMSENDERS]; // OSC sender mode. Currently 2 modes available: 1 (normal mode) and 16 (c_setn mode for IEM Graz)
    string sendIP[OSC_NUMSENDERS]; // OSC sender IP address
	int sendport[OSC_NUMSENDERS]; // OSC sender UDP port
    int lastOSCSender; // number of the last active OSC sender
	bool OSCsenderOpen; // flag for OSC sender status
    ofxOscMessage m[OSC_FRAMELENGTH]; // static amount of messages in one dataframe
	int OSCsendingInterval; // OSC time interval for sending data
	long OSCtime, OSCprevTime; // OSC time interval counting variables
	//int OSCcounter;
	int numOSCloops; // TBD...
	bool sendFullFrame; // toggle whole sensor values OSC sending
	bool sendRawValues; // toggle raw values OSC sending (bandwidth management)
    int oldSendTS;
    int sendDelta;
    
	/// OSC sender addresses.
	string imuaddresses[12];
	string buttonaddresses[3];
	string airaddresses[2];
	string timestampAddressServer;
	string timestampAddressLeft;
	string timestampAddressRight;
	string timestampAddressAir;
	string keycodeaddress;
	string midinoteaddress;
	string headingaddress;
	string batteryAddressMain;
	string batteryAddressAir;
	string linkQualityAddressLeft;
	string linkQualityAddressRight;
	string linkQualityAddressAir;

	unsigned char input[3][SABRE_PATTERNLEN_RIGHT];    // working buffer for each packet type
	bool haveInput[3];				// flags to signal a full packet was parsed successfully
    
	/* --------------------------
	 * SABRe-specific variables
	 * -------------------------- */
	sabreKeys keys[SABRE_MAXNUMMESSAGES];
	sabreMidiNote midiNote[128];
    sabreAir airValue; 

	int numKeyAddr;
	int numImuAddr;
	int numButtonAddr;
	int numAirAddr;
	// data reception buffers
	int raw[10];
	long rawIMU[10];
	double IMU[10];
	double summedIMU[3];
	long airLong[2];
	unsigned long airULong[2];
	double air[2];
	bool button[3];
	bool buttonOld[3];
	bool buttonChanged[3];
	// calculation values
	double heading;
	double tilt;
	double headingOld_x;
	double headingOld_y;
	double headingLowpassFactor;
	// SABRe timestamps
	long timestampLeft;
	long timestampRight;
	long timestampAir;
	// battery level
	int batteryLevelRight;
	int batteryLevelAir;
    int batteryLevel;
	// radio transducer link quality
	int linkQualityLeft;
	int linkQualityRight;
	int linkQualityAir;
    int linkQuality;
	// key calibration buffers
	int keymin[SABRE_MAXNUMMESSAGES];
	int keymax[SABRE_MAXNUMMESSAGES];
	bool keyInverted[SABRE_MAXNUMMESSAGES];
	// IMU scaling values
	double scale10;
	double scale11;
	double scale12;
	double scale13;
	double scale16;
	double scale32;
	// key threshold values
	double threshDown;
	double threshUp;
	// accelerometer-specific values
	int accelResolution;
    int accelRange;
	long accelOffset;
	double accelScale;
	// gyroscope-specific values
	int gyroResolution;
	long gyroOffset;
	double gyroScale;
	// magnetometer-specific values
	int magnetoResolution;
	long magnetoOffset;
	double magnetoScale;
	// temperature sensor (on gyro) values
	long tempOffset;
	double tempScale;
	// calibration values
	bool calibrateSwitch;
	bool calibrateSingle;
	bool calibrate[SABRE_MAXNUMMESSAGES];
	double lowThresh;
	double highThresh;
	long debounceTimeout; // debouncing timeout value (in ms)

	int displaySpeed;
	int display;
	// keycode & MIDI values
	long keycode;
	long keycodeOld;
	bool keycodeChanged;
	int note;
	bool validMidiNote;
	// computer system time
	long systimeL;
	long systimeR;
	long oldSystimeL;
	long oldSystimeR;
	long deltaTimeL;
	long deltaTimeR;
	long systemTimestamp;
    long systemTimestampBase;
	    
	// draw flag
    bool drawValues;
    
    // debug variables
	/*
    bool dLhParsing, dRhParsing, dAmParsing;
    unsigned long long dParserStartT, dParserStopT, dParserSum;
    unsigned long long dLhStartT, dLhStopT;
    unsigned long long dRhStartT, dRhStopT;
    unsigned long long dAmStartT, dAmStopT;
    ofBuffer dBuffer;
	*/
};
