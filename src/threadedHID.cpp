/*
 *  threadedHID.cpp
 *
 *  Copyright © 2014 Zurich University of the Arts. All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  3. The name of the author may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY [LICENSOR] "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 *  EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  @author Jan Schacher
 *  @@date 20140727
 *
 */

#include "ofApp.h"
#include "threadedHID.h"
#include "sabreKeys.h"

 //--------------------------------------------------------------
threadedHID::threadedHID()
{
//	TTF.loadFont("inconsolata.ttf", 11, 1, 1, 0);
	debounceTimeout = 0;

	scale10 = 1.0 / 1024.0;
	scale11 = 1.0 / 2048.0;
	scale12 = 1.0 / 4096.0;
	scale13 = 1.0 / 8192.0;
	scale16 = 1.0 / 65536.0;
	//	scale32 = 1.0 / 2147483648;
	
    // v3.4 comm structure
	//	accelResolution = 4;
	//	accelOffset = 1024;
	//	accelScale = scale11;
    // v3.5 comm structure
	accelResolution = 4;
	accelOffset = 32768;
	accelScale = scale16;
	
	gyroResolution = 16;
	gyroOffset = 32768;
	gyroScale = scale16;
	
	magnetoResolution = 12;
	magnetoOffset = 2048;
	magnetoScale = scale12;
	
	tempOffset = 13200;
	tempScale = scale16;
	
	headingLowpassFactor = 0.2;
	headingOld_x = headingOld_y = 0.0;
	
	//TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0);

	calibrateSwitch = 0;
	for(int i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
		calibrate[i] = 0;
	}
    
    batteryAddressMain = "/sabre/battery/main";
    batteryAddressAir = "/sabre/battery/air";
    
    timestampAddressLeft = "/sabre/timestamp/left";
    timestampAddressRight = "/sabre/timestamp/right";
    timestampAddressAir = "/sabre/timestamp/air";
    
    linkQualityAddressLeft = "/sabre/linkquality/left";
    linkQualityAddressRight = "/sabre/linkquality/right";
    linkQualityAddressAir = "/sabre/linkquality/air";
    
    airValue.calibrationFlag = 0;
    airValue.calibrationCounter = 0;
    airValue.calibrationValue = 0.0;
    
    raw[0] = 0;
    IMU[0] = 0.0f;
    raw[1] = 0;
    IMU[1] = 0.0f;
    raw[2] = 0;
    IMU[2] = 0.0f;
    
    raw[3] = 0;
    IMU[3] = 0.0f;
    raw[4] = 0;
    IMU[4] = 0.0f;
    raw[5] = 0;
    IMU[5] = 0.0f;
    
    raw[6] = 0;
    IMU[6] = 2147483648.0f;
    raw[7] = 0;
    IMU[7] = 2147483648.0f;
    raw[8] = 0;
    IMU[8] = 2147483648.0f;
    
    raw[9] = 0;
    IMU[9] = 0.0f;

    airLong[0] = 0;
    air[0] = 1000.0;
    airValue.range = 0.0f;

    button[0] = 0;
    button[1] = 0;
    button[2] = 0;

	batteryLevelRight = 0;
    batteryLevelAir = 0;

	linkQualityLeft = 0;
    linkQualityRight = 0;
    linkQualityAir = 0;

	airValue.calibrationFlag = true;
    airValue.calibrationValue = 0;
    airValue.calibrationCounter = 0;

	OSCprevTime = ofGetElapsedTimeMillis();
	OSCsenderOpen = false;

	keycode = 0;
	keycodeOld = 0;
	keycodeChanged = false;
    note = 67;
	validMidiNote = true;

	senderActive[0] = true;
    sendFullFrame = false; // TODO

//    dBuffer.clear();
}

//--------------------------------------------------------------
threadedHID::~threadedHID()
{
	if( isThreadRunning() ) { 
		stopThread();
	}
    if(rawHID.deviceOpen){
        rawHID.closeDevice();
    }
}

//--------------------------------------------------------------
void threadedHID::start()
{
	startThread(true);   // blocking
    oldSystimeL = oldSystimeR = ofGetElapsedTimeMicros();
	if (appDebug) printf("[threadedHID::start] starting thread... oldSystime = %d\n", oldSystimeL);
}

//--------------------------------------------------------------
void threadedHID::stop()
{
	if (appDebug) printf("[threadedHID::stop] stopping thread...\n");
	stopThread();
}

