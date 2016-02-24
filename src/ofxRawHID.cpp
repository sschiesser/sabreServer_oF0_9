/*
 *  ofxRawHID.cpp
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
 *  @date 27/7/2014
 *
 *  @author Sebastien Schiesser
 *  @data 20160120
 *
 */

#include "ofApp.h"
#include "ofxRawHID.h"
#include <string.h>

 //--------------------------------------------------------------
 /* creation of a new class member */
ofxRawHID::ofxRawHID()
{
	// Variable initialization
	deviceSelected = false;
    deviceOpen = false;
	deviceUnplugged = true;
}

//--------------------------------------------------------------
/* destruction of a class member */
ofxRawHID::~ofxRawHID()
{
    clearDeviceList();
    closeDevice();
}

//--------------------------------------------------------------
/* try to open a selected HID device */
bool ofxRawHID::openDevice()
{
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	int res;

	//deviceSelected = true;
	/* If no device has been selected, nothing to open */
    if(deviceSelected) {
		int vendorID = selectedDeviceInfo.vendor_id;
		int productID = selectedDeviceInfo.product_id;
		int usagePage = selectedDeviceInfo.usage_page;
		int usage = selectedDeviceInfo.usage;
        
		if(appDebug) printf("[ofxRawHID::openDevice] Trying to open vid 0x%04x, pid 0x%04x, usagePage 0x%04x, usage 0x%04x\n", vendorID, productID, usagePage, usage);
		selectedDeviceInfo.handle = hid_open(vendorID, productID, NULL);
		if (!selectedDeviceInfo.handle) {
			if(appDebug) printf("[ofxRawHID::openDevice] Unable to open hid device!\n");
			deviceOpen = false;
			//return -1;
		}
		else {
			if(appDebug) printf("[ofxRawHID::openDevice] found HID device: 0x%04X : 0x%04X... handle is 0x%x\n", vendorID, productID, &selectedDeviceInfo.handle);
			deviceOpen = true;

			/* Read the Manufacturer String */
			wstr[0] = 0x0000;
			res = hid_get_manufacturer_string(selectedDeviceInfo.handle, wstr, MAX_STR);
			if (appDebug) {
				if (res < 0) printf("[ofxRawHID::openDevice] Unable to read manufacturer string\n");
				else printf("[ofxRawHID::openDevice] Manufacturer String: %ls\n", wstr);
			}

			/* Read the Product String */
			wstr[0] = 0x0000;
			res = hid_get_product_string(selectedDeviceInfo.handle, wstr, MAX_STR);
			if (appDebug) {
				if (res < 0) printf("[ofxRawHID::openDevice] Unable to read product string\n");
				else printf("[ofxRawHID::openDevice] Product String: %ls\n", wstr);
			}

			/* Read the Serial Number String */
			wstr[0] = 0x0000;
			res = hid_get_serial_number_string(selectedDeviceInfo.handle, wstr, MAX_STR);
			if (appDebug) {
				if (res < 0) printf("[ofxRawHID::openDevice] Unable to read serial number string\n");
				else printf("[ofxRawHID::openDevice] Serial Number String: (%d) %ls\n", wstr[0], wstr);
			}

			/* Read Indexed String 1 */
			wstr[0] = 0x0000;
			res = hid_get_indexed_string(selectedDeviceInfo.handle, 1, wstr, MAX_STR);
			if (appDebug) {
				if (res < 0) printf("[ofxRawHID::openDevice] Unable to read indexed string 1\n");
				else printf("[ofxRawHID::openDevice] Indexed String 1: %ls\n", wstr);
			}

			//return 0;
		}
    } else {
		if (appDebug) printf("[ofxRawHID::openDevice] no rawhid device selected\n");
		deviceOpen = false;
    }

	return deviceOpen;
}

//--------------------------------------------------------------
bool ofxRawHID::closeDevice()
{
    if(deviceOpen) {
		if(appDebug) printf("[ofxRawHID::closeDevice] Closing device & deleting handle (0x%x)\n", &selectedDeviceInfo.handle);
        hid_close(selectedDeviceInfo.handle);
		selectedDeviceInfo.handle = NULL;
        deviceOpen = false;
		return true;
	}
	else {
		return false;
	}
}

//--------------------------------------------------------------
bool ofxRawHID::isOpen()
{
	if (selectedDeviceInfo.handle) return true;
	else return false;
}

//--------------------------------------------------------------
int ofxRawHID::receivePacket()
{
	/* Put HID data into buf and receive the number of read bytes */
    int result = hid_read(selectedDeviceInfo.handle, buf, sizeof(buf));

	/* On error, result = -1 */
    //if (result < 0) {
    //    if(appDebug) printf("[ofxRawHID::receivePacket] error reading rawHID, device went offline\n");
    //}
    return result;
}

//--------------------------------------------------------------
int ofxRawHID::sendPacket()
{
	int result = hid_write(selectedDeviceInfo.handle, buf, HID_BUFFERSIZE);
    return result;
}

//--------------------------------------------------------------
int ofxRawHID::listDevices()
{
	/* NEW: hidapi!! */
	hid_device_info *devs, *cur_dev; // pointers to hid_device_info members
	ofxHIDDeviceInfo localDeviceInfo; // temporary info for one device before pushing to HIDdevices
	unsigned int dev_cnt = 0; // device counter

	/* Enumerate HID devices and display fetched informations */
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		if (appDebug) {
			printf("====================================\n");
			printf("Device Found:\n");
			printf("  type:          0x%04X  0x%04X\n", cur_dev->vendor_id, cur_dev->product_id);
			printf("  path:          %s\n", cur_dev->path);
			printf("  manufacturer:  %ls\n", cur_dev->manufacturer_string);
			printf("  product:       %ls\n", cur_dev->product_string);
			printf("  release:       0x%04X\n", cur_dev->release_number);
			printf("  interface:     %d\n", cur_dev->interface_number);
			printf("  usage page:    0x%04X\n", cur_dev->usage_page);
			printf("  usage:         0x%04X\n", cur_dev->usage);
			printf("====================================\n\n");
		}
		dev_cnt++;
		cur_dev = cur_dev->next;
	}
	if(appDebug) printf("[ofxRawHID::listDevices] #devices: %d ...sorting\n", dev_cnt);
    
	/* If some devices have been found... */
	if (dev_cnt > 0) {
		/* Clear the HIDdevice list */
		clearDeviceList();

		/* If the number of listed devices exceed HID_MAXNUMDEVICES, show an alert and return error */
		if (dev_cnt >= HID_MAXNUMDEVICES) {
			ofSystemAlertDialog("Too many HID devices! Unplug some...");
			return -1;
		}

		/* Re-scan all enumerated devices and push them to HIDdevices */
		cur_dev = devs;
		dev_cnt = 0;
		while (cur_dev) {
			localDeviceInfo.index = dev_cnt;
			localDeviceInfo.manufacturer_string = cur_dev->manufacturer_string;
			localDeviceInfo.product_string = cur_dev->product_string;
			localDeviceInfo.vendor_id = cur_dev->vendor_id;
			localDeviceInfo.product_id = cur_dev->product_id;
			HIDdevices.push_back(localDeviceInfo);
			//if(appDebug) printf("Device #%d: 0x%04X 0x%04X   %ls  %ls\n", dev_cnt, cur_dev->vendor_id, cur_dev->product_id, cur_dev->manufacturer_string, cur_dev->product_string);
			cur_dev = cur_dev->next;
			dev_cnt++;
		}

		/* free hid device enumerator 
		 * REMARK: Currently not freeing because of a seldom error.
		 *         'manufacturer_string' and 'product_string' members of
		 *         HIDdevices lose some character encodings and cannot
		 *         be used for comparison anymore. Since the enumeration
		 *         is (theoretically) done only once (or a few times) and
		 *         the resulting list is not huge, the memory resources
		 *         are not affected.
		 */
		//hid_free_enumeration(devs);

		/* Sort and remove duplicates from HIDdevices vector */
		std::sort(HIDdevices.begin(), HIDdevices.end());
		auto last = unique(HIDdevices.begin(), HIDdevices.end());
		HIDdevices.erase(last, HIDdevices.end());
		//if(appDebug) {
		//	vector <ofxHIDDeviceInfo>::iterator it;
		//	for (it = HIDdevices.begin(); it < HIDdevices.end(); it++) {
		//		printf("Sorted device  0x%04X  0x%04X   %ls  %ls\n", it->vendor_id, it->product_id, it->manufacturer_string, it->product_string);
		//	}
		//}

		return HIDdevices.size();
	} else {
		return -1;
	}
}

//--------------------------------------------------------------
bool ofxRawHID::clearDeviceList()
{
    HIDdevices.clear();
	if (appDebug) printf("[ofxRawHID::clearDeviceList] HIDdevice list cleared\n");
	return true;
}
