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
 * \file ofApp.cpp
 
 * \author Jan Schacher
 * \author Sebastien Schiesser
 
 * \date 2 March 2016
 
 * \version 0.99
 
 */

#include "ofApp.h"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

//--------------------------------------------------------------
void ofApp::setup() {
	appVersion = SERVER_VERSION;
	titleString = "sabreServer version " + appVersion; // Text to display on the server title bar
	if (appDebug) {
		printf("************************************\n");
		printf("**   sabreServer version %s    **\n", appVersion.c_str());
		printf("************************************\n\n");
	}
    
    appWindowSize.x = APP_WINDOW_WIDTH;
    appWindowSize.y = APP_WINDOW_HEIGHT;
    moduleWindowPos.x = 0;
    moduleWindowPos.y = 48;
    moduleWindowSize.x = 350;
    moduleWindowSize.y = APP_WINDOW_HEIGHT - moduleWindowPos.y;


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
    ImGuiIO io = ImGui::GetIO();
//    ImGui::GetIO().MouseDrawCursor = false;
    fontDisplay = io.Fonts->AddFontFromFileTTF(&ofToDataPath("lucidagrande.ttf")[0], 14.f);
    fontClock = io.Fonts->AddFontFromFileTTF(&ofToDataPath("lucidagrande.ttf")[0], 24.f);
    fontScale = io.Fonts->AddFontFromFileTTF(&ofToDataPath("lucidagrande.ttf")[0], 10.f);
    io.Fonts->GetTexDataAsRGBA32(&fontPx, &fontW, &fontH);

    gui.setup();
    backgroundColorMain = ofColor(114, 144, 154);
    
	if(!appDebug) ofSetEscapeQuitsApp(false); // disable ESC button to escape application
//	ofEnableAlphaBlending(); // turn on alpha blending
//	TTF.load("lucidagrande.ttf", 8, 1, 1, 0); // load font (must be in 'data' folder)
//	TTFsmall.load("lucidagrande.ttf", 8, 1, 0, 0);
//	texScreen.allocate(440, 700, GL_RGB); // allocate a texture to the given dimensions
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
    /* Calulate elapsed time when HID thread is running */
    if (rawHIDobject->rawHID.deviceOpen) {
        rawHIDobject->systemTimestamp = ofGetElapsedTimeMillis() - rawHIDobject->systemTimestampBase;
    }
    
	//receiveOSC();
    
    /* Warn if HID device has been unplugged */
	if (rawHIDobject->rawHID.deviceUnplugged) {
		ofSystemAlertDialog("Device not plugged in or not recognized!");
		rawHIDobject->rawHID.closeDevice(); // "disconnect" the HID device
		rawHIDobject->stop(); // stop the HID/OSCsender thread
		rawHIDobject->rawHID.deviceOpen = false; //
		rawHIDobject->rawHID.deviceSelected = false;
		rawHIDobject->rawHID.deviceUnplugged = false; // reset deviceUnplugged flag to avoid dialog window continuously coming
	}
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
			GUIdeviceInfo = "RawHID device: ERROR - unable to open!";

			retVal = false;
		}
		redrawFlag = 1;
	}
	else {
		if (appDebug) printf("[ofApp::startHID] No HID device selected...\n");
		retVal = false;
	}

    if(retVal) rawHIDobject->systemTimestampBase = ofGetElapsedTimeMillis();
    
	return retVal;
}

//--------------------------------------------------------------
void ofApp::stopHID()
{
    /* First stop the HID/OSCsender thread to avoid error message */
    while (rawHIDobject->isThreadRunning()) {
        rawHIDobject->stop();
#ifdef _WIN64
		Sleep((DWORD)(THREAD_STOPSLEEP_US / 1000));
#elif _WIN32
		Sleep((DWORD)(THREAD_STOPSLEEP_US / 1000));
#elif __APPLE__
        usleep(THREAD_STOPSLEEP_US);
#endif
        stopOSC();
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
		receiver.getNextMessage(m);
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
        if(appDebug) printf("[ofApp::readPrefs] Framerate: %d\n", framerate);
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
		rawHIDobject->calcLastSender();
//		if(appDebug) printf("calcLastSender exited\n");

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
bool ofApp::controlHID() {
    bool retVal = false;
    /* First check if a device is selected and ready to open */
    if (!rawHIDobject->rawHID.deviceSelected) {
        /* If no device slected, do an enumeration and selection */
        getHIDDeviceList();
        /* If device successfully selected, just change flag for next step */
        if (selectHIDdevice()) {
            rawHIDobject->rawHID.deviceSelected = true;
        }
        /* If no device selected, set 'unplugged' flag to recall the system alert dialog box and return false to re-set the button to 'not running' */
        else {
            rawHIDobject->rawHID.deviceSelected = false;
            rawHIDobject->rawHID.deviceUnplugged = true;
            retVal = false;
        }
    }
    
    /* If a compatible device has been found and is selected, check device open status */
    if(rawHIDobject->rawHID.deviceSelected) {
        /* Device open --> stopping */
        if (rawHIDobject->rawHID.deviceOpen) {
            stopHID();
            retVal = false;
        }
        /* Device closed --> starting */
        else {
            stopHID();
            int num = startOSC();
            if (num > 0) {
                /* If HID thread start did not work, stop previously opened OSC senders,
                 * and reset the flags in order to re-start an enumeration */
                if (!startHID()) {
                    stopOSC();
                    rawHIDobject->rawHID.deviceSelected = false;
                    rawHIDobject->rawHID.deviceUnplugged = true;
                    retVal = false;
                }
                retVal = true;
            }
        }
    }
    if(appDebug) printf("[ofApp::controlHID] returning %s\n", (retVal) ? "TRUE" : "FALSE");
    return retVal;
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
	ofDrawRectangle(295, 36, 295 + 124, 36 + 20);

	/* ----------------- *
	 * Start/stop button *
	 * ----------------- */
//	if (x > 270 && x < 394 && y > 4 && y < 25) {
//		if(appDebug) printf("\n------------------------------------\nstart/stop clicked\n - device selected? %s\n - device open? %s\n------------------------------------\n", (rawHIDobject->rawHID.deviceSelected ? "YES" : "NO"), (rawHIDobject->rawHID.deviceOpen ? "OPEN" : "CLOSED"));
//
//		/* First check if a device is selected and ready to open */
//		if (!rawHIDobject->rawHID.deviceSelected) {
//			/* If no device slected, do an enumeration and selection */
//			getHIDDeviceList();
//			/* If device successfully selected, just change flag for next step */
//			if (selectHIDdevice()) {
//				rawHIDobject->rawHID.deviceSelected = true;
//			}
//			/* If no device selected, set 'unplugged' flag to recall the system alert dialog box */
//			else {
//				rawHIDobject->rawHID.deviceSelected = false;
//				rawHIDobject->rawHID.deviceUnplugged = true;
//			}
//		}
//
//		/* If a compatible device has been found and is selected, check device open status */
//		if(rawHIDobject->rawHID.deviceSelected) {
//			/* Device open --> stopping */
//			if (rawHIDobject->rawHID.deviceOpen) {
//				stopHID();
//			}
//			/* Device closed --> starting */
//			else {
//				stopHID();
//				int num = startOSC();
//				if (num > 0) {
//					/* If HID thread start did not work, stop previously opened OSC senders,
//					 * and reset the flags in order to re-start an enumeration */
//					if (!startHID()) {
//						stopOSC();
//						rawHIDobject->rawHID.deviceSelected = false;
//						rawHIDobject->rawHID.deviceUnplugged = true;
//					}
//				}
//			}
//
//			drawValues = !hiddenValues;
//			rawHIDobject->drawValues = !hiddenValues;
//		}
//
//		windowChanged = true;
//		redrawFlag = 1;
//	}

	/* ---------------- *
	 * Show/hide values *
	 * ---------------- */
//	if (x > 397 && x < 520 && y > 3 && y < 24) {
//        /* Hide values */
//		if (drawValues != 0) {
//			if(appDebug) printf("[ofApp::mousePressed] Hiding values\n");
//			drawValues = false;
//			hiddenValues = true;
//			rawHIDobject->drawValues = 0;
//			rawHIDobject->calibrateSwitch = 0;
//		}
//        /* Show values */
//		else {
//			if (appDebug) printf("[ofApp::mousePressed] Showing values\n");
//			drawValues = true;
//			hiddenValues = false;
//			rawHIDobject->drawValues = 1;
//		}
//		windowChanged = true;
//		redrawFlag = 1;
//	}

	/* -------------------------- *
	 * Main keys calibrate button *
	 * -------------------------- */
	// ofDrawRectangle(295, 690, 124, 20);
//	if (x > 375 && x < 500 && y > 480 && y < 500) {
//		/* Switch off */
//		if (rawHIDobject->calibrateSwitch != 0) {
//			rawHIDobject->calibrateSwitch = 0;
//			rawHIDobject->calibrateSingle = 0;
//			for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
//				rawHIDobject->calibrate[i] = 0;
//			}
//			writeScaling();
//
//		}
//		/* Switch on */
//		else {
//			rawHIDobject->calibrateSwitch = 1;
//			rawHIDobject->calibrateSingle = 1;
//			//            for(i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
//			//				rawHIDobject->calibrate[i] = 1;
//			//			}
//			//            
//			//			resetCalibrate();
//		}
//		//		printf("rawHIDobject->calibrateSwitch is %d\n", rawHIDobject->calibrate);
//	}
//	if (rawHIDobject->calibrateSwitch) {
//		// click in calibrateAll == calibrateSingle flag :: conditional on main calibrate button
//		if (x > 375 && x < 500 && y > 458 && y < 478) {
//			if (rawHIDobject->calibrateSingle != 0) { // switch off
//				rawHIDobject->calibrateSingle = 0;
//				// reset all to zero
//				for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
//					rawHIDobject->calibrate[i] = 1;
//				}
//				resetCalibrate();
//			}
//			else { // switch on
//				rawHIDobject->calibrateSingle = 1;
//				for (i = 0; i < SABRE_MAXNUMMESSAGES; i++) {
//					rawHIDobject->calibrate[i] = 0;
//				}
//				writeScaling();
//
//			}
//			//            printf("rawHIDobject->calibrateSingle is %d\n", rawHIDobject->calibrateSingle);
//		}
//
//		if (x > 346 && x < 362) {
//			int yy = y - 57;
//			i = yy / 18;
//			//            printf("clicked inside calibrate toggle Nr. %d at pos %d %d\n",i, x, y);
//			if (rawHIDobject->calibrate[i] == 0) {
//				rawHIDobject->calibrate[i] = 1;
//				resetSingleCalibrate(i);
//			}
//			else {
//				rawHIDobject->calibrate[i] = 0;
//				writeScaling();
//			}
//		}
//
//		if (x > 375 && x < 500 && y > 436 && y < 456) {
//			//            printf("clicked in reset Calibration");
//			for (int i = 0; i < rawHIDobject->numKeyAddr; i++)
//			{
//				rawHIDobject->keys[i].minimum = 0;
//				rawHIDobject->keys[i].maximum = 1023;
//			}
//		}
//
//	}

	/* -------------------------------- *
	 * Calibrate pressure min/max range *
	 * -------------------------------- */
//	if (x > 502 && x < 627 && y > 480 && y < 500) {
//		if (rawHIDobject->airValue.calibratePressureRange == true) {
//			rawHIDobject->airValue.calibratePressureRange = false;
//			//            printf("finished calibrating air with values minimum %f maximum %f\n",rawHIDobject->airValue.minimum, rawHIDobject->airValue.maximum);
//
//			writeScaling();
//		}
//		else {
//			rawHIDobject->airValue.calibratePressureRange = true;
//			resetAirCalibrate();
//		}
//	}

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
    ofSetBackgroundColor(backgroundColorMain);
    gui.begin();
    // Main window
    {
        ImGui::SetNextWindowSize(ImVec2(ofGetWidth(), 48));
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGuiWindowFlags winFlagsMain = 0;
        winFlagsMain |= ImGuiWindowFlags_NoMove;
        winFlagsMain |= ImGuiWindowFlags_NoResize;
        winFlagsMain |= ImGuiWindowFlags_NoTitleBar;
        bool showWindowMain = true;
        ImGui::Begin("Main Window", &showWindowMain, winFlagsMain);
        ImGui::BeginGroup();
        ImGui::BeginGroup();
        /* HID device info */
        ImGui::Text(GUIdeviceInfo.c_str());
        /* Framerate */
        ImGui::Text("Current framerate: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::EndGroup();
        /* Start/stop button */
        ImGui::SameLine(286);
        bool running = rawHIDobject->rawHID.deviceOpen;
        ImGui::PushFont(fontClock);
        if(running) { // thread currently running...
            ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1/7.0f, 0.8f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(1/7.0f, 0.9f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(1/7.0f, 1.0f, 1.0f));
            if(ImGui::Button("Stop")) {
                controlHID();
            }
        }
        else { // thread currently stopped, ready to start
            ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1/7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(1/7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImColor::HSV(1/7.0f, 0.8f, 0.8f));
            if(ImGui::Button("Start")) {
                controlHID();
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::PopFont();
        /* Timecode */
        ImGui::SameLine(360);
        long ts = rawHIDobject->systemTimestamp;
        long ms = ts % 1000;
        long s = ((ts - ms)/1000) % 60;
        long m = ((((ts - ms)/1000) - s)/60) % 60;
        long h = (((((ts - ms)/1000) - s)/60) - m)/60;
        ImGui::PushFont(fontClock);
        ImGui::Text("Systime: %ldh %02ldm %02lds %03ld", h, m, s, ms);
        ImGui::PopFont();
        
        ImGui::EndGroup();
        ImGui::End();
    }
    
    // 1. module window
    {
        // General window settings
        ImGui::SetNextWindowSize(moduleWindowSize);
        ImGui::SetNextWindowPos(moduleWindowPos);
        ImGuiWindowFlags winFlagsMod = 0;
        winFlagsMod |= ImGuiWindowFlags_NoMove;
        winFlagsMod |= ImGuiWindowFlags_NoResize;
        bool showWindowMod = false;
        ImGui::Begin("Module #1", &showWindowMod, winFlagsMod);

        // Non-collapsable elements
        {
            // Link quality display
            ImGui::Text("Link:");
            ImGui::SameLine();
            float link = (float)rawHIDobject->linkQualityLeft/256.0f;
            char buf[32];
            sprintf(buf, "%d/%d", (int)((link+0.09)*10), 10);
            ImGui::PushItemWidth(100);
            ImGui::ProgressBar(link, ImVec2(0.f, 0.f), buf);
            ImGui::PopItemWidth();
            // Battery level display
            ImGui::SameLine();
            ImGui::Text("Battery:");
            ImGui::SameLine(234);
            float battery = (float)rawHIDobject->batteryLevelRight;
            ImGui::PushItemWidth(100);
            ImGui::ProgressBar(battery, ImVec2(0.0f, 0.0f));
            ImGui::PopItemWidth();
        }
        
        // OSC header
        {
            if(ImGui::CollapsingHeader("OSC")) {
                string label;
                char txt[OSC_NUMSENDERS][128];
                int in[OSC_NUMSENDERS];
                bool en[OSC_NUMSENDERS];
                string ID;
                for (int i = 0; i < OSC_NUMSENDERS; i++) {
                    // label
                    label = "Sender#" + ofToString(i+1) + ":";
                    ImGui::Text(label.c_str());
                    // IP
                    ImGui::Text("IP:"); ImGui::SameLine();
                    strcpy(txt[i], rawHIDobject->sendIP[i].c_str());
                    ID = "##txt" + ofToString(i);
                    ImGui::PushItemWidth(100);
                    if(ImGui::InputText(ID.c_str(), txt[i], IM_ARRAYSIZE(txt[i]), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        rawHIDobject->sendIP[i] = ofToString(txt[i]);
                        if(appDebug) printf("New IP: %s\n", rawHIDobject->sendIP[i].c_str());
                    }
                    ImGui::PopItemWidth();
                    // Port
                    ImGui::SameLine();
                    in[i] = rawHIDobject->sendport[i];
                    ImGui::Text("Port:"); ImGui::SameLine();
                    ID = "##in" + ofToString(i);
                    ImGui::PushItemWidth(84);
                    if(ImGui::InputInt(ID.c_str(), &in[i], 1, 1, ImGuiInputTextFlags_EnterReturnsTrue)) {
                        rawHIDobject->sendport[i] = in[i];
                        if(appDebug) printf("New port: %d\n", rawHIDobject->sendport[i]);
                    }
                    ImGui::PopItemWidth();
                    // Enable
                    ImGui::SameLine();
                    en[i] = rawHIDobject->senderActive[i];
                    ImGui::Text("En:"); ImGui::SameLine();
                    ID = "##en" + ofToString(i);
                    if(ImGui::Checkbox(ID.c_str(), &en[i])) {
                        rawHIDobject->senderActive[i] = en[i];
                        if(appDebug) printf("Sender activated: %s\n", (rawHIDobject->senderActive[i]) ? "YES" : "NO");
                    }
                }
            }
        }
        
        // Accelerometer header
        {
            if(ImGui::CollapsingHeader("Accelerometer")) {
                static float accel[4];
                static ImVector<float> accelPlot[4];
                static int accelPlotOffset[4] = {0, 0, 0, 0};
                // Fill the 3 accelerometer values
                for(int i = 0; i < 4; i++) {
                    if(accelPlot[i].empty()) {
                        accelPlot[i].resize(50);
                        memset(accelPlot[i].Data, 0, accelPlot[i].Size * sizeof(float));
                    }
                    accel[i] = ( (i >= 3) ? ((float)rawHIDobject->summedIMU[0]) : ((float)rawHIDobject->IMU[i]) );
                    accelPlot[i][accelPlotOffset[i]] =  accel[i];
                    accelPlotOffset[i] = (accelPlotOffset[i] + 1) % accelPlot[i].Size;
                }

                // X
                ImGui::Text("X"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##accelX", &accel[0], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##accelXPlot", accelPlot[0].Data, accelPlot[0].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Y
                ImGui::Text("Y"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##accelX", &accel[1], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##accelXPlot", accelPlot[1].Data, accelPlot[1].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Z
                ImGui::Text("Z"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##accelX", &accel[2], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##accelXPlot", accelPlot[2].Data, accelPlot[2].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Sum
                ImGui::Text("Sum"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##accelSum", &accel[3], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##accelSi,Plot", accelPlot[3].Data, accelPlot[3].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
            }
        }
        
        // Gyroscope header
        {
            if(ImGui::CollapsingHeader("Gyroscope")) {
                static float gyro[4];
                static ImVector<float> gyroPlot[4];
                static int gyroPlotOffset[4] = {0, 0, 0, 0};
                // Fill the 3 accelerometer values
                for(int i = 0; i < 4; i++) {
                    if(gyroPlot[i].empty()) {
                        gyroPlot[i].resize(50);
                        memset(gyroPlot[i].Data, 0, gyroPlot[i].Size * sizeof(float));
                    }
                    gyro[i] = ( (i >= 3) ? ((float)rawHIDobject->summedIMU[1]) : ((float)rawHIDobject->IMU[i+3]) );
                    gyroPlot[i][gyroPlotOffset[i]] =  gyro[i];
                    gyroPlotOffset[i] = (gyroPlotOffset[i] + 1) % gyroPlot[i].Size;
                }
                // X
                ImGui::Text("X"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##gyroX", &gyro[0], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##gyroXPlot", gyroPlot[0].Data, gyroPlot[0].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Y
                ImGui::Text("Y"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##gyroY", &gyro[1], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##gyroYPlot", gyroPlot[1].Data, gyroPlot[1].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Z
                ImGui::Text("Z"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##gyroZ", &gyro[2], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##gyroZPlot", gyroPlot[2].Data, gyroPlot[2].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
                // Sum
                ImGui::Text("Sum"); ImGui::SameLine(40);
                ImGui::PushItemWidth(140);
                ImGui::SliderFloat("##gyroSum", &gyro[3], 0.0f, 1.0f); ImGui::SameLine();
                ImGui::PlotLines("##gyroSumPlot", gyroPlot[3].Data, gyroPlot[3].Size, 0, "", 0.0f, 1.0f, ImVec2(0, 20));
                ImGui::PopItemWidth();
            }
        }
        
        // Heading/tilt header
        {
            if(ImGui::CollapsingHeader("Heading/tilt")) {
                
            }
        }
        
        // AHRS header
        {
            if(ImGui::CollapsingHeader("AHRS")) {
                
            }
        }
        
        // Buttons header
        {
            if(ImGui::CollapsingHeader("Buttons")) {
                ImGui::Text(""); ImGui::SameLine(80);
                ImGui::Checkbox("##but1", &rawHIDobject->button[0]); ImGui::SameLine(200);
                ImGui::Checkbox("##but2", &rawHIDobject->button[1]);
                ImGui::Text(""); ImGui::SameLine(85);
                ImGui::Text("1"); ImGui::SameLine(205);
                ImGui::Text("2");
            }
        }
        
        // Pressure header
        {
            if(ImGui::CollapsingHeader("Pressure")) {
                static float press;
                static ImVector<float> pVals;
                static int pValsOffset = 0;
                if (pVals.empty()) {
                    pVals.resize(100);
                    memset(pVals.Data, 0, pVals.Size * sizeof(float));
                }
                press = (float)rawHIDobject->air[0];
                pVals[pValsOffset] = (float)rawHIDobject->air[0];
                pValsOffset = (pValsOffset + 1) % pVals.Size;
                ImGui::BeginGroup();
                ImGui::BeginGroup();
                ImGui::Text("Pressure:");
                ImGui::Text("%.0f mbar", rawHIDobject->air[0]);
                ImGui::EndGroup(); ImGui::SameLine(80);
                ImGui::VSliderFloat("##pressSlider", ImVec2(12, 80), &press, -200.0f, 200.0f, ""); ImGui::SameLine(92);
                ImGui::PlotLines("##pressPlot", pVals.Data, pVals.Size, 0 , "Pressure (mbar)", -200.0f, 200.0f, ImVec2(252, 80));
                ImGui::EndGroup();
            }
        }
        
        // Temperature header
        {
            if(ImGui::CollapsingHeader("Temperature")) {
                static ImVector<float> tiVals;
                static int tiValsOffset = 0;
                if (tiVals.empty()) {
                    tiVals.resize(100);
                    memset(tiVals.Data, 0, tiVals.Size * sizeof(float));
                }
                tiVals[tiValsOffset] = (float)rawHIDobject->air[1];
                tiValsOffset = (tiValsOffset + 1) % tiVals.Size;
                ImGui::BeginGroup();
                ImGui::BeginGroup();
                ImGui::Text("Temp_i:");
                ImGui::Text("%.2f °C", rawHIDobject->air[1]);
                ImGui::EndGroup(); ImGui::SameLine(80);
                ImGui::PlotLines("##temp_i", tiVals.Data, tiVals.Size, 0, "Temperature (°C)", 20.0f, 32.0f, ImVec2(264, 80));
                ImGui::EndGroup();
                
                ImGui::Spacing();
                ImGui::Spacing();
                
                static ImVector<float> teVals;
                static int teValsOffset = 0;
                if (teVals.empty()) {
                    teVals.resize(100);
                    memset(teVals.Data, 0, teVals.Size * sizeof(float));
                }
                teVals[teValsOffset] = (float)rawHIDobject->IMU[9];
                teValsOffset = (teValsOffset + 1) % teVals.Size;
                ImGui::BeginGroup();
                ImGui::BeginGroup();
                ImGui::Text("Temp_e:");
                ImGui::Text("%.2f °C", rawHIDobject->IMU[9]);
                ImGui::EndGroup(); ImGui::SameLine(80);
                ImGui::PlotLines("##temp_e", teVals.Data, teVals.Size, 0, "Temperature (°C)", 20.0f, 32.0f, ImVec2(264, 80));
                ImGui::EndGroup();
            }
        }
        
        // Packet transmition header
        {
            if(ImGui::CollapsingHeader("Packets transmition")) {
                static float plotRange = 40.f;
                static float plotDiv = plotRange/5.f;
                static ImVector<float> tslVals;
                static int tslValsOffset = 0;
                if (tslVals.empty()) {
                    tslVals.resize(100);
                    memset(tslVals.Data, 0, tslVals.Size * sizeof(float));
                }
                tslVals[tslValsOffset] = (float)rawHIDobject->deltaTimeL/1000;
//                tslVals[tslValsOffset] = 16.f;
                tslValsOffset = (tslValsOffset + 1) % tslVals.Size;
                ImGui::BeginGroup();
                ImGui::BeginGroup();
                ImGui::Text("Delta:");
                ImGui::Text("%.1f ms", ((float)rawHIDobject->deltaTimeL/1000));
                ImGui::EndGroup(); ImGui::SameLine(80);
                ImGui::PlotHistogram("##deltaLeft", tslVals.Data, tslVals.Size, 0, "Delta left (ms)", 0.0f, plotRange, ImVec2(0, 80)); ImGui::SameLine();
                ImGui::PushFont(fontScale);
                ImGui::BeginGroup();
                ImGui::Text("");
                ImGui::Text("- %.0f", (4*plotDiv));
                ImGui::Text("- %.0f", (3*plotDiv));
                ImGui::Text("- %.0f", (2*plotDiv));
                ImGui::Text("- %.0f", plotDiv);
                ImGui::Text("- %.0f", 0.f);
                ImGui::EndGroup();
                ImGui::PopFont();
                ImGui::EndGroup();
                ImGui::Spacing();
                ImGui::Spacing();
                
                static ImVector<float> tsrVals;
                static int tsrValsOffset = 0;
                if (tsrVals.empty()) {
                    tsrVals.resize(100);
                    memset(tsrVals.Data, 0, tsrVals.Size * sizeof(float));
                }
                tsrVals[tsrValsOffset] = ((float)rawHIDobject->deltaTimeR/1000);
                tsrValsOffset = (tsrValsOffset + 1) % tsrVals.Size;
                ImGui::BeginGroup();
                ImGui::BeginGroup();
                ImGui::Text("Delta:");
                ImGui::Text("%.1f ms", ((float)rawHIDobject->deltaTimeR/1000));
                ImGui::EndGroup(); ImGui::SameLine(80);
                ImGui::PlotHistogram("##deltaRight", tsrVals.Data, tsrVals.Size, 0, "Delta right (ms)", 0.0f, plotRange, ImVec2(0, 80)); ImGui::SameLine();
                ImGui::PushFont(fontScale);
                ImGui::BeginGroup();
                ImGui::Text("");
                ImGui::Text("- %.0f", (4*plotDiv));
                ImGui::Text("- %.0f", (3*plotDiv));
                ImGui::Text("- %.0f", (2*plotDiv));
                ImGui::Text("- %.0f", plotDiv);
                ImGui::Text("- %.0f", 0.f);
                ImGui::EndGroup();
                ImGui::PopFont();
                ImGui::EndGroup();
            }
        }
        
        ImGui::End();
    }
    gui.end();
}