//--------------------------------------------------------------
void threadedHID::threadedFunction()
{
	while( isThreadRunning() != 0 )
	{
		haveInput[0] = 0;
		haveInput[1] = 0;
		haveInput[2] = 0;

		/* Receive some data packets from the HID device */
        int num = rawHID.receivePacket();
		
		/* On error, num = -1 */
		if (num < 0) {
			if(appDebug) printf("[threadedHID::threadedFunction]\n************************************\nerror reading, device went offline\n************************************\n");
			
			/* Notice that the device hase been unplugged in order to close *
			 * the socket and to stop the HID/OSCsender thread */
			rawHID.deviceUnplugged = true;
		}
		/* If num > 0 some packets have been received */
		else if (num > 0) {
			/* Truncate the lines from where there are only zeroes */
			if (appDebug) {
				for (int i = 0; i < SABRE_MAXPATTERNLEN; i++) {
					printf("%d\t", rawHID.buf[i]);
				}
				printf("\n");
			}

			/* Parse received message and fill the specific
			 * (key, button, air, imu, ...) buffers */
			HIDparse();
            
            OSCtime = ofGetElapsedTimeMillis();
            if(OSCtime >= (OSCprevTime + OSCsendingInterval) ) {
                
                for(int i = 0; i < OSC_NUMSENDERS; i++) {
                    if(senderActive[i]) {
                        if(i == resetID) {
                            sendOSC( i, true );
                        } else {
                            sendOSC( i, false );
                        }
                    }
                }
                OSCprevTime = OSCtime;
            }
		}
		#ifdef WIN32
		Sleep(1);
		#else
		usleep(1000);
		#endif
//		unlock();
   	}
}

//--------------------------------------------------------------
void threadedHID::HIDparse()
{
	int i;// j;
	//long sum;
	//char tempBuf[20];
	
//	dParserStartT = ofGetElapsedTimeMicros();	
//	dParserStopT = ofGetElapsedTimeMicros();
//	dParserSum += (dParserStopT - dParserStartT);
	
	/* Pattern matching */
	if (rawHID.buf[0] == SABRE_STARTBYTE) { // packet start marker
		/* Left-hand packet */
		if (rawHID.buf[1] == SABRE_ADDRESSBYTE_LEFT) {
			if (rawHID.buf[SABRE_PATTERNLEN_LEFT-1] == SABRE_STOPBYTE) { // packet stop marker
				for(i = 0; i < (SABRE_PATTERNLEN_LEFT-3); i++) { // collect n-2 bytes into buffer
					input[0][i] = rawHID.buf[i+2];
				}
//				printf("LEFT - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("LEFT; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dLhStartT = ofGetElapsedTimeMicros();
//				dLhParsing = true;
				haveInput[0] = true; // set input flag for left hand
				parseLeft(); // parse left-hand data
				calcKeycode(); // calculate keycode
			}
		}
		/* Right-hand packet */
		else if (rawHID.buf[1] == SABRE_ADDRESSBYTE_RIGHT) {
			if (rawHID.buf[SABRE_PATTERNLEN_RIGHT-1] == SABRE_STOPBYTE) { // packet stop marker
				for(i = 0; i < (SABRE_PATTERNLEN_RIGHT-3); i++) { // collect n-2 bytes into buffer
					input[1][i] = rawHID.buf[i+2];
				}
//				printf("RIGHT - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("RIGHT; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dRhStartT = ofGetElapsedTimeMicros();
//				dRhParsing = true;
				haveInput[1] = true; // set input flag for right hand
				parseRight(); // parse right-hand data
				parseIMU(); // parse IMU data separately
				calcKeycode(); // calculate keycode
			}
			
		}
		/* airMEMS packet */
		else if (rawHID.buf[1] == SABRE_ADDRESSBYTE_AIR) {
			if( rawHID.buf[SABRE_PATTERNLEN_AIR-1] == SABRE_STOPBYTE) { // oacket stop byte marker
				for(i = 0; i < (SABRE_PATTERNLEN_AIR-3); i++) { // collect n-2 bytes into buffer
					input[2][i] = rawHID.buf[i+2];
				}
//				printf("AM - serial parsing time: %lld us\n", dParserSum);
//				dBuffer.append("airMEMS; ");
//				sprintf(tempBuf, "%lld", dParserSum);
//				dBuffer.append(tempBuf);
//				dBuffer.append("; ");
//				dParserSum = 0;
//				dAmStartT = ofGetElapsedTimeMicros();
//				dAmParsing = true;
				haveInput[2] = true; // set input flag for airMEMS
				parseAir(); // parse airMEMS data
			}
		}
	}
}

//--------------------------------------------------------------
void threadedHID::parseLeft()
{	
	if(haveInput[0]) {
        /* Parse out the 13 left-hand keys and fill key[].raw buffer */
        keys[0].raw = input[0][0]; // LSBs
		keys[1].raw = input[0][1];
		keys[2].raw = input[0][2];
		keys[3].raw = input[0][3];
		keys[4].raw = input[0][4];
		keys[5].raw = input[0][5];
		keys[6].raw = input[0][6];
		keys[7].raw = input[0][7];
		keys[8].raw = input[0][8];
        keys[9].raw = input[0][9];
        keys[10].raw = input[0][10];
        keys[11].raw = input[0][11];
        keys[12].raw = input[0][12];
        
		keys[0].raw += (input[0][13] & 0xC0) << 2; // MSBs
		keys[1].raw += (input[0][13] & 0x30) << 4;
		keys[2].raw += (input[0][13] & 0xC) << 6;
        keys[3].raw += (input[0][13] & 0x3) << 8;
        keys[4].raw += (input[0][14] & 0xC0) << 2;
		keys[5].raw += (input[0][14] & 0x30) << 4;
		keys[6].raw += (input[0][14] & 0xC) << 6;
        keys[7].raw += (input[0][14] & 0x3) << 8;
        keys[8].raw += (input[0][15] & 0xC0) << 2;
		keys[9].raw += (input[0][15] & 0x30) << 4;
		keys[10].raw += (input[0][15] & 0xC) << 6;
        keys[11].raw += (input[0][15] & 0x3) << 8;
        keys[12].raw += (input[0][16] & 0xC0) << 2;
		
		/* Parse out the left thumb buttons */
#if (SABRE_INSTRUMENTNR == 1)
		button[0] = (input[0][16] & 0x10) >> 4;
		button[1] = (input[0][16] & 0x8) >> 3;
		button[2] = (input[0][16] & 0x20) >> 5;
#elif (SABRE_INSTRUMENTNR == 2)
		button[0] = (input[0][16] & 0x8) >> 3;
		button[1] = (input[0][16] & 0x10) >> 4;
		button[2] = (input[0][16] & 0x20) >> 5;
#elif (SABRE_INSTRUMENTNR == 3)
		button[0] = (input[0][16] & 0x10) >> 4;
		button[1] = (input[0][16] & 0x8) >> 3;
		button[2] = (input[0][16] & 0x20) >> 5;
#endif

		/* Parse out system data */
		timestampLeft = input[0][17] + (input[0][18] << 8);
		linkQualityLeft = input[0][19];

		/* ----------------------------------------------------- */

		/* Get current time for debouncing */
        int now = ofGetElapsedTimeMillis();

		/* Cook 13 left-hand keys */
		for (int i = 0; i < 13; i++) { // 13 keys
			/* Check for invert flag and modify keys behavious */
			if(keys[i].inverted) {
				keys[i].raw = 1023 - keys[i].raw;
			}
			/* If calibration set, set min & max values */
			if(calibrate[i]) {
				/* If current value < minimum -> set new minimum */
				if(keys[i].raw < keys[i].minimum){
					keys[i].minimum = (keys[i].raw + SERVER_CALIBRATEOFFSET);
				}
				/* If current value > maximum -> set new maximum */
				if(keys[i].raw > keys[i].maximum){
					keys[i].maximum = (keys[i].raw - SERVER_CALIBRATEOFFSET);
				}
				/* Set key range */
				if(keys[i].maximum != keys[i].minimum) {
					keys[i].range = 1.0 / (keys[i].maximum - keys[i].minimum);
				}
				/* Something went wrong, reset range */
				else {
					keys[i].range = 0.0;
				}
			}
			/* Calculate scaled value of a key and clamp between 0.0 and 1.0 */
			keys[i].scaled = (float)( keys[i].raw - keys[i].minimum ) * keys[i].range;
			keys[i].scaled = CLAMP(keys[i].scaled, 0.0, 1.0);
			/* Calculate key binary value */
			if(keys[i].scaled < keys[i].threshDown) {
				keys[i].binary = false;
			}
			else if (keys[i].scaled > keys[i].threshUp) {
				if(now - keys[i].lastTriggerTime > debounceTimeout) {
					keys[i].binary = true;
					keys[i].lastTriggerTime = now;
				}
			}
/* Redundancy check to reduce the amound of OSC data */
#ifdef SERVER_FILTERCHANGE
			/* Check if key raw value has changed (!!) */
			if(keys[i].raw != keys[i].rawOld) {
				keys[i].changed = true;
				keys[i].rawOld = keys[i].raw;
				/* Check if key binary value has changed */
				if(keys[i].binary != keys[i].binaryOld) {
					keys[i].binaryChanged = true;
					keys[i].binaryOld = keys[i].binary;
				}
			}
#endif
		}
        
		/* Check if button values have changed and set flags */
		for(int i = 0; i < 3; i++) {
			if(button[i] != buttonOld[i]) {
				buttonChanged[i] = true;
				buttonOld[i] = button[i];
			}
		}
        
        /* Calculate time difference since last left-hand parsing */
        systimeL = ofGetElapsedTimeMillis();
        deltaTimeL = systimeL - oldSystimeL;
        oldSystimeL = systimeL;
	}
}

