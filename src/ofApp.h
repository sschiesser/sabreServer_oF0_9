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
 * \file ofApp.h
 
 * \author Jan Schacher
 * \author Sebastien Schiesser
 
 * \date 2 March 2016
 
 * \version 0.99
 
 */

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxOscParameterSync.h"
#include "ofxGui.h"
#include "ofxImGui.h"
#include "threadedHID.h"
#include "sabreKeys.h"

#define APP_WINDOW_WIDTH 1054
#define APP_WINDOW_HEIGHT 768
#define APP_MAX_MODULES 3


/* DEFAULT SABRE RECEIVER HID VALUES
 * =================================
 * Change either here (static) or in the XML conf file (dynamic)
 * Note: the constant values are 8-bit encoded. Conversion is
 *       needed to fit with some wchar_t variables.
 */
#define SABRE_VENDORID "1C57"
#define SABRE_PRODUCTID "5ABE"
#define SABRE_MANUFACTURERSTRING "ICST"
#define SABRE_PRODUCTSTRING "SABRe"
#define SABRE_USAGE 0x0100
#define SABRE_USAGEPAGE 0xFFAB
/* =================================
 */

/* MODIFY AT EACH VERSION CHANGE!!
 * =================================
 */
#define SERVER_VERSION "0.99"
/* =================================
 */

/* Project-wide debug flag */
#ifdef _DEBUG
	const bool appDebug = true;
#else
	const bool appDebug = false;
#endif

class ofApp : public ofBaseApp
{
	public:
		/* ---------------- *
		 * MEMBER FUNCTIONS *
		 * ---------------- */
		void setup();
		void update();
		void draw();
		void exit();

		// ofxRawHID
		bool startHID();
		void stopHID();
		void getHIDDeviceList();
		bool selectHIDdevice();

		// OSC sender
		int startOSC();
		void stopOSC();

		// XML preferences
		void writePrefs();
		void dumpPrefs();
		bool readPrefs();
		bool readMidicodes();

		void resetCalibrate();
		void resetSingleCalibrate(int i);
		void resetAirCalibrate();

		void writeScaling();
    
    bool controlHID();

		void receiveOSC();

		// ofxGui
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		/* ---------------- *
		 * MEMBER VARIABLES *
		 * ---------------- */
    
    /* ofxImGui tools */
    ofxImGui gui;
    ImVec4 backgroundColorMain;
    ImVec4 backgroundColorMod[APP_MAX_MODULES];
    ImVec2 appWindowSize;
    ImVec2 moduleWindowSize;
    ImVec2 moduleWindowPos;
    ImFont *fontClock;
    ImFont *fontDisplay;
    ImFont *fontScale;
    unsigned char* fontPx;
    int fontW, fontH;
    
		threadedHID * rawHIDobject;

		ofxOscReceiver receiver;
		ofxOscParameterSync sync;
		int receiveport;

//		ofParameter<float> size;
//		ofParameter<int> number;
//		ofParameter<bool> check;
//		ofParameterGroup parameters;
//		ofParameter<ofColor> color;
//		ofxPanel gui;

		// ofxXmlSettings
		ofxXmlSettings XML;
		ofxXmlSettings XMLmidi;
		string str1;

		// ofTrueTypeFont
		ofTrueTypeFont TTF;
		ofTrueTypeFont TTFsmall;

		// ofTexture
		ofTexture texScreen;

		int		framerate;
		bool	status;
		int		display;
		bool	windowChanged;

		bool	prefStatus;
		string	GUIdeviceInfo;
		string	GUIoscInfo[OSC_NUMSENDERS];
		int     whichStatus;
		string	GUIfIOstatus;
		string	tempStr[1024];

		string appVersion;
		string titleString;

		bool	senderStatus;
		bool	drawValues;
		bool	hiddenValues;

		float	timeOut;
		float	lastTime;

		bool	runOnce;
		float	runOnceDelay;
		float	runOnceStart;

		bool	redrawFlag;
		float	redrawInterval;
		float	lastRedraw;
		float   redrawValues[6];
		bool	firstflag;
		bool	drawTex;

		//bool	menuState;
		int		numMenuItems;
		ofPoint menuClickPos;
		bool    sendOscToSC;

};
