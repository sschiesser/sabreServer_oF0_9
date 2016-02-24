/*
*  ofApp.cpp
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
*  @date 20140727
*
*  @author Sebastien Schiesser
*  @date 20160225
*/

#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
	appVersion = SERVER_VERSION;
	titleString = "sabreServer version " + appVersion; // Text to display on the server title bar
	if (appDebug) {
		printf("************************************\n");
		printf("**   sabreServer version %s    **\n", appVersion.c_str());
		printf("************************************\n\n");
	}

	/* Create a new threaded HID object, which contains:
	 * - rawHID -> the wrapping object for all hidapi function
	 * - ofThread functions and some calculation of received data
	 * - sendOSC -> the embedded OSC sender to avoid variable conflicts
	 *          (note that a OSC receiver is implemented as separate thread)
	 * - all SABRe variables (HID and OSC)
	 * - classes sabreKeys, sabreAir, sabreMidiNote
	 * - some flags to communicate with the main application
	 */
	rawHIDobject = new(threadedHID); // create a new threaded HID object

	/* GUI initialization */
	ofSetEscapeQuitsApp(false); // disable ESC button to escape application
	ofEnableAlphaBlending(); // turn on alpha blending
	TTF.loadFont("lucidagrande.ttf", 8, 1, 1, 0); // load font (must be in 'data' folder)
	TTFsmall.loadFont("lucidagrande.ttf", 8, 1, 0, 0);
	texScreen.allocate(440, 700, GL_RGB); // allocate a texture to the given dimensions
    ofSetWindowTitle(titleString); // set window title
	windowChanged = true; // flag to activate a window refresh
	drawValues = true; // flag to activate a values redraw
	rawHIDobject->drawValues = 0; // ?
	hiddenValues = false; // flag to tell if the monitor values are hidden

	GUIdeviceInfo = "RawHID device: --"; // Text to display on the device info



	receiveport = 41001;
	rawHIDobject->debounceTimeout = 0;

	/* Initialize OSC senders */
	for (int i = 0; i < OSC_NUMSENDERS; i++) {
		rawHIDobject->sendIP[i] = "127.0.0.1";
		rawHIDobject->sendport[i] = 40002 + i;
		rawHIDobject->senderActive[i] = false;
		rawHIDobject->senderMode[i] = 1;
	}

	/* Read XML preference file */
	//status = readPrefs();
	if (readPrefs()) {
		GUIfIOstatus = "Success reading settings in \"sabreServer.xml\"";
	}
	else {
		GUIfIOstatus = "Failed reading settings in \"sabreServer.xml\"";
	}
	if (appDebug) printf("[ofApp::setup] %s\n", GUIfIOstatus.c_str());
	/* Read MIDI notes file */
	//status = readMidicodes();
	if (readMidicodes()) {
		GUIfIOstatus = "Success reading MIDI codes in \"sabreMidicodes.xml\"";
	}
	else {
		GUIfIOstatus = "Failed reading MIDI codes in \"sabreMidicodes.xml\"";
	}
	if (appDebug) printf("[ofApp::setup] %s\n", GUIfIOstatus.c_str());
	//    dumpPrefs();

	lastTime = ofGetElapsedTimef();

	receiver.setup(receiveport);

	//	framerate = 20;
	ofSetFrameRate(framerate); // cap the glut callback rate
							   //	ofSetVerticalSync( true );
							   //	ofBackground( 224, 224, 224);
	ofBackground(255, 255, 255);
	redrawFlag = 1;
	// redrawInterval = redrawValues[display]; // in seconds
	firstflag = 1;

	runOnce = 1;
	runOnceDelay = 2.0f;
	lastRedraw = runOnceStart = ofGetElapsedTimef();
	
	ofSetWindowPosition(0, 44);

	/* STARTING APPLICATION */
	getHIDDeviceList();
	if (selectHIDdevice()) {
		if(appDebug) printf("[ofApp::setup] Device connected\n");
		rawHIDobject->rawHID.deviceSelected = true;
		rawHIDobject->rawHID.deviceUnplugged = false;
	}
	else {
		if(appDebug) printf("[ofApp::setup] No device found\n");
		rawHIDobject->rawHID.deviceSelected = false;
	}
	/* Make sure that the HID device and the HID/OSCsender thread are closed/stopped */
	stopHID();

	/* template code */
	//parameters.setName("parameters");
	//parameters.add(size.set("size",10,1,100));
	//parameters.add(number.set("number",10,1,100));
	//parameters.add(check.set("check",false));
	//parameters.add(color.set("color",ofColor(127),ofColor(0,0),ofColor(255)));
	//gui.setup(parameters);
	//// by now needs to pass the gui parameter groups since the panel internally creates it's own group
	//sync.setup((ofParameterGroup&)gui.getParameter(),6667,"localhost",6666);
	//ofSetVerticalSync(true);
	/* template code */
}

//--------------------------------------------------------------
void ofApp::getHIDDeviceList()
{
	if (appDebug) {
		printf("[ofApp::getHIDDeviceList] Listing plugged HID devices...\n");
	}
	/* List and get the number of plugged HID devices */
	int ret = rawHIDobject->rawHID.listDevices();
	if (ret > 0) {
		if (appDebug) {
			printf("[ofApp::getHIDDeviceList] %d UNIQUE devices found!\n", ret);
			//vector<ofxHIDDeviceInfo>::iterator it;
			//for (it = rawHIDobject->rawHID.HIDdevices.begin(); it < rawHIDobject->rawHID.HIDdevices.end(); it++) {
			//	printf("[ofApp::getHIDDeviceList] Device:  0x%04X  0x%04X   %ls  %ls\n", it->vendor_id, it->product_id, it->manufacturer_string, it->product_string);
			//}
		}
	}
	else {
		if (appDebug) {
			printf("[ofApp::getHIDDeviceList] no device found\n");
		}
	}
}

//--------------------------------------------------------------
bool ofApp::selectHIDdevice() {
	vector<ofxHIDDeviceInfo>::iterator it; // iterator on all the HIDdevices vector
	bool retVal = false; // return value (success)

	/* Convert the constant strings to int for comparison purposes */
	int sVID = strtol(SABRE_VENDORID, NULL, 16);
	int sPID = strtol(SABRE_PRODUCTID, NULL, 16);
	/* Convert the 8-bit strings to wchar_t for comparison purposes */
	string tMan = SABRE_MANUFACTURERSTRING;
	wstringstream wssMan;
	wssMan << tMan.c_str();
	wstring sManName = wssMan.str();
	string tProd = SABRE_PRODUCTSTRING;
	wstringstream wssProd;
	wssProd << tProd.c_str();
	wstring sProdName = wssProd.str();

	/* Look if one device int the HIDdevices list matches all criteria. If success,
	 * save the necessary information into the selectedDeviceInfo variable */
	for (it = rawHIDobject->rawHID.HIDdevices.begin(); it < rawHIDobject->rawHID.HIDdevices.end(); it++) {
		if (it->vendor_id == sVID) {
			if(it->product_id == sPID) {
				if (wcscmp(it->manufacturer_string, sManName.c_str()) == 0) {
					if (wcscmp(it->product_string, sProdName.c_str()) == 0) {
						rawHIDobject->rawHID.selectedDeviceInfo.vendor_id = sVID;
						rawHIDobject->rawHID.selectedDeviceInfo.product_id = sPID;
						wcscpy(rawHIDobject->rawHID.selectedDeviceInfo.manufacturer_string, sManName.c_str());
						wcscpy(rawHIDobject->rawHID.selectedDeviceInfo.product_string, sProdName.c_str());
						rawHIDobject->rawHID.selectedDeviceInfo.usage_page = SABRE_USAGEPAGE;
						rawHIDobject->rawHID.selectedDeviceInfo.usage = SABRE_USAGE;

						/* Display HID device info on the GUI */
						wstring tempWStr1(rawHIDobject->rawHID.selectedDeviceInfo.manufacturer_string);
						wstring tempWStr2(rawHIDobject->rawHID.selectedDeviceInfo.product_string);
						string tempStr1(tempWStr1.begin(), tempWStr1.end());
						string tempStr2(tempWStr2.begin(), tempWStr2.end());
						GUIdeviceInfo = "RawHID device: " + tempStr1 + " " + tempStr2;

						/* Return success */
						retVal = true;
					}
				}
			}
		}
	}
	return retVal;
}

//--------------------------------------------------------------
void ofApp::update(){
	//receiveOSC();
	if (rawHIDobject->rawHID.deviceUnplugged) {
		ofSystemAlertDialog("Device not plugged in or not recognized!");
		rawHIDobject->rawHID.closeDevice(); // "disconnect" the HID device
		rawHIDobject->stop(); // stop the HID/OSCsender thread
		rawHIDobject->rawHID.deviceOpen = false; //
		rawHIDobject->rawHID.deviceSelected = false;
		rawHIDobject->rawHID.deviceUnplugged = false; // reset deviceUnplugged flag to avoid dialog window continuously coming
	}
	/* template code */
	//sync.update();
	/* template code */
}

//--------------------------------------------------------------
void ofApp::exit() {
	free(rawHIDobject);

	XML.pushTag("sabre", 0);
	XML.removeTag("display", 0);
	XML.setValue("display", drawValues, 0);
	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
}

//--------------------------------------------------------------
bool ofApp::startHID()
{
	bool retVal = false;
	if(appDebug) printf("[ofApp::startHID] starting HID thread...\n");
	if (rawHIDobject->rawHID.deviceSelected) { // first check if a device has been selected
		//if (rawHIDobject->rawHID.isOpen()) {
		if (rawHIDobject->rawHID.deviceOpen) {
			if (appDebug) printf("[ofApp::startHID] HID device already open... closing and re-opening\n");
			rawHIDobject->rawHID.closeDevice();
		}

		/* Trying to open HID device... return bool success information */
		rawHIDobject->rawHID.deviceOpen = rawHIDobject->rawHID.openDevice();

		/* If HID device is open, start HID/OSCsender thread */
		if (rawHIDobject->rawHID.deviceOpen) {
			bool running = rawHIDobject->isThreadRunning();
			if (appDebug) printf("[ofApp::startHID] HID thread running? %s\n", (running ? "Yes" : "No"));
			/* If thread is already running, stop and re-start */
			if (running) {
				rawHIDobject->stop();
			}
			rawHIDobject->start(); // the HID thread
			rawHIDobject->rawHID.deviceUnplugged = false;
			if (appDebug) printf("[ofApp::startHID] HID thread open\n");
			///* Display HID device info on the GUI */
			//wstring tempWStr1(rawHIDobject->rawHID.selectedDeviceInfo.manufacturer_string);
			//wstring tempWStr2(rawHIDobject->rawHID.selectedDeviceInfo.product_string);
			//string tempStr1(tempWStr1.begin(), tempWStr1.end());
			//string tempStr2(tempWStr2.begin(), tempWStr2.end());
			//GUIdeviceInfo = "RawHID device: " + tempStr1 + " " + tempStr2;

			retVal = true;
		}
		else {
			/* Stop the HID/OSCsender thread */
			rawHIDobject->stop();
			//ofSystemAlertDialog("SABRe Server \n" + GUIdeviceInfo + rawHIDobject->rawHID.selectedDeviceInfo.manufacturer_string + " " + rawHIDobject->rawHID.selectedDeviceInfo.product_string);
			if (appDebug) printf("[ofApp::startHID] Failed to open HID thread\n");

			/* Display error on the GUI */
			GUIdeviceInfo = "ERROR: unable to open!";

			retVal = false;
		}
		redrawFlag = 1;
	}
	else {
		if (appDebug) printf("[ofApp::startHID] No HID device selected...\n");
		retVal = false;
	}

	return retVal;
}

//--------------------------------------------------------------
void ofApp::stopHID()
{
    /* First stop the HID/OSCsender thread to avoid error message */
    while (rawHIDobject->isThreadRunning()) {
        rawHIDobject->stop();
        usleep(5000);
//        stopOSC();
    }

	/* Then close the HID device if open */
	if (rawHIDobject->rawHID.deviceOpen) {
		bool ret = rawHIDobject->rawHID.closeDevice();
		if (ret) {
			if (appDebug) printf("[ofApp::stopHID] HID device closed successfully\n");
			rawHIDobject->rawHID.deviceOpen = false;
		}
		else {
			if (appDebug) printf("[ofApp::stopHID] HID device closing error!!\n");
		}
	}
	/* Update GUI */
	redrawFlag = 1;
}

//--------------------------------------------------------------
int ofApp::startOSC()
{
	int retVal = 0;

	/* If OSC already started, stop the HID thread in order to restart */
	if (rawHIDobject->OSCsenderOpen) {
		rawHIDobject->stop(); // stops the thread, deletes the object which also closes the socket
		if (appDebug) printf("[ofApp::startOSC] OSC thread already open... closing and re-opening\n");
		stopOSC();
	}

	/* Try to create a socket for each active OSC sender and display connection status on the GUI */
	for (int i = 0; i < OSC_NUMSENDERS; i++) {
		if (rawHIDobject->senderActive[i]) {
			if(appDebug) printf("[ofApp::startOSC] Sender %d active... setting up OSC socket:\n  - IP:   %s\n  - port: %d\n", i, rawHIDobject->sendIP[i].c_str(), rawHIDobject->sendport[i]);
			bool senderStatus = rawHIDobject->sender[i].setup(rawHIDobject->sendIP[i].c_str(), rawHIDobject->sendport[i]);
			if (senderStatus) {
				GUIoscInfo[i] = "OSC-stream " + ofToString(i + 1) + ": IP " + rawHIDobject->sendIP[i] + " Port " + ofToString(rawHIDobject->sendport[i]) + " on";
				retVal += 1;
			}
			else {
				GUIoscInfo[i] = "Unable to open Network " + rawHIDobject->sendIP[i] + " on port " + ofToString(rawHIDobject->sendport[i]);
			}
		}
		else {
			GUIoscInfo[i] = "OSC-stream " + ofToString(i + 1) + " OFF - " + rawHIDobject->sendIP[i] + " / " + ofToString(rawHIDobject->sendport[i]);
			if(appDebug) printf("[startOSC] Sender %d inactive... jumping\n", i);
		}
	}

	rawHIDobject->OSCsenderOpen = true; // set OSC open flag... even if no sender is active

	redrawFlag = 1;

    if(appDebug) printf("[ofApp::startOSC] OSC sender started... %d active senders\n", retVal);
	return retVal;
}

//--------------------------------------------------------------
void ofApp::stopOSC()
{
    if(appDebug) printf("[ofApp::stopOSC] clearing OSC open flag\n");
	rawHIDobject->OSCsenderOpen = false;
}

//--------------------------------------------------------------
void ofApp::receiveOSC()
{
	int i;
	string temp;

	while (receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage(&m);
		temp = m.getAddress();

		if (!strcmp(temp.c_str(), "/sabre/framerate")) {
			//			framerate = m.getArgAsInt32( 0 );
			//			GUIoscInfo = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}
		else if (strcmp(temp.c_str(), "/sabre/display") == 0) {
			display = m.getArgAsInt32(0);
			windowChanged = true;
		}
		else if (strcmp(temp.c_str(), "/sabre/reset") == 0) {
		}
		else if (strcmp(temp.c_str(), "/sabre/writePrefs") == 0) {
			ofApp::writePrefs();
		}
		else if (strcmp(temp.c_str(), "/sabre/readPrefs") == 0) {
			ofApp::readPrefs();
		}
		else if (strcmp(temp.c_str(), "/sabre/network/receiver/port") == 0) {
			receiveport = m.getArgAsInt32(0);
			receiver.setup(receiveport);
		}
		else if (strcmp(temp.c_str(), "/sabre/network/sender/port") == 0) {
			rawHIDobject->sendport[0] = m.getArgAsInt32(0);
			// TODO switch between fulspeed or OSc thread
			//			rawHIDobject->sender.setup( rawHIDobject->sendIP, rawHIDobject->sendport );
			//			GUIoscInfo = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}
		else if (strcmp(temp.c_str(), "/sabre/network/sender/IP") == 0) {
			rawHIDobject->sendIP[0] = m.getArgAsString(0);
			// TODO switch between fulspeed or OSc thread
			//			rawHIDobject->sender.setup( m.getArgAsString(0), rawHIDobject->sendport );
			//			GUIoscInfo = "sending OSC to "+rawHIDobject->sendIP+" on port "+ofToString(rawHIDobject->sendport);
		}
		else if (strcmp(temp.c_str(), "/sabre/exit") == 0) {
			ofApp().exit();
		}
		else if (strcmp(temp.c_str(), "/sabre/calibrateSwitch") == 0) {
			if (rawHIDobject->calibrateSwitch == 1) { // before we switch it off

				for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
					rawHIDobject->calibrate[i] = 0;
				}
				writeScaling(); // we write the values into the prefs

			}
			rawHIDobject->calibrateSwitch = !rawHIDobject->calibrateSwitch;
			for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
				rawHIDobject->calibrate[i] = !rawHIDobject->calibrate[i];
			}
			//			printf("calibrate is %d\n", rawHIDobject->calibrateSwitch);

		}
		else if (strcmp(temp.c_str(), "/sabre/calibrate") == 0) {
			int which = m.getArgAsInt32(0);
			rawHIDobject->calibrate[which] = m.getArgAsInt32(1);
			if (rawHIDobject->calibrate[which] == 1) { // reset calibration values
				rawHIDobject->keys[which].minimum = 1023;
				rawHIDobject->keys[which].maximum = 0;
			}

			if (rawHIDobject->calibrate[which] == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
			//			printf("calibrate[%d] is %d\n", which, rawHIDobject->calibrate[which]);
		}
		else if (strcmp(temp.c_str(), "/sabre/calibrate/air") == 0) {
			int which = m.getArgAsInt32(0);
			rawHIDobject->airValue.calibratePressureRange = m.getArgAsInt32(1);
			if (rawHIDobject->airValue.calibratePressureRange == 1) { // reset calibration values
				rawHIDobject->keys[which].minimum = 32768;
				rawHIDobject->keys[which].maximum = -32768;
			}

			if (rawHIDobject->airValue.calibratePressureRange == 0) { // before we switch it off
				writeScaling(); // we write the values into the prefs
			}
			//			printf("calibrateAir is %d\n", rawHIDobject->airValue.calibratePressureRange);
		}
	}
}

//--------------------------------------------------------------
bool ofApp::readPrefs()
{
	int numTags;
	int numPtTags;
	int totalToRead;
	int i;
	int ID;
	int bitwidth;

	/* Load sabreServer.xml with all settings */
	bool result = XML.loadFile("sabreServer.xml");
	if (result) {
		/* Look for VID & PID values. If none, set default */
		string VID = XML.getValue("sabre:rawHID:VID", SABRE_VENDORID);
		string PID = XML.getValue("sabre:rawHID:PID", SABRE_PRODUCTID);
		rawHIDobject->rawHID.selectedDeviceInfo.vendor_id = strtol(VID.c_str(), NULL, 16);
		rawHIDobject->rawHID.selectedDeviceInfo.product_id = strtol(PID.c_str(), NULL, 16);
		/* Look for manufacturer name. If none, set default */
		string tempStr1 = XML.getValue("sabre:rawHID:manufacturername", SABRE_MANUFACTURERSTRING);
		const size_t tempLen1 = (strlen(tempStr1.c_str()) + 1) * 2;
		size_t convChars1 = 0;
		wchar_t *tempWStr1 = new wchar_t[tempLen1];
        mbstowcs(tempWStr1, tempStr1.c_str(), tempLen1);
        // VisualStudio issue... to check
//		mbstowcs_s(&convChars1, tempWStr1, tempLen1, tempStr1.c_str(), _TRUNCATE);
		rawHIDobject->rawHID.selectedDeviceInfo.manufacturer_string = tempWStr1;
		
		string tempStr2 = XML.getValue("sabre:rawHID:productname", "SABRe");
		const size_t tempLen2 = (strlen(tempStr2.c_str()) + 1) * 2;
		size_t convChars2 = 0;
		wchar_t *tempWStr2 = new wchar_t[tempLen2];
        mbstowcs(tempWStr2, tempStr2.c_str(), tempLen2);
        // VisualStudio issue... to check
//        mbstowcs_s(&convChars2, tempWStr2, tempLen2, tempStr2.c_str(), _TRUNCATE);
		rawHIDobject->rawHID.selectedDeviceInfo.product_string = tempWStr2;
		//rawHIDobject->rawHID.selectedDeviceInfo.manufacturerName = XML.getValue("sabre:rawHID:manufacturername", "ICST");
		//rawHIDobject->rawHID.selectedDeviceInfo.productName = XML.getValue("sabre:rawHID:productname", "SABRe");

		rawHIDobject->sendRawValues = XML.getValue("sabre:OSCsender:sendRawValues", 0);
		rawHIDobject->OSCsendingInterval = XML.getValue("sabre:OSCsender:interval", 3);
		rawHIDobject->OSCsendingInterval = MAX(rawHIDobject->OSCsendingInterval, 3); // clip to 3

		rawHIDobject->numOSCloops = rawHIDobject->OSCsendingInterval * 2;

		framerate = XML.getValue("sabre:framerate", 20);


		numTags = XML.getNumTags("sabre:network:sender");
		if (numTags > 0) {
			XML.pushTag("sabre", numTags - 1);
			XML.pushTag("network", numTags - 1);

			numPtTags = XML.getNumTags("sender");
			if (numPtTags > 0) {
				totalToRead = MIN(numPtTags, OSC_NUMSENDERS);
				for (i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("sender", "id", 0, i);
					ID--;
					rawHIDobject->sendIP[ID] = XML.getValue("sender:IP", "127.0.0.1", i);
					rawHIDobject->sendport[ID] = XML.getValue("sender:port", 40002 + ID, i);
					rawHIDobject->senderActive[ID] = XML.getValue("sender:active", 0, i);
					rawHIDobject->senderMode[ID] = XML.getValue("sender:mode", 1, i);
				}
			}
			XML.popTag();
			XML.popTag();
		}
		rawHIDobject->calcResetID();
		//if(appDebug) printf("calcResetID exited\n");

		receiveport = XML.getValue("sabre:network:receiver:port", 40001);

		drawValues = XML.getValue("sabre:display", 0);
		rawHIDobject->drawValues = drawValues = CLAMP(drawValues, 0, 1);

		rawHIDobject->threshDown = XML.getValue("sabre:thresholds:down", 0.2);
		rawHIDobject->threshUp = XML.getValue("sabre:thresholds:up", 0.8);

		for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
			rawHIDobject->keys[i].threshDown = rawHIDobject->threshDown;
			rawHIDobject->keys[i].threshUp = rawHIDobject->threshUp;
		}

		rawHIDobject->debounceTimeout = XML.getValue("sabre:debounce-timeout", 0);
		rawHIDobject->accelResolution = XML.getValue("sabre:accel-resolution", 4);


		// 	4g/8g/16g 10/11/12/13bit resolution
		//  2g (10bit) : 512, 4g (11bit) : 1024, 8g (12bit) :  2048, 16g (13bit) : 4096
		if (rawHIDobject->accelResolution == 2) {
			bitwidth = 10;
		}
		else if (rawHIDobject->accelResolution == 4) {
			bitwidth = 11;
		}
		else if (rawHIDobject->accelResolution == 8) {
			bitwidth = 12;
		}
		else if (rawHIDobject->accelResolution == 16) {
			bitwidth = 13;
		}

		// v3.5 comm structure
		//		double rangeVal = pow((double)2.0, (double)bitwidth);
		//		rawHIDobject->accelOffset = rangeVal * 0.5;
		//		rawHIDobject->accelScale = 1.0 / rangeVal;

		numTags = XML.getNumTags("sabre:key");
		if (numTags > 0)
		{
			XML.pushTag("sabre", numTags - 1);

			rawHIDobject->numKeyAddr = numPtTags = XML.getNumTags("key"); // key tags
			if (numPtTags > 0) {
				totalToRead = MIN(numPtTags, 64);
				for (i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("key", "id", 0, i);
					ID--;
					str1 = XML.getValue("key:oscaddress", "/sabre/key/0", i);
					if (str1.length() > 0) {
						rawHIDobject->keys[ID].oscaddress = str1;
					}
					rawHIDobject->keys[ID].inverted = XML.getValue("key:invert", 0, i);
					rawHIDobject->keys[ID].minimum = XML.getValue("key:minimum", 0, i);
					rawHIDobject->keys[ID].maximum = XML.getValue("key:maximum", 1023, i);
					//					keys[ID].range = keys[ID].maximum - keys[ID].minimum;
					if (rawHIDobject->keys[ID].maximum != rawHIDobject->keys[ID].minimum) {
						rawHIDobject->keys[ID].range = 1.0 / (rawHIDobject->keys[ID].maximum - rawHIDobject->keys[ID].minimum);
					}
					else {
						rawHIDobject->keys[ID].range = 0.0;
					}
					rawHIDobject->keys[ID].threshUp = XML.getValue("key:threshold:up", rawHIDobject->threshUp, i);
					rawHIDobject->keys[ID].threshDown = XML.getValue("key:threshold:down", rawHIDobject->threshDown, i);
				}
			}

			rawHIDobject->numImuAddr = numPtTags = XML.getNumTags("imu"); // imu tags
			if (numPtTags > 0) {
				totalToRead = MIN(numPtTags, 12);
				for (i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("imu", "id", 0, i);

					str1 = XML.getValue("imu:oscaddress", "/sabre/motion/0", i);
					if (str1.length() > 0) {
						rawHIDobject->imuaddresses[ID] = str1;
					}
				}
			}

			rawHIDobject->raw[0] = 0;
			rawHIDobject->IMU[0] = 0.0f;
			rawHIDobject->raw[1] = 0;
			rawHIDobject->IMU[1] = 0.0f;
			rawHIDobject->raw[2] = 0;
			rawHIDobject->IMU[2] = 0.0f;
			rawHIDobject->raw[3] = 0;
			rawHIDobject->IMU[3] = 0.0f;
			rawHIDobject->raw[4] = 0;
			rawHIDobject->IMU[4] = 0.0f;
			rawHIDobject->raw[5] = 0;
			rawHIDobject->IMU[5] = 0.0f;
			rawHIDobject->raw[6] = 0;
			rawHIDobject->IMU[6] = 0.0f;
			rawHIDobject->raw[7] = 0;
			rawHIDobject->IMU[7] = 0.0f;
			rawHIDobject->raw[8] = 0;
			rawHIDobject->IMU[8] = 0.0f;


			rawHIDobject->numButtonAddr = numPtTags = XML.getNumTags("button");
			if (numPtTags > 0) {
				totalToRead = MIN(numPtTags, 3);
				for (i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("button", "id", 0, i);

					str1 = XML.getValue("button:oscaddress", "/sabre/button/0", i);
					if (str1.length() > 0) {
						rawHIDobject->buttonaddresses[ID] = str1;
					}
				}
			}

			rawHIDobject->numAirAddr = numPtTags = XML.getNumTags("air");
			if (numPtTags > 0) {
				totalToRead = MIN(numPtTags, 2);
				for (i = 0; i < totalToRead; i++) {
					ID = XML.getAttribute("air", "id", 0, i);

					str1 = XML.getValue("air:oscaddress", "/sabre/air/0", i);
					if (str1.length() > 0) {
						rawHIDobject->airaddresses[ID] = str1;
					}
				}
				rawHIDobject->airValue.minimum = XML.getValue("air:minimum", 0, -20.0);
				rawHIDobject->airValue.maximum = XML.getValue("air:maximum", 120, 80.0);
				if (rawHIDobject->airValue.maximum != rawHIDobject->airValue.minimum) {
					if (rawHIDobject->airValue.maximum > abs(rawHIDobject->airValue.minimum)) {
						rawHIDobject->airValue.range = (1.0 / rawHIDobject->airValue.maximum) * 0.5;
					}
					else if (rawHIDobject->airValue.maximum < abs(rawHIDobject->airValue.minimum)) {
						rawHIDobject->airValue.range = (1.0 / abs(rawHIDobject->airValue.minimum)) * 0.5;
					}
					else {
						rawHIDobject->airValue.range = 0.0;
					}


				}
				else {
					rawHIDobject->airValue.range = 0.0;
				}
			}

			XML.popTag(); // pop root /sabre tag
		}

		str1 = XML.getValue("system:oscaddress", "/sabre/systime", i);
		if (str1.length() > 0) {
			rawHIDobject->timestampAddressServer = str1 + "/server";
			rawHIDobject->timestampAddressLeft = str1 + "/left";
			rawHIDobject->timestampAddressRight = str1 + "/right";
			rawHIDobject->timestampAddressAir = str1 + "/air";
		}

		str1 = XML.getValue("keycode:oscaddress", "/sabre/keycode", i);
		if (str1.length() > 0) {
			rawHIDobject->keycodeaddress = str1;
		}

		str1 = XML.getValue("midinote:oscaddress", "/sabre/note", i);
		if (str1.length() > 0) {
			rawHIDobject->midinoteaddress = str1;
		}
	}
	return result;
}

//--------------------------------------------------------------
void ofApp::dumpPrefs()
{
	int i;

	//printf("serialport %s\n", rawHIDobject->serialport.c_str());

	printf("sender 1 IP %s\n", rawHIDobject->sendIP[0].c_str());
	printf("sender 1 port %d\n", rawHIDobject->sendport[0]);
	printf("sender 1 active %d\n", rawHIDobject->senderActive[0]);
	printf("sender 1 mode %d\n", rawHIDobject->senderMode[0]);

	printf("sender 2 IP %s\n", rawHIDobject->sendIP[1].c_str());
	printf("sender 2 port %d\n", rawHIDobject->sendport[1]);
	printf("sender 2 active %d\n", rawHIDobject->senderActive[1]);
	printf("sender 2 mode %d\n", rawHIDobject->senderMode[1]);

	printf("sender 3 IP %s\n", rawHIDobject->sendIP[2].c_str());
	printf("sender 3 port %d\n", rawHIDobject->sendport[2]);
	printf("sender 3 active %d\n", rawHIDobject->senderActive[2]);
	printf("sender 3 mode %d\n", rawHIDobject->senderMode[2]);

	printf("sender 4 IP %s\n", rawHIDobject->sendIP[3].c_str());
	printf("sender 4 port %d\n", rawHIDobject->sendport[3]);
	printf("sender 4 active %d\n", rawHIDobject->senderActive[3]);
	printf("sender 4 mode %d\n", rawHIDobject->senderMode[3]);

	printf("receive port %d\n", receiveport);
	//printf("baudrate %d\n", rawHIDobject->baudrate);
	printf("framerate %d\n", framerate);
	printf("display %d\n", display);
	printf("threshDown %f\n", rawHIDobject->threshDown);
	printf("threshUp %f\n", rawHIDobject->threshUp);
	printf("accelResolution %d\n", rawHIDobject->accelResolution);
	printf("accelOffset %ld\n", rawHIDobject->accelOffset);
	printf("accelScale %2.12f\n", rawHIDobject->accelScale);
	//	printf("OSCfullspeed %d\n", rawHIDobject->fullspeedOSC);
	printf("OSCinterval %d\n", rawHIDobject->OSCsendingInterval);

	for (i = 0; i < rawHIDobject->numKeyAddr; i++) {
		printf("key %d\n", i);
		printf("    oscaddress %s\n", rawHIDobject->keys[i].oscaddress.c_str());
		printf("    inverted %d\n", rawHIDobject->keys[i].inverted);
		printf("    minimum %ld\n", rawHIDobject->keys[i].minimum);
		printf("    maximum %ld\n", rawHIDobject->keys[i].maximum);
		printf("    threshUp %f\n", rawHIDobject->keys[i].threshUp);
		printf("    threshDown %f\n", rawHIDobject->keys[i].threshDown);
	}
	printf("keycode\n    oscaddress %s\n", rawHIDobject->keycodeaddress.c_str());

	for (i = 0; i < rawHIDobject->numAirAddr; i++) {
		printf("airValues %d\n", i);
		printf("    minimum %f\n", rawHIDobject->airValue.minimum);
		printf("    maximum %f\n", rawHIDobject->airValue.maximum);
	}

	printf("imu\n");
	for (i = 0; i < rawHIDobject->numImuAddr; i++) {
		printf("    oscaddress %d %s\n", i, rawHIDobject->imuaddresses[i].c_str());
	}

	printf("button\n");
	for (i = 0; i < rawHIDobject->numButtonAddr; i++) {
		printf("    oscaddress %d %s\n", i, rawHIDobject->buttonaddresses[i].c_str());
	}
	printf("air\n");

	for (i = 0; i < rawHIDobject->numAirAddr; i++) {
		printf("    oscaddress %d %s\n", i, rawHIDobject->airaddresses[i].c_str());

	}
	printf("timestamp\n    oscaddress server %s\n", rawHIDobject->timestampAddressServer.c_str());
	printf("    oscaddress left %s\n", rawHIDobject->timestampAddressLeft.c_str());
	printf("    oscaddress right %s\n", rawHIDobject->timestampAddressRight.c_str());
	printf("    oscaddress air %s\n", rawHIDobject->timestampAddressAir.c_str());

	printf("link quality\n    oscaddress left %s\n", rawHIDobject->linkQualityAddressLeft.c_str());
	printf("    oscaddress right %s\n", rawHIDobject->linkQualityAddressRight.c_str());
	printf("    oscaddress air %s\n", rawHIDobject->linkQualityAddressAir.c_str());

	printf("battery\n    oscaddress main %s\n", rawHIDobject->batteryAddressMain.c_str());
	printf("    oscaddress air %s\n", rawHIDobject->batteryAddressAir.c_str());


}

//--------------------------------------------------------------
void ofApp::writePrefs()
{
	// only store GUI changeable control parameters:
	XML.setValue("sabre:display", display);
	//XML.setValue("sabre:serialport", rawHIDobject->serialport);
	XML.saveFile("sabreServer.xml");
	return;
}

//--------------------------------------------------------------
bool ofApp::readMidicodes()
{
	int numTags; // found tags
	/* Load MIDI codes xml file */
	bool result = XMLmidi.loadFile("sabreMidicodes.xml");
	if(appDebug) printf("[ofApp::readMidicodes] midiCodes loaded with result %d\n", result);
	if (result) {
		/* First look for a correctly-written sabreMidicodes.xml file */
		numTags = XMLmidi.getNumTags("sabre:note");
		if(appDebug) printf("[ofApp::readMidicodes] readMidicodes numTags %d\n", numTags);
		if (numTags > 0) {
			XMLmidi.pushTag("sabre", 0); // enter one hierarchy level deeper
			numTags = XMLmidi.getNumTags("note"); // look for all MIDI <note> tags
			if(appDebug) printf("[ofApp::readMidicodes] midiCodes loaded with %d note(s)\n", numTags);
			/* Erase all previously loaded MIDI notes */
			for (int i = 0; i < SABRE_NUMMIDI; i++) {
				rawHIDobject->midiNote[i].note = 0;
				rawHIDobject->midiNote[i].keycode = -1;
			}
			/* Load the midiNote[] array with the found <note> tags content */
			for (int i = 0; i < numTags; i++) {
				rawHIDobject->midiNote[i].note = XMLmidi.getValue("note:midi", 0, i);
				rawHIDobject->midiNote[i].keycode = XMLmidi.getValue("note:code", 0, i);
				if(appDebug) printf("[ofApp::readMidicodes] midiNote ID %d note %d code %ld\n", i, rawHIDobject->midiNote[i].note, rawHIDobject->midiNote[i].keycode); 
			}
			XML.popTag(); // exit the previously entered hierarchy level
		}
	}
	return result;
}

//--------------------------------------------------------------
void ofApp::writeScaling()
{
	XML.pushTag("sabre", 0);

	for (int i = 0; i < rawHIDobject->numKeyAddr; i++) {
		XML.pushTag("key", i);

		XML.removeTag("minimum", 0);
		XML.removeTag("maximum", 0);
		XML.setValue("minimum", (int)rawHIDobject->keys[i].minimum, i);
		XML.setValue("maximum", (int)rawHIDobject->keys[i].maximum, i);
		// printf("key %d min %d max %d\n", i, (int)rawHIDobject->keys[i].minimum, (int)rawHIDobject->keys[i].maximum);
		XML.popTag();
	}
	XML.removeTag("air:minimum", 0);
	XML.removeTag("air:maximum", 0);
	XML.setValue("air:minimum", rawHIDobject->airValue.minimum, 0);
	XML.setValue("air:maximum", rawHIDobject->airValue.maximum, 0);

	XML.popTag();
	XML.saveFile("sabreServer.xml");
	return;
}

//--------------------------------------------------------------
void ofApp::resetCalibrate()
{
	for (int i = 0; i < rawHIDobject->numKeyAddr; i++)
	{
		rawHIDobject->keys[i].minimum = 1023;
		rawHIDobject->keys[i].maximum = 0;
	}
}

//--------------------------------------------------------------
void ofApp::resetAirCalibrate()
{
	rawHIDobject->airValue.minimum = 32768;
	rawHIDobject->airValue.maximum = -32768;
}