//--------------------------------------------------------------
void threadedHID::parseRight()
{
	if(haveInput[1]) {
		/* Parse out the 12 right-hand keys and fill key[].raw buffer */
		keys[13].raw = input[1][0]; // LSBs
		keys[14].raw = input[1][1];
		keys[15].raw = input[1][2];
		keys[16].raw = input[1][3];
		keys[17].raw = input[1][4];
		keys[18].raw = input[1][5];
		keys[19].raw = input[1][6];
		keys[20].raw = input[1][7];
		keys[21].raw = input[1][8];
        keys[22].raw = input[1][9];
        keys[23].raw = input[1][10];
        keys[24].raw = input[1][11];
        
		keys[13].raw += ((input[1][12] & 0xC0) << 2); // MSBs
		keys[14].raw += ((input[1][12] & 0x30) << 4);
		keys[15].raw += ((input[1][12] & 0xC) << 6);
        keys[16].raw += ((input[1][12] & 0x3) << 8);
        keys[17].raw += ((input[1][13] & 0xC0) << 2);
		keys[18].raw += ((input[1][13] & 0x30) << 4);
		keys[19].raw += ((input[1][13] & 0xC) << 6);
        keys[20].raw += ((input[1][13] & 0x3) << 8);
        keys[21].raw += ((input[1][14] & 0xC0) << 2);
		keys[22].raw += ((input[1][14] & 0x30) << 4);
		keys[23].raw += ((input[1][14] & 0xC) << 6);
        keys[24].raw += ((input[1][14] & 0x3) << 8);
        
		/* Parse out system data */
		timestampRight = input[1][36] + (input[1][37] << 8);
		linkQualityRight = input[1][38];
		batteryLevelRight = input[1][35] & 0xF;

		/* ----------------------------------------------------- */

		/* Get current time for debouncing */
		int now = ofGetElapsedTimeMillis();

		/* Cook 12 right-hand keys */
		for (int i = 13; i < 25; i++) {
			/* Check for invert flag and modify keys behavious */
			if(keys[i].inverted) {
				keys[i].raw = 1023 - keys[i].raw;
			}
			/* If calibration set, set min & max values */
			if(calibrate[i]) {
				if(keys[i].raw < keys[i].minimum){
					keys[i].minimum = (keys[i].raw + SERVER_CALIBRATEOFFSET);
				}
				if(keys[i].raw > keys[i].maximum){
					keys[i].maximum = (keys[i].raw - SERVER_CALIBRATEOFFSET);
				}
				if(keys[i].maximum != keys[i].minimum) {
					keys[i].range = 1.0 / (keys[i].maximum - keys[i].minimum);
				} else {
					keys[i].range = 0.0;
				}
			}
			/* Calculate scaled value of a key and clamp between 0.0 and 1.0 */
			keys[i].scaled = (float)( keys[i].raw - keys[i].minimum ) * keys[i].range;
			keys[i].scaled = CLAMP(keys[i].scaled, 0.0, 1.0);
			//if(appDebug) printf("#%d-%03f ", (i + 1), keys[i].scaled);
			/* Calculate key binary value */
			if(keys[i].scaled < keys[i].threshDown) {
				keys[i].binary = false;
			}
			else if (keys[i].scaled > keys[i].threshUp) {
				if(now - keys[i].lastTriggerTime > debounceTimeout){
					keys[i].binary = true;
					keys[i].lastTriggerTime = now;
				}
			}
/* Redundancy check to reduce the amound of OSC data */
#ifdef SERVER_FILTERCHANGE
			/* Check if key raw value has changed (!!) */
			if(keys[i].raw != keys[i].rawOld) {
				keys[i].changed = true;
				keys[i].rawOld = keys[i].raw;
				/* Check if key binary value has changed */
				if(keys[i].binary != keys[i].binaryOld) {
					keys[i].binaryChanged = true;
                    keys[i].binaryOld = keys[i].binary;
				}
			}
#endif
		}
		//if(appDebug) printf("\n");

        // NOTE: IMU parsing is done in separate function using same input buffer
        
		/* Calculate time difference since last right-hand parsing */
		systimeR = ofGetElapsedTimeMillis();
		deltaTimeR = systimeR - oldSystimeR;
		oldSystimeR = systimeR;

	}
}

//--------------------------------------------------------------
void threadedHID::parseIMU()
{
	if(haveInput[1]) {
		/* Parse out single IMU sensors (v3.5 comm structure) */
		raw[0] = input[1][29] + (input[1][32] << 8); // accelerometer
		raw[1] = input[1][30] + (input[1][33] << 8);
		raw[2] = input[1][31] + (input[1][34] << 8);
				
		raw[3] = input[1][16] + (input[1][20] << 8); // gyroscope
		raw[4] = input[1][17] + (input[1][21] << 8);
		raw[5] = input[1][18] + (input[1][22] << 8);
		
		raw[9] = input[1][15] + (input[1][19] << 8); // temperature
		
		raw[6] = input[1][23] + (input[1][26] << 8); // magneto
		raw[7] = input[1][24] + (input[1][27] << 8);
		raw[8] = input[1][25] + (input[1][28] << 8);
		
		/* Cook IMU sensors - v3.5 comm structure (16 bit) */
		// accelerometer
		for(int i = 0; i < 3; i++) {
			// correct sign
			if (raw[i] >= 32768) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + accelOffset; // accelOffset = 0.5 * 2^16 = 32768
			IMU[i] = rawIMU[i] * accelScale; // accelScale = 16 bit (since v3.5)
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}
		// gyroscope
		for(int i = 3; i < 6; i++) {
			// correct sign
			if (raw[i] >= 32768) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + gyroOffset; // gyroOffset = 0.5 * 2^16 = 32768
			IMU[i] = rawIMU[i] * gyroScale; // gyroScale = 16 bit
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}
		// magnetometer
		for(int i = 6; i < 9; i++) {
			// correct sign
			if (raw[i] >= 32768) {
				raw[i] -= 65535;
			}
			rawIMU[i] = raw[i] + magnetoOffset; // magnetoOffset = 0.5 * 2^12 = 2048
			IMU[i] = rawIMU[i] * magnetoScale; // magnetoScale = 12 bit
			IMU[i] = CLAMP(IMU[i], 0.0 , 1.0);
		}
		// temperature
		int i = 9;
		// correct sign
		if( raw[i] >= 32768 ) {
			raw[i] -= 65535;
		}
		rawIMU[i] = raw[i] + tempOffset; // tempOffset = 13200 --> 35 °C
		IMU[i] = (rawIMU[i] / 280.) + 35.0; // sensitivity = 280 LSB/ °C
		
		/* Calculate the sums */
		summedIMU[0] = ( fabs(IMU[0] - 0.5) + fabs(IMU[1] - 0.5) + fabs(IMU[2] - 0.5) ) * 0.6666666666666666666666666; // accelerometer 
		summedIMU[1] = ( fabs(IMU[3] - 0.5) + fabs(IMU[4] - 0.5) + fabs(IMU[5] - 0.5) ) * 0.6666666666666666666666666; // gyroscope
		summedIMU[2] = ( fabs(IMU[6] - 0.5) + fabs(IMU[7] - 0.5) + fabs(IMU[8] - 0.5) ) * 0.6666666666666666666666666; // magnetometer
		
		/* Calculate heading & tilt */
		calcHeadingTilt();
	}
}

//--------------------------------------------------------------
void threadedHID::parseAir()
{
	if(haveInput[2]) {
		/* Parse out airMEMS data */
		airLong[0] = input[2][0] + (input[2][1] << 8) + (input[2][2] << 16) + (input[2][3] << 24) ; // pressure
		airLong[1] = input[2][4] + (input[2][5] << 8) + (input[2][6] << 16) + (input[2][7] << 24) ; // temperature
		/* Convert the long integers into a double °C realistic value */
		air[0] = ((double)(airLong[0] / 100.0)); // pressure
		air[1] = ((double)(airLong[1] / 100.0)); // temprature
        /* Parse out system data */
        batteryLevelAir = input[2][8] & 0xF; // battery level
        timestampAir = input[2][9] + (input[2][10] << 8); // timestamp
        linkQualityAir = input[2][11]; // link quality
        
		/* Collect the first atmospheric pressure value data *
		 * (index AIR_CALIBSTART to AIR_CALIBSTOP) to define *
		 * the offset (middle point) */
        if(airValue.calibrationFlag) {
            if(airValue.calibrationCounter == 0) {
                printf("start calibrating atmospheric pressure after %lld ms\n", ofGetElapsedTimeMillis());
            }
            if(airValue.calibrationCounter < AIR_CALIBSTART) {
                airValue.offset = air[0];
			}
			else {
				if (airValue.calibrationCounter < AIR_CALIBSTOP) {
					airValue.calibrationValue += air[0];
					// printf("air[0] calib %f\n", air[0]);
				}
				if (airValue.calibrationCounter >= AIR_CALIBSTOP) {
					airValue.offset = airValue.calibrationValue / (airValue.calibrationCounter - AIR_CALIBSTART);
					airValue.calibrationFlag = false; // lock up after you
					printf("airValue.offset after calibration is %f\n", airValue.offset);
					printf("finished calibrating atmospheric pressure after %lld ms\n", ofGetElapsedTimeMillis());
				}
			}
            airValue.calibrationCounter++;
        }
        
		/* Define pressure value relative to offset */
        airValue.relative = air[0] - airValue.offset;
        
		/* If range calibration set, look for min/max values */
        if(airValue.calibratePressureRange) {
			if (airValue.relative < airValue.minimum) { // look for minimum
                airValue.minimum = airValue.relative;
            }
			if (airValue.relative > airValue.maximum) { // look for maximum
                airValue.maximum = airValue.relative;
            }
			/* Set the range to 2x the greatest between min and max,c *
			 * in order to keep the offset in the middle */
			if (airValue.maximum > abs(airValue.minimum)) {
                airValue.range = ( 1.0 / airValue.maximum) * 0.5;
			}
			else if (airValue.maximum < abs(airValue.minimum)) {
				airValue.range = ( 1.0 / abs(airValue.minimum)) * 0.5;
			}
			else {
                airValue.range = 0.0; // something went wrong... resetting
            }
        }
        
		/* Calculate scaled pressure value to remain in the 0.0-1.0 range */
        airValue.scaled = airValue.relative * airValue.range + 0.5;
        airValue.scaled = CLAMP(airValue.scaled, 0.0, 1.0);
	}
}

