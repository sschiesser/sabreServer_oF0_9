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
 * \file ofxRawHID.h
 
 * \author Jan Schacher
 * \author Sebastien Schiesser
 
 * \date 2 March 2016
 
 * \version 0.99
 
 */


/* --------------------------------------------------------------
 *     INCLUDES
 * -------------------------------------------------------------- */
#include "ofMain.h"

/* OS-specific includes */
extern "C" {
    #ifdef _WIN64 // windows (64-bit)
        #include <hidapi.h>
    #elif _WIN32 // windows (32-bit)
        #include <hidapi.h>
    #elif __APPLE__ // apple
        #include "hidapi.h"
    
        #if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
        #elif TARGET_OS_IPHONE
        #else
        #define TARGET_OS_OSX 1
        #endif
    
    #elif __linux // linux
    #elif __unix // all unices not caught above
    #elif __posix // POSIX
    #endif
}


/* --------------------------------------------------------------
 *     MACROS
 * -------------------------------------------------------------- */
#define HID_BUFFERSIZE 64 // max buffer size for read & write
#define HID_MAXNUMDEVICES 32 // max number of device to be enumerated


/* --------------------------------------------------------------
 *     CLASS DECLARATIONS
 * -------------------------------------------------------------- */
/**
 * \brief ofxHIDDeviceInfo class
 *        Wrapper for the hid_device_info struct
 *
 * Class properties:
 * - inherits hid_device_info struct
 * - adds some operators to allow sorting
 * - adds device index and handle pointer
 */
class ofxHIDDeviceInfo: public hid_device_info {
public:
	friend bool operator<(const hid_device_info& c1, const hid_device_info& c2) {
		return c1.product_id < c2.product_id;
	}

	friend bool operator==(const hid_device_info& c1, const hid_device_info& c2) {
		return c1.product_id == c2.product_id;
	}

	hid_device *handle; // pointer to a hid_device item
    int index; // device index set during enumeration
};


/**
 * \brief ofxRawHID class
 *        Contains attributes of the connected HID devices
 *
 * Class components:
 * - functions to handle with devices
 * - vector of all enumerated devices
 * - information about selected device (ofxHIDDeviceInfo)
 * - read/write data buffer
 * - flags to device status
 */
class ofxRawHID
{
public:
    ofxRawHID();
	~ofxRawHID();

    bool openDevice();
    bool closeDevice();
    bool isOpen();
    int receivePacket();
    int sendPacket();
    int listDevices();
    bool clearDeviceList();
    
	vector <ofxHIDDeviceInfo> HIDdevices; // vector of all enumerated HID devices
    ofxHIDDeviceInfo selectedDeviceInfo; // information for one device
    unsigned char buf[HID_BUFFERSIZE]; // data buffer for HID read & write transactions
    bool deviceSelected; // (flag) one device found in the list and selected
    bool deviceOpen; // (flag) selected device open
	bool deviceUnplugged; // (flag) device unplugged
    
private:
};