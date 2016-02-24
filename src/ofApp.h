#pragma once
//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxOsc.h"
#include "ofxOscParameterSync.h"
#include "ofxGui.h"
#include "threadedHID.h"
#include "sabreKeys.h"

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
		threadedHID * rawHIDobject;

		ofxOscReceiver receiver;
		ofxOscParameterSync sync;
		int receiveport;

		ofParameter<float> size;
		ofParameter<int> number;
		ofParameter<bool> check;
		ofParameterGroup parameters;
		ofParameter<ofColor> color;
		ofxPanel gui;

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