//--------------------------------------------------------------
void threadedHID::calcKeycode()
{
	/* Keycode calculation */
	keycode = 0;
	for(int i = 0; i < SABRE_NUMKEYS; i++) {
		if(keys[i].binary == 1) {
			keycode += pow((double)2, (double)i);
		}
	}

/* Redundancy check to reduce the amount of sent OSC data */
#ifdef SERVER_FILTERCHANGE
	if (keycode != keycodeOld) {
		keycodeOld = keycode;
		keycodeChanged = true;
		/* Check if the calculated keycode corresponds to one MIDI code */
		validMidiNote = false;
		note = -1;
		for (int i = 0; i < SABRE_NUMMIDI; i++) {
			if (keycode == midiNote[i].keycode) {
				note = midiNote[i].note;
				validMidiNote = true;
				break;
			}
		}
	}
#endif
}

//--------------------------------------------------------------
void threadedHID::calcHeadingTilt()
{
	double x, y;
	double ax, ay, bx, by;
	
	/* 0g --> IMU[] = 0.5
	 * thus re-center values around 0
	 * and change angles to radian */
	x = (IMU[1] - 0.5) * DEG_TO_RAD;
	y = (IMU[2] - 0.5) * DEG_TO_RAD;
	
	// old = old + (new - old) * factor;
	x = headingOld_x + ((x - headingOld_x) * 0.2);
	y = headingOld_y + ((y - headingOld_y) * 0.2);
	
	headingOld_x = x;
	headingOld_y = y;
	
	ax = sin(x);
	ay = sin(y);
	
	bx = tan(x);
	bx = bx * bx;
	
	by = tan(y);
	by = by * by;
	
	heading = atan2(ax, ay) * RAD_TO_DEG;
	tilt = CLAMP( (atan( sqrt(bx + by) ) * RAD_TO_DEG * 8.0), 0.0, 1.0);
	//	tilt = atan( sqrt(bx + by) ) * RAD_TO_DEG * 10.0;
}