//--------------------------------------------------------------
void ofApp::resetSingleCalibrate(int i)
{
	rawHIDobject->keys[i].minimum = 1023;
	rawHIDobject->keys[i].maximum = 0;

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	case 'f': // f-key: switch winow size
		drawValues = !drawValues;
		rawHIDobject->drawValues = drawValues;
		windowChanged = true;
		redrawFlag = 1;
		//			if(!drawValues) {
		//				rawHIDobject->calibrate = 0;
		//				writeScaling();
		//			} else {
		//				resetCalibrate();
		//			}
		break;
	case 'c': // c-key:
			  /*
			  rawHIDobject->calibrate != rawHIDobject->calibrate;
			  if(rawHIDobject->calibrate == 1) { // before we switch it off
			  writeScaling();  // we write the values into the prefs
			  } else {
			  resetCalibrate();
			  }
			  redrawFlag = 1;
			  windowChanged = true;
			  lastRedraw = ofGetElapsedTimef();
			  */
		break;
	case 'r': // r key:
		break;
	case 'd':
		dumpPrefs();
		break;
	case 'p':
		// sabreServer::readPrefs();
		break;
	case 'F':
		drawValues = !drawValues;
		rawHIDobject->drawValues = drawValues;
		windowChanged = true;
		redrawFlag = 1;
		//			if(!drawValues) {
		//				rawHIDobject->calibrate = 0;			
		//				writeScaling();
		//			} else {
		//				resetCalibrate();
		//			}
		break;
	case 'w':
		// sabreServer::writePrefs();
		break;
	case OF_KEY_LEFT:
		break;
	case OF_KEY_UP:
		whichStatus++;
		if (whichStatus >3) {
			whichStatus = 0;
		}
		redrawFlag = 1;
		break;
	case OF_KEY_RIGHT:
		break;
	case OF_KEY_DOWN:
		whichStatus--;
		if (whichStatus < 0) {
			whichStatus = 3;
		}
		redrawFlag = 1;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	int i;
	//	printf("mousepressed at %d %d\n", x, y);
	ofRect(295, 36, 295 + 124, 36 + 20);

	/* ----------------- *
	 * Start/stop button *
	 * ----------------- */
	if (x > 270 && x < 394 && y > 4 && y < 25) {
		if(appDebug) printf("\n------------------------------------\nstart/stop clicked\n - device selected? %s\n - device open? %s\n------------------------------------\n", (rawHIDobject->rawHID.deviceSelected ? "YES" : "NO"), (rawHIDobject->rawHID.deviceOpen ? "OPEN" : "CLOSED"));

		/* First check if a device is selected and ready to open */
		if (!rawHIDobject->rawHID.deviceSelected) {
			/* If no device slected, do an enumeration and selection */
			getHIDDeviceList();
			/* If device successfully selected, just change flag for next step */
			if (selectHIDdevice()) {
				rawHIDobject->rawHID.deviceSelected = true;
			}
			/* If no device selected, set 'unplugged' flag to recall the system alert dialog box */
			else {
				rawHIDobject->rawHID.deviceSelected = false;
				rawHIDobject->rawHID.deviceUnplugged = true;
			}
		}

		/* If a compatible device has been found and is selected, check device open status */
		if(rawHIDobject->rawHID.deviceSelected) {
			/* Device open --> stopping */
			if (rawHIDobject->rawHID.deviceOpen) {
				stopHID();
				stopOSC();
			}
			/* Device closed --> starting */
			else {
				stopHID();
				int num = startOSC();
//				if (appDebug) printf("[ofApp::mousePressed] %d OSC sender(s) started\n", num);
				if (num > 0) {
					/* If HID thread start did not work, stop previously opened OSC senders,
					 * and reset the flags in order to re-start an enumeration */
					if (!startHID()) {
						stopOSC();
						rawHIDobject->rawHID.deviceSelected = false;
						rawHIDobject->rawHID.deviceUnplugged = true;
					}
				}
			}

			drawValues = !hiddenValues;
			rawHIDobject->drawValues = !hiddenValues;
		}

		windowChanged = true;
		redrawFlag = 1;
	}

	/* ---------------- *
	 * Show/hide values *
	 * ---------------- */
	if (x > 397 && x < 520 && y > 3 && y < 24) {
		if (drawValues != 0) { // hide
			if(appDebug) printf("[ofApp::mousePressed] Hiding values\n");
			drawValues = false;
			hiddenValues = true;
			rawHIDobject->drawValues = 0;
			rawHIDobject->calibrateSwitch = 0;
		}
		else { // show
			if (appDebug) printf("[ofApp::mousePressed] Showing values\n");
			drawValues = true;
			hiddenValues = false;
			rawHIDobject->drawValues = 1;
		}
		windowChanged = true;
		redrawFlag = 1;
	}

	/* -------------------------- *
	 * main keys calibrate button *
	 * -------------------------- */
	// ofRect(295, 690, 124, 20);
	if (x > 375 && x < 500 && y > 480 && y < 500) {
		/* Switch off */
		if (rawHIDobject->calibrateSwitch != 0) {
			rawHIDobject->calibrateSwitch = 0;
			rawHIDobject->calibrateSingle = 0;
			for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
				rawHIDobject->calibrate[i] = 0;
			}
			writeScaling();

		}
		/* Switch on */
		else {
			rawHIDobject->calibrateSwitch = 1;
			rawHIDobject->calibrateSingle = 1;
			//            for(i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
			//				rawHIDobject->calibrate[i] = 1;
			//			}
			//            
			//			resetCalibrate();
		}
		//		printf("rawHIDobject->calibrateSwitch is %d\n", rawHIDobject->calibrate);
	}
	if (rawHIDobject->calibrateSwitch) {
		// click in calibrateAll == calibrateSingle flag :: conditional on main calibrate button
		if (x > 375 && x < 500 && y > 458 && y < 478) {
			if (rawHIDobject->calibrateSingle != 0) { // switch off
				rawHIDobject->calibrateSingle = 0;
				// reset all to zero
				for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
					rawHIDobject->calibrate[i] = 1;
				}
				resetCalibrate();
			}
			else { // switch on
				rawHIDobject->calibrateSingle = 1;
				for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
					rawHIDobject->calibrate[i] = 0;
				}
				writeScaling();

			}
			//            printf("rawHIDobject->calibrateSingle is %d\n", rawHIDobject->calibrateSingle);
		}

		if (x > 346 && x < 362) {
			int yy = y - 57;
			i = yy / 18;
			//            printf("clicked inside calibrate toggle Nr. %d at pos %d %d\n",i, x, y);
			if (rawHIDobject->calibrate[i] == 0) {
				rawHIDobject->calibrate[i] = 1;
				resetSingleCalibrate(i);
			}
			else {
				rawHIDobject->calibrate[i] = 0;
				writeScaling();
			}
		}

		if (x > 375 && x < 500 && y > 436 && y < 456) {
			//            printf("clicked in reset Calibration");
			for (int i = 0; i < rawHIDobject->numKeyAddr; i++)
			{
				rawHIDobject->keys[i].minimum = 0;
				rawHIDobject->keys[i].maximum = 1023;
			}
		}

	}

	/* -------------------------------- *
	 * Calibrate pressure min/max range *
	 * -------------------------------- */
	if (x > 502 && x < 627 && y > 480 && y < 500) {
		if (rawHIDobject->airValue.calibratePressureRange == true) {
			rawHIDobject->airValue.calibratePressureRange = false;
			//            printf("finished calibrating air with values minimum %f maximum %f\n",rawHIDobject->airValue.minimum, rawHIDobject->airValue.maximum);

			writeScaling();
		}
		else {
			rawHIDobject->airValue.calibratePressureRange = true;
			resetAirCalibrate();
		}
	}

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::draw() {
	int i;
	int anchorx = 15;
	int anchory = 66;
	int stepsize = 18;
	int columnwidth = 200;
	int rightColumn = 270;
	int leftColumn = 10;
	int width = 0;
	int height = 0;
	double yy;
	int pos_x;

	if (windowChanged) {
		if (drawValues == 0) {
			width = 550;
			height = 52;
			ofSetWindowShape(width, height);
			windowChanged = false;
		}
		else if (drawValues == 1) {
			width = 746;
			height = 514; // 790
			timeOut = 5.0;
			ofSetWindowShape(width, height);
			windowChanged = false;
		}
	}

	if (redrawFlag == 1) // drawn once after first update
	{
		// header frame background
		ofFill();
		ofSetColor(0, 0, 0, 1);
		ofRect(0, 0, width, 49);

		ofSetColor(255, 127, 0, 10);
		ofRect(0, 0, width, 3 * height);

		if (rawHIDobject->rawHID.deviceOpen) {
			ofSetColor(63, 63, 63, 255);
		}
		else {
			ofSetColor(127, 127, 127, 255);
		}
		TTFsmall.drawString(GUIdeviceInfo, anchorx, 18);
		TTFsmall.drawString(GUIoscInfo[whichStatus], anchorx, 42);

		// separator lines
		//		ofSetColor(240, 240, 240, 127);
		//		ofLine(0, anchory-14, width, anchory-14);	
		//		ofSetColor(127, 127, 127, 127);
		//		ofLine(0, anchory-13, width, anchory-13);		

		ofSetColor(200, 200, 200, 255);

		// Menu
		//		ofFill();
		//		ofSetColor(232, 232, 232);
		//		ofRect(leftColumn, 3, 188, 18);
		//		ofNoFill();
		//		ofSetColor(127, 127, 127);
		//		ofRect(leftColumn, 3, 188, 18);
		//		ofFill();
		//		ofTriangle(leftColumn+177,7,leftColumn+185, 7, leftColumn+181, 13);
		//		ofSetColor(0, 0, 0);
		//		TTFsmall.drawString(rawHIDobject->serialport,leftColumn+5, 16);

		// start/stop button
		if (rawHIDobject->rawHID.deviceOpen) {
			ofFill();
			ofSetColor(255, 255, 255);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Stop", rightColumn + 48, 16);
		}
		else {
			ofFill();
			ofSetColor(232, 232, 232);
			ofRect(rightColumn, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Start", rightColumn + 48, 16);
		}

		// show values button
		if (!drawValues) {
			ofFill();
			ofSetColor(232, 232, 232);
			ofRect(rightColumn + 126, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn + 126, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTFsmall.drawString("Show Values", rightColumn + 28 + 126, 16);
		}
		else {
			ofFill();
			ofSetColor(255, 255, 255);
			ofRect(rightColumn + 126, 3, 124, 18);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(rightColumn + 126, 3, 124, 18);
			ofSetColor(0, 0, 0);
			TTF.drawString("Hide Values", rightColumn + 32 + 126, 16);
		}
		// Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(375, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(375, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Keys", 375 + 20, 480 + 14);

		// Calibrate Button
		ofFill();
		ofSetColor(232, 232, 232);
		ofRect(502, 480, 124, 20);
		ofNoFill();
		ofSetColor(127, 127, 127);
		ofRect(502, 480, 124, 20);
		ofSetColor(0, 0, 0);
		TTFsmall.drawString("Calibrate Air", 500 + 24, 480 + 14);

		// value display left column

		ofFill();
		for (i = 0; i < 25; i++) { // stripes
			if ((i % 2) == 0) {
				ofSetColor(255, 127, 0, 10);
				ofRect(anchorx - 2, anchory + ((i - 1) * stepsize) + 7, 94, 16);
			}
		}
		for (i = 0; i < 9; i++) { // stripes
			if ((i % 3) == 0) {
				ofSetColor(255, 127, 0, 10);
				ofRect(anchorx - 2 + 360, anchory + ((i - 1) * stepsize) + 7, 144, 16);
			}
		}
		ofFill(); // heading
		ofSetColor(255, 127, 0, 10);
		ofRect(anchorx - 2 + 360, anchory + ((8) * stepsize) + 7, 144, 16);

		ofFill(); // button
		ofSetColor(255, 127, 0, 10);
		ofRect(anchorx - 2 + 360, anchory + ((11) * stepsize) + 7, 144, 16);

		ofFill(); // pressure
		ofSetColor(255, 127, 0, 10);
		ofRect(anchorx - 2 + 360, anchory + ((13) * stepsize) + 7, 144, 16);

		ofFill(); // keycode
		ofSetColor(255, 127, 0, 10);
		ofRect(anchorx - 2 + 360, anchory + ((15) * stepsize) + 7, 144, 16);

		ofSetColor(0, 0, 0, 191);

		for (i = 0; i < 25; i++) { // key addresses
			TTFsmall.drawString(rawHIDobject->keys[i].oscaddress, anchorx, anchory + ((i)* stepsize));
		}
		for (i = 0; i < 9; i++) { // imu addresses
			std::string str = rawHIDobject->imuaddresses[i / 3];
			std::string::size_type end = str.find_last_of('/');
			if (end != str.npos)
				str = str.substr(0, end);
			TTFsmall.drawString(str, anchorx + 360, anchory + ((i)* stepsize));
			//			TTFsmall.drawString(str, anchorx + 360, anchory+((i+25) * stepsize) );
			//			TTFsmall.drawString(rawHIDobject->imuaddresses[i/3], anchorx, anchory+5+((i+25) * stepsize) );
		}

		TTFsmall.drawString(rawHIDobject->imuaddresses[10], anchorx + 360, anchory + ((9) * stepsize));
		TTFsmall.drawString(rawHIDobject->imuaddresses[11], anchorx + 360, anchory + ((10) * stepsize));

		for (i = 0; i < 1; i++) { // first button address truncated
			char temp[64];
			strncpy(temp, rawHIDobject->buttonaddresses[0].c_str(), rawHIDobject->buttonaddresses[0].size() - 2);
			temp[rawHIDobject->buttonaddresses[0].size() - 2] = 0;
			//			TTFsmall.drawString(temp, anchorx + 360, anchory+((i+35) * stepsize) );
			TTFsmall.drawString(temp, anchorx + 360, anchory + ((i + 12) * stepsize));
		}

		// air addresses
		TTFsmall.drawString(rawHIDobject->airaddresses[0], anchorx + 360, anchory + (14 * stepsize));

		// air addresses
		TTFsmall.drawString(rawHIDobject->keycodeaddress, anchorx + 360, anchory + (16 * stepsize));
		TTFsmall.drawString(rawHIDobject->midinoteaddress, anchorx + 360, anchory + (17 * stepsize));

		texScreen.loadScreenData(0, 0, 440, 700);
		drawTex = 1;
	}
	else {
		if (drawTex) {
			texScreen.draw(0, 0, 440, 266);
			drawTex = 0;
		}
	}
#pragma mark draw values    

	anchorx = 12;
	anchory = 66;
	leftColumn = 110;
	int midColumn = 150;
	rightColumn = 220;
	int farRightColumn = 330;

	int imuColumnLeft = 410;

	stepsize = 18;
	columnwidth = 180;
	width = 430;
	height = 635;

	//if (status == 1 && drawValues)
	if (drawValues && !hiddenValues) {
		//			ofFill();
		//			ofSetColor(200, 200, 200, 255);
		//			ofRect(leftColumn-1, 79, width-leftColumn-5, height-anchory-15);
		ofSetColor(0, 127, 255, 255);

		for (i = 0; i < 25; i++) { // stripes
			if ((i % 2) == 0) {
				ofFill();
				ofSetColor(255, 127, 0, 10);
				ofRect(leftColumn - 1, anchory + ((i - 1) * stepsize) + 7, width - leftColumn - 85, 16);
				ofSetColor(0, 0, 0, 255);
			}
		}

		for (i = 0; i < 9; i++) { // stripes
			if ((i % 3) == 0) {
				ofFill();
				ofSetColor(255, 127, 0, 10);
				ofRect(leftColumn - 1 + imuColumnLeft, anchory + ((i - 1) * stepsize) + 7, width - leftColumn - 103, 16);
				ofSetColor(0, 0, 0, 255);
			}
		}

		ofFill(); // heading
		ofSetColor(255, 127, 0, 10);
		ofRect(leftColumn - 1 + imuColumnLeft, anchory + ((8) * stepsize) + 7, width - leftColumn - 103, 16);

		ofFill(); // button
		ofSetColor(255, 127, 0, 10);
		ofRect(leftColumn - 1 + imuColumnLeft, anchory + ((11) * stepsize) + 7, width - leftColumn - 103, 16);

		ofFill(); // pressure
		ofSetColor(255, 127, 0, 10);
		ofRect(leftColumn - 1 + imuColumnLeft, anchory + ((13) * stepsize) + 7, width - leftColumn - 103, 16);

		ofFill(); // keycode
		ofSetColor(255, 127, 0, 10);
		ofRect(leftColumn - 1 + imuColumnLeft, anchory + ((15) * stepsize) + 7, width - leftColumn - 103, 16);

		for (i = 0; i < 25; i++) { // keys
			ofSetColor(0, 0, 0, 255);
			yy = anchory + (i * stepsize);
			TTF.drawString(ofToString(rawHIDobject->keys[i].raw, 6), leftColumn, yy);
			//                printf("%lx ", keys[i].raw);
			TTF.drawString(ofToString(rawHIDobject->keys[i].scaled, 6), midColumn, yy);
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(rightColumn, yy - 9, 104, 12);
			ofFill();
			ofSetColor(0, 0, 0, 127);
			if (rawHIDobject->calibrate[i]) {
				ofSetColor(255, 127, 0, 191);
				ofRect(rightColumn + (104 * rawHIDobject->keys[i].minimum * rawHIDobject->scale10), yy - 7, (104 * (rawHIDobject->keys[i].maximum - rawHIDobject->keys[i].minimum) * rawHIDobject->scale10), 9);
				ofSetColor(0, 0, 0, 255);
				ofRect(rightColumn + (104 * (rawHIDobject->keys[i].raw * rawHIDobject->scale10)), yy - 9, 2, 12);

			}
			else {
				ofRect(rightColumn + (104 * rawHIDobject->keys[i].scaled), yy - 9, 2, 12);
				ofSetColor(91, 91, 91, 255);
				ofLine(rightColumn + (104 * rawHIDobject->keys[i].threshDown), yy - 9, rightColumn + (104 * rawHIDobject->keys[i].threshDown), yy + 4);
				ofLine(rightColumn + (104 * rawHIDobject->keys[i].threshUp), yy - 9, rightColumn + (104 * rawHIDobject->keys[i].threshUp), yy + 4);
			}
			// draw binary boxes
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(farRightColumn, yy - 9, 12, 12);

			if (rawHIDobject->keys[i].binary) {
				ofFill();
				ofSetColor(0, 0, 0, 255);
				ofRect(farRightColumn + 2, yy - 6, 7, 7);
			}
			// individual toggles
			if (rawHIDobject->calibrateSwitch) {
				if (rawHIDobject->calibrateSingle) {

					if (rawHIDobject->calibrate[i]) {
						ofFill();
						ofSetColor(255, 127, 0, 191);
						ofRect(rightColumn + 126, yy - 9, 16, 12);
					}
					ofNoFill();
					ofSetColor(0, 0, 0);
					ofRect(rightColumn + 126, yy - 9, 16, 12);
					TTF.drawString("c", rightColumn + 130, yy + 1);
				}
			}
		}

		for (i = 0; i < 9; i++) { // imu
			ofSetColor(0, 0, 0, 255);
			//				yy = anchory+((i+25) * stepsize);
			yy = anchory + ((i)* stepsize);
			TTF.drawString(ofToString(rawHIDobject->raw[i], 6), leftColumn + imuColumnLeft, yy);
			TTF.drawString(ofToString(rawHIDobject->IMU[i], 6), midColumn + 10 + imuColumnLeft, yy);
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
			ofFill();
			ofSetColor(0, 0, 0, 255);
			ofRect(rightColumn + imuColumnLeft + (104 * rawHIDobject->IMU[i]), yy - 9, 2, 12);
			ofNoFill();
			ofSetColor(91, 91, 91, 255);
			ofLine(rightColumn + 52 + imuColumnLeft, yy - 9, rightColumn + 52 + imuColumnLeft, yy + 4);
		}

		// heading
		ofSetColor(0, 0, 0, 255);
		yy = anchory + ((9) * stepsize);
		TTF.drawString(ofToString(rawHIDobject->heading, 2), midColumn + 10 + imuColumnLeft, yy);
		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
		ofFill();
		ofSetColor(0, 0, 0, 255);
		ofRect(rightColumn + imuColumnLeft + 52 + (rawHIDobject->heading / 3.46153846153846), yy - 9, 2, 12);
		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		ofLine(rightColumn + 52 + imuColumnLeft, yy - 9, rightColumn + 52 + imuColumnLeft, yy + 4);

		// tilt
		ofSetColor(0, 0, 0, 255);
		yy = anchory + ((10) * stepsize);
		TTF.drawString(ofToString(rawHIDobject->tilt, 2), midColumn + 10 + imuColumnLeft, yy);
		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
		ofFill();
		ofSetColor(0, 0, 0, 255);
		//        ofRect( rightColumn + imuColumnLeft + 52 + (52 * rawHIDobject->tilt), yy-9, 2, 12);
		ofRect(rightColumn + imuColumnLeft + (104 * rawHIDobject->tilt), yy - 9, 2, 12);
		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		//        ofLine(rightColumn+52 + imuColumnLeft, yy-9, rightColumn+52 + imuColumnLeft, yy+4);

		// air
		ofSetColor(0, 0, 0, 255);
		//			yy = anchory+(34 * stepsize);
		yy = anchory + (14 * stepsize);
		TTF.drawString(ofToString(rawHIDobject->air[0], 2), leftColumn + imuColumnLeft, yy);
		TTF.drawString(ofToString(rawHIDobject->airValue.range, 2), midColumn + 10 + imuColumnLeft, yy);

		TTF.drawString(ofToString(rawHIDobject->keycode, 2), midColumn + 10 + imuColumnLeft, anchory + (16 * stepsize));
		if (rawHIDobject->validMidiNote) {
			TTF.drawString(ofToString(rawHIDobject->note, 2), midColumn + 10 + imuColumnLeft, anchory + (17 * stepsize));
		}
		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
		ofFill();
		ofSetColor(0, 0, 0, 127);
		ofRect(rightColumn + imuColumnLeft + (104 * (CLAMP(((rawHIDobject->airLong[0] - 500.0) * 0.001), 0, 1))), yy - 9, 2, 12);

		if (rawHIDobject->airValue.calibratePressureRange) {
			ofSetColor(255, 224, 0, 191);
			ofRect(rightColumn + imuColumnLeft, yy - 7, (103), 9);
			// TODO figure scaling for the rangebars
			ofSetColor(0, 0, 0, 255);
			ofRect(rightColumn + imuColumnLeft + (104 * (CLAMP(((rawHIDobject->airValue.range - 500.0) * 0.001), 0, 1))), yy - 9, 2, 12);
		}
		else {
			if (rawHIDobject->airValue.calibrationFlag) {
				ofFill();
				ofSetColor(255, 0, 0, 255);
				ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
			}
			else {
				ofNoFill();
				ofSetColor(91, 91, 91, 255);
				ofRect(rightColumn + imuColumnLeft, yy - 9, 104, 12);
			}
			ofFill();
			ofSetColor(0, 0, 0, 127);
			ofRect(rightColumn + imuColumnLeft + CLAMP((104 * rawHIDobject->airValue.range), 0, 104), yy - 9, 2, 12);
		}

		// buttons
		ofSetColor(0, 0, 0, 255);
		//			yy = anchory+((35) * stepsize);
		yy = anchory + ((12) * stepsize);
		TTF.drawString(ofToString(rawHIDobject->button[0], 1), midColumn + 10 + imuColumnLeft, yy);
		TTF.drawString(ofToString(rawHIDobject->button[1], 1), midColumn + 10 + 12 + imuColumnLeft, yy);
		TTF.drawString(ofToString(rawHIDobject->button[2], 1), midColumn + 10 + 24 + imuColumnLeft, yy);

		ofNoFill();
		ofSetColor(91, 91, 91, 255);
		ofRect(rightColumn + imuColumnLeft, yy - 9, 12, 12);
		ofRect(rightColumn + imuColumnLeft + 14, yy - 9, 12, 12);
		ofRect(rightColumn + imuColumnLeft + 28, yy - 9, 12, 12);

		ofFill();
		ofSetColor(0, 0, 0, 255);
		if (rawHIDobject->button[0]) {
			ofRect(rightColumn + imuColumnLeft + 2, yy - 6, 7, 7);
		}
		if (rawHIDobject->button[1]) {
			ofRect(rightColumn + imuColumnLeft + 16, yy - 6, 7, 7);
		}
		if (rawHIDobject->button[2]) {
			ofRect(rightColumn + imuColumnLeft + 30, yy - 6, 7, 7);
		}

		// battery
		//            ofSetColor(0, 0, 0, 255);
		////			yy = anchory+((36) * stepsize);
		//            yy = 40;
		//			TTF.drawString( "main: "+ofToString((int)(batteryLevelRight*12.5))+"%", anchorx+82 + 360, yy );
		//			TTF.drawString( "mouthpiece: "+ofToString((int)(batteryLevelAir*12.5))+"%", leftColumn+12 + 360, yy );

		if (rawHIDobject->calibrateSwitch) {
			ofFill();
			ofSetColor(255, 127, 0);
			ofRect(375, 480, 124, 20);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(375, 480, 124, 20);
			ofSetColor(0, 0, 0);
			TTF.drawString("Calibrating Keys", 375 + 12, 480 + 14);

			if (rawHIDobject->calibrateSingle == 0) {
				ofFill();
				ofSetColor(255, 127, 0);
				ofRect(375, 458, 124, 20);
				ofNoFill();
				ofSetColor(127, 127, 127);
				ofRect(375, 458, 124, 20);
				ofSetColor(0, 0, 0);
				TTF.drawString("Calibrate All...", 375 + 24, 458 + 14);
			}
			else {
				ofNoFill();
				ofSetColor(127, 127, 127);
				ofRect(375, 458, 124, 20);
				ofSetColor(0, 0, 0);
				TTF.drawString("Calibrate All Keys", 375 + 10, 458 + 14);
			}

			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(375, 436, 124, 20);
			ofSetColor(0, 0, 0);
			TTF.drawString("Reset Key Calibr.", 375 + 12, 436 + 14);
		}

		if (rawHIDobject->airValue.calibratePressureRange) {
			ofFill();
			ofSetColor(255, 224, 0);
			ofRect(502, 480, 124, 20);
			ofNoFill();
			ofSetColor(127, 127, 127);
			ofRect(502, 480, 124, 20);
			ofSetColor(0, 0, 0);
			TTF.drawString("Calibrating Air", 502 + 12, 480 + 14);
		}

	}
	else {
		//printf("SabreServer: couldn't start HID Thread !! can't lock! either an error or the thread has stopped\n");
	}


#pragma mark draw levels

	ofSetColor(63, 63, 63, 255);
	TTFsmall.drawString("battery: main       air", 280, 34);

	// battery display
	for (i = 0; i < 15; i++) {
		pos_x = 360;
		if (rawHIDobject->batteryLevelRight*6.667 >= (i * 6.667)) {
			ofSetColor(127, 127, 127);
			ofRect(pos_x + i * 2, 25, 2, 10);
		}
	}
	//    ofRect
	for (i = 0; i < 15; i++) {
		pos_x = 425;
		if (rawHIDobject->batteryLevelAir*6.667 >= (i * 6.667)) {
			ofSetColor(127, 127, 127);
			ofRect(pos_x + i * 2, 25, 2, 10);
		}
	}
	ofSetColor(63, 63, 63, 255);
	ofNoFill();
	ofRect(360, 25, 31, 10);
	ofRect(425, 25, 31, 10);
	ofRect(391, 27, 2, 6);
	ofRect(456, 27, 2, 6);

	ofFill();


	ofSetColor(63, 63, 63, 255);
	TTFsmall.drawString("wireless: left       right      air", 280, 48);

	for (i = 0; i < 8; i++) {
		pos_x = 360;
		if ((CLAMP(rawHIDobject->linkQualityLeft, 0, 205) - 0) >= (i * 18)) {
			ofSetColor(127, 127, 127, 255);
		}
		else {
			ofSetColor(212, 212, 212, 255);

		}
		ofRect(pos_x + i * 4, 36 + (10 - i), 2, 2 + i);
	}
	for (i = 0; i < 8; i++) {
		pos_x = 430;
		if ((CLAMP(rawHIDobject->linkQualityRight, 0, 205) - 0) >= (i * 18)) {
			ofSetColor(127, 127, 127, 255);
		}
		else {
			ofSetColor(212, 212, 212, 255);

		}
		ofRect(pos_x + i * 4, 36 + (10 - i), 2, 2 + i);
	}
	for (i = 0; i < 8; i++) {
		pos_x = 485;
		if ((CLAMP(rawHIDobject->linkQualityAir, 0, 205) - 0) >= (i * 18)) {
			ofSetColor(127, 127, 127, 255);
		}
		else {
			ofSetColor(212, 212, 212, 255);

		}
		ofRect(pos_x + i * 4, 36 + (10 - i), 2, 2 + i);
	}


	/* template code */
	//gui.draw();
	//ofSetColor(color);
	//for(int i=0;i<number;i++){
	//	ofDrawCircle(ofGetWidth()*.5-size*((number-1)*0.5-i), ofGetHeight()*.5, size);
	//}
	/* template code */
}
