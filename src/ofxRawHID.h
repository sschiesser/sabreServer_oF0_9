/*
 *  ofxRawHID.h
 *  openFrameworks addon for PJRC rawHID devices
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
 *  @@date 27/7/2014
 *
 *  @author Sebastien Schiesser
 *  @data 20160120
 *
 */

#pragma once

//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>

#include "ofMain.h"


#define HID_BUFFERSIZE 64 // max buffer size for read & write
#define HID_MAXNUMDEVICES 32 // max number of device to be enumerated

// OS-specific includes
extern "C" {
	#ifdef _WIN64 // windows (64-bit)
        #include <hidapi.h>
	#elif _WIN32 // windows (32-bit)
        #include <hidapi.h>
	#elif __APPLE__ // apple
        #include "hidapi.h"
//   #include "TargetConditionals.h"
//   #include <sys/ioctl.h>
//   #include <termios.h>
//   #if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
//	// define something for simulator   
//   #elif TARGET_OS_IPHONE
//	// define something for iphone  
//   #else
//   #define TARGET_OS_OSX 1
//	// define something for OSX
//   #endif
	#elif __linux // linux
//   #include <sys/ioctl.h>
//   #include <termios.h>
	#elif __unix // all unices not caught above
//   #include <sys/ioctl.h>
//   #include <termios.h>
	#elif __posix // POSIX
//   #include <sys/ioctl.h>
//   #include <termios.h>
	#endif
}

/** ---------------------- *
 ** ofxHIDDeviceInfo class *
 ** ---------------------- *
 ** - inherits hid_device_info struct 
 ** - collect all information attached to one HID device */
class ofxHIDDeviceInfo: public hid_device_info {
public:
	/* ---------------- *
	 * MEMBER FUNCTIONS *
	 * ---------------- */
	friend bool operator<(const hid_device_info& c1, const hid_device_info& c2) {
		return c1.product_id < c2.product_id;
	}

	friend bool operator==(const hid_device_info& c1, const hid_device_info& c2) {
		return c1.product_id == c2.product_id;
	}

	/* ---------------- *
	 * MEMBER VARIABLES *
	 * ---------------- */
	hid_device *handle; // pointer to a hid_device item
    int index; // device index set during enumeration
};


/** --------------- *
 ** ofxRawHID class *
 ** --------------- */
class ofxRawHID
{
public:
	/* ---------------- *
	 * MEMBER FUNCTIONS *
	 * ---------------- */
    ofxRawHID();
	~ofxRawHID();

    bool openDevice();
    bool closeDevice();
    bool isOpen();
    int receivePacket();
    int sendPacket();
    int listDevices();
    bool clearDeviceList();
    
	/* ---------------- *
	 * MEMBER VARIABLES *
	 * ---------------- */
	vector <ofxHIDDeviceInfo> HIDdevices; // vector of all enumerated HID devices
    ofxHIDDeviceInfo selectedDeviceInfo; // information for one device
    bool deviceSelected; // (flag) one device found in the list and selected
    bool deviceOpen; // (flag) selected device open
	bool deviceUnplugged; // (flag) device unplugged
    unsigned char buf[HID_BUFFERSIZE]; // data buffer for HID read & write transactions
    
private:
};