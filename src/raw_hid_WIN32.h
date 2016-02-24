/*
 * Simple Raw HID functions for Linux - for use with Teensy RawHID example
 * http://www.pjrc.com/teensy/rawhid.html
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 *  rawhid_open - open 1 or more devices
 *  rawhid_recv - receive a packet
 *  rawhid_send - send a packet
 *  rawhid_close - close a device
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Version 1.0: Initial Release
 *
 * Version 1.1: adapted to C++ and provide device listing on OS X
 * @author: jasch www.jasch.ch
 * @date: 20140727
 *
 */

#pragma once
//#ifndef RAW_HID_WIN32_H
//#define RAW_HID_WIN32_H
#include "ofMain.h"

EXTERN_C {    
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
	//OSX
	//#include <unistd.h>
	//#include <IOKit/IOKitLib.h>
	//#include <IOKit/IOHIDLib.h>

	//WIN32
	#include <windows.h>
	#include <SetupAPI.h>
	#include <hidsdi.h>
	//#include <devguid.h>
	//#include <devpkey.h>
	#include <hidclass.h>
	#include <cfgmgr32.h>
	//#include <regstr.h>
	//#include <objbase.h>
	#include <initguid.h>
        
    #define BUFFER_SIZE 64
    #define MAX_NUM_DEVICES 32

    // a linked list of all opened HID devices, so the caller can simply refer to them by number
    typedef struct hid_struct hid_t;
	//OSX
    //typedef struct buffer_struct buffer_t;

    static hid_t *first_hid = NULL;
    static hid_t *last_hid = NULL;

	//OSX
	//static IOHIDDeviceRef * device_array;
	//WIN32
	static LPTSTR device_array;

    struct hid_struct {
		//OSX
        //IOHIDDeviceRef ref;

		//WIN32
		HANDLE handle;
		
        int open;

		//OSX
        //uint8_t buffer[BUFFER_SIZE];
        //buffer_t *first_buffer;
        //buffer_t *last_buffer;
		
        struct hid_struct *prev;
        struct hid_struct *next;
    };

	//OSX
    //struct buffer_struct {
    //    struct buffer_struct *next;
    //    uint32_t len;
    //    uint8_t buf[BUFFER_SIZE];
    //};

	//WIN32
	static HANDLE rx_event = NULL;
	static HANDLE tx_event = NULL;
	static CRITICAL_SECTION rx_mutex;
	static CRITICAL_SECTION tx_mutex;


    int rawhid_open(int max, int vid, int pid, int usage_page, int usage);
    int rawhid_recv(int num, void *buf, int len, int timeout);
    int rawhid_send(int num, void *buf, int len, int timeout);
    //void rawhid_close(int num);
    int rawhid_isOpen(int num);
    int rawhid_listdevices(int * index, long * vid, long * pid, char ** manufacturer_name, char ** product_name);

    // private functions, not intended to be used from outside this file
    static void add_hid(hid_t *h);
    static hid_t * get_hid(int num);
    static void free_all_hid(void);
    //static void rawhid_close(hid_t *hid);
	//WIN32
	void print_win32_err(void);

	//OSX
    //static void attach_callback(void *, IOReturn, void *, IOHIDDeviceRef);
    //static void detach_callback(void *, IOReturn, void *hid_mgr, IOHIDDeviceRef dev);
    //static void timeout_callback(CFRunLoopTimerRef, void *);
    //static void input_callback(void *, IOReturn, void *, IOHIDReportType,
    //                           uint32_t, uint8_t *, CFIndex);
    
    // public functions are defined in "raw_hid.h"
    
//#ifdef __cplusplus
}

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(a) \
	((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#ifdef DEFINE_DEVPROPKEY
#undef DEFINE_DEVPROPKEY
#endif
#ifdef INITGUID
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) \
	EXTERN_C const DEVPROPKEY DECLSPEC_SELECTANY name = {{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }, pid }
#else
#define DEFINE_DEVPROPKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) \
	EXTERN_C const DEVPROPKEY name
#endif // INITGUID

DEFINE_DEVPROPKEY(DEVPKEY_Device_BusReportedDeviceDesc, 0x540b947e, 0x8b40, 0x45bc, 0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2, 4);     // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_ContainerId, 0x8c7ed206, 0x3f8a, 0x4827, 0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c, 2);     // DEVPROP_TYPE_GUID
DEFINE_DEVPROPKEY(DEVPKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_DeviceDisplay_Category, 0x78c34fc8, 0x104a, 0x4aca, 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57, 90);    // DEVPROP_TYPE_STRING_LIST
DEFINE_DEVPROPKEY(DEVPKEY_Device_LocationInfo, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 15);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_Manufacturer, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 13);    // DEVPROP_TYPE_STRING
DEFINE_DEVPROPKEY(DEVPKEY_Device_SecuritySDS, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 26);    // DEVPROP_TYPE_SECURITY_DESCRIPTOR_STRING