//--------------------------------------------------------------
void threadedHID::sendOSC(int ID, bool resetFlags)
{
    int i, j;
    //char tempBuf[20];
    
    /* Standard data-mode */
    if( (senderMode[ID] & 1) == 1) {
		
		//if(appDebug) printf("[threadedHID::sendOSC] sending OSC on %d with reset %d\n", ID, resetFlags);
		/* Get the latest timestamp */
		systemTimestamp = ofGetElapsedTimeMillis();
		
		/* TODO: send full dataframe */
        if(sendFullFrame) {
            m[0].clear();
            m[0].setAddress( "/sabre/dataframe" ); // timestamp server
            m[0].addStringArg( "begin" );
            sender[ID].sendMessage( m[0] );
        }
		
		/* Prepare timestamps & airMEMS */
        m[1].clear();
        m[1].setAddress( timestampAddressServer ); // timestamp server
        m[1].addIntArg( deltaTimeL );
        m[1].addIntArg( deltaTimeR );
        m[1].addIntArg( systemTimestamp );
        
        m[2].clear();
        m[2].setAddress( timestampAddressLeft ); // timestamp left
        m[2].addIntArg( timestampLeft );
		
        m[3].clear();
        m[3].setAddress( timestampAddressRight ); // timestamp right
        m[3].addIntArg( timestampRight );
        
        m[4].clear();
        m[4].setAddress( timestampAddressAir ); // timestamp airMEMS
        m[4].addIntArg( timestampAir );
        
        m[5].clear();
        m[5].setAddress( airaddresses[0] ); // airMEMS pressure
        m[5].addFloatArg( airValue.range);
        
        m[6].clear();
        m[6].setAddress( airaddresses[1] ); // airMEMS temperature
        m[6].addFloatArg( air[1]);
        
		/* Send first batch with system data */
        for(i = 1; i < 7; i++) {
            sender[ID].sendMessage( m[i] );
        }
		
		/* Prepare & send keycode & MIDI */
        if(keycodeChanged) { // keycode
            m[7].clear();
            m[7].setAddress( keycodeaddress );
            m[7].addIntArg( keycode );

			sender[ID].sendMessage( m[7] );
            if(resetFlags) {
                keycodeChanged = false;
            }

            if(validMidiNote) {
                m[8].clear();	// midinote derived from keycode
                m[8].setAddress(midinoteaddress);
                m[8].addIntArg(note);
				
				sender[ID].sendMessage( m[8] );
            }
        }
        
		/* Prepare & send buttons values */
        for(i = 9, j = 2; i < 12; i++, j--) { // buttons
            if(buttonChanged[j]) {
                m[i].clear();
                m[i].setAddress( buttonaddresses[j] );
                m[i].addIntArg( button[j] );
                if(resetFlags) {
                    buttonChanged[j] = false;
                }
                sender[ID].sendMessage( m[i] );
            }
        }
        
		/* Prepare & send binary key values */
        for(i = 12, j = 0; i < 37; i++, j++) { // binary key values
            if(keys[j].binaryChanged) {
                m[i].clear();
                m[i].setAddress( keys[j].oscaddress+"/down");
                m[i].addIntArg( keys[j].binary );
                if(resetFlags) {
                    keys[j].binaryChanged = false;
                }
                sender[ID].sendMessage( m[i] );
                
            }
        }
        
		/* Prepare & send scaled key values */
        for(i = 37, j = 0; i < 62; i++, j++) { // scaled key values
            if(keys[j].changed) {
                m[i].clear();
                m[i].setAddress( keys[j].oscaddress+"/continuous");
                m[i].addFloatArg( keys[j].scaled );
                if(!sendRawValues && resetFlags) {
                    keys[j].changed = false;
                }
                
                sender[ID].sendMessage( m[i] );
            }
        }
        
		/* If demanded, prepare & send raw key values */
        if(sendRawValues) {
            for(i = 62, j = 0; i < 87; i++, j++) { // raw key values
                if(keys[j].changed) {
                    m[i].clear();
                    m[i].setAddress( keys[j].oscaddress+"/raw");
                    m[i].addIntArg( keys[j].raw );
                    if(resetFlags) {
                        keys[j].changed = false;
                    }
                    sender[ID].sendMessage( m[i] );
                }
            }
        }
		
		/* Prepare IMU values */		
        m[87].clear();
        m[87].setAddress( imuaddresses[0] ); // IMU accelero scaled
        m[87].addFloatArg( IMU[0] );
        m[87].addFloatArg( IMU[1] );
        m[87].addFloatArg( IMU[2] );
        
        m[88].clear();
        m[88].setAddress( imuaddresses[1] ); // IMU gyro scaled
        m[88].addFloatArg( IMU[3] );
        m[88].addFloatArg( IMU[4] );
        m[88].addFloatArg( IMU[5] );
        
        m[89].clear();
        m[89].setAddress( imuaddresses[2] ); // IMU magneto scaled
        m[89].addFloatArg( IMU[6] );
        m[89].addFloatArg( IMU[7] );
        m[89].addFloatArg( IMU[8] );
        
        m[90].clear();
        m[90].setAddress( imuaddresses[6] ); // IMU accelero summed
        m[90].addFloatArg( summedIMU[0] );
        
        m[91].clear();
        m[91].setAddress( imuaddresses[7] ); // IMU gyro summed
        m[91].addFloatArg( summedIMU[1] );
        
        m[92].clear();
        m[92].setAddress( imuaddresses[8] ); // IMU magneto summed
        m[92].addFloatArg( summedIMU[2] );
        
        m[93].clear();
        m[93].setAddress( imuaddresses[10] ); // IMU heading from accelerometer
        m[93].addFloatArg( heading );
        
        m[94].clear();
        m[94].setAddress( imuaddresses[11] ); // IMU tilt from accelerometer
        m[94].addFloatArg( tilt );
        
        m[95].clear();
        m[95].setAddress( imuaddresses[9] ); // IMU temperature in degreee celsius
        m[95].addFloatArg( IMU[9] );
        
		/* Send IMU */
        for(i = 87; i < 96; i++) {
            sender[ID].sendMessage( m[i] );
        }

		/* If demanded, prepare & send raw IMU values */
		if(sendRawValues) {
            m[96].clear();
            m[96].setAddress( imuaddresses[3] ); // IMU accelero raw
            m[96].addFloatArg( rawIMU[0] );
            m[96].addFloatArg( rawIMU[1] );
            m[96].addFloatArg( rawIMU[2] );
            
            m[97].clear();
            m[97].setAddress( imuaddresses[4] ); // IMU gyro raw
            m[97].addFloatArg( rawIMU[3] );
            m[97].addFloatArg( rawIMU[4] );
            m[97].addFloatArg( rawIMU[5] );
            
            m[98].clear();
            m[98].setAddress( imuaddresses[5] ); // IMU magneto raw
            m[98].addFloatArg( rawIMU[6] );
            m[98].addFloatArg( rawIMU[7] );
            m[98].addFloatArg( rawIMU[8] );
            
            for(i = 96; i < 99; i++) {
                sender[ID].sendMessage( m[i] );
            }
        }
        
		/* Prepare system data */
        m[99].clear();
        m[99].setAddress( batteryAddressAir ); // air battery
        m[99].addIntArg( batteryLevelAir);
        
        m[100].clear();
        m[100].setAddress( linkQualityAddressAir ); // air link quality
        m[100].addIntArg( linkQualityAir);
		
        m[101].clear();
        m[101].setAddress( linkQualityAddressLeft ); // left link quality
        m[101].addIntArg( linkQualityLeft );
        
        m[102].clear();
        m[102].setAddress( linkQualityAddressRight ); // right link quality
        m[102].addIntArg( linkQualityRight );
        
        m[103].clear();
        m[103].setAddress( batteryAddressMain ); // battery level main
        m[103].addIntArg( batteryLevelRight );
        
		/* Send system data */
        for(i = 99; i < 104; i++) {
            sender[ID].sendMessage( m[i] );
        }
        
		/* TODO: close the full dataframe */
        if(sendFullFrame) {
            m[104].clear();
            m[104].setAddress( "/sabre/dataframe" ); // timestamp server
            m[104].addStringArg( "end" );
            sender[ID].sendMessage( m[104] );
        }
    }
    
	/* Gerhard Eckels direct SuperCollider server mode */
    if( ((senderMode[ID] >> 4) & 1) == 1){
        m[0].clear();
        m[0].setAddress( "/c_setn" ); // SC server command
        m[0].addIntArg( 0 ); // SC server Bus Nr.
        m[0].addIntArg( 26 ); // SC server NumArgs
		
        if(sendRawValues == 0){
            m[0].addFloatArg( airValue.range);
            for(i = 0; i < 25; i++){
                m[0].addFloatArg( keys[i].scaled );
            }
        }else if(sendRawValues == 1){
            m[0].addFloatArg( airValue.relative);
            for(i = 0; i < 25; i++){
                m[0].addIntArg( keys[i].raw );
            }
        }
        sender[ID].sendMessage( m[0] );
    }

    /*
    if(dLhParsing) {
        dLhStopT = ofGetElapsedTimeMicros();
        printf("LH time: %01.03f ms\n\n", ((float)(dLhStopT - dLhStartT)/1000));
        sprintf(tempBuf, "%lld", (dLhStopT-dLhStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dLhStartT = 0;
        dLhStopT = 0;
        dLhParsing = false;
    }
    if(dRhParsing) {
        dRhStopT = ofGetElapsedTimeMicros();
        printf("RH time: %01.03f ms\n\n", ((float)(dRhStopT - dRhStartT)/1000));
        sprintf(tempBuf, "%lld", (dRhStopT-dRhStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dRhStartT = 0;
        dRhStopT = 0;
        dRhParsing = false;
    }
    if(dAmParsing) {
        dAmStopT = ofGetElapsedTimeMicros();
        printf("aM time: %01.03f ms\n\n", ((float)(dAmStopT - dAmStartT)/1000));
        sprintf(tempBuf, "%lld", (dAmStopT-dAmStartT));
        dBuffer.append(tempBuf);
        dBuffer.append(";\n");
        dAmStartT = 0;
        dAmStopT = 0;
        dAmParsing = false;
    }
    */
    return;
}

//--------------------------------------------------------------
void threadedHID::calcResetID()
{
    resetID = -1;
    for(int i = 0; i < OSC_NUMSENDERS; i++) {
        if(senderActive[i] == 1){
            resetID = i;
        }
    }
    if(appDebug) printf("[threadedHID::calcResetID] resetID is %d\n", resetID);
}