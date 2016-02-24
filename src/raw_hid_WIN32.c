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

#include "raw_hid_WIN32.h"
#include "ofApp.h"


/////////////
//
//  rawhid_recv - receive a packet
//    Inputs:
//	num = device to receive from (zero based)
//	buf = buffer to receive packet
//	len = buffer's size
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes received, or -1 on error
//
int rawhid_recv(int num, void *buf, int len, int timeout)
{
//	hid_t *hid;
//	unsigned char tmpbuf[516];
//	OVERLAPPED ov;
//	DWORD n, r;
//
//	if (sizeof(tmpbuf) < len + 1) {
//		return -1;
//	}
//	hid = get_hid(num);
//	if (!hid || !hid->open) {
//		return -1;
//	}
//	EnterCriticalSection(&rx_mutex);
//	ResetEvent(&rx_event);
//	memset(&ov, 0, sizeof(ov));
//	ov.hEvent = rx_event;
//	if (!ReadFile(hid->handle, tmpbuf, len + 1, NULL, &ov)) {
//		if (GetLastError() != ERROR_IO_PENDING) {
//			goto return_error;
//		}
//		r = WaitForSingleObject(rx_event, timeout);
//		if (r == WAIT_TIMEOUT) {
//			goto return_timeout;
//		}
//		if (r != WAIT_OBJECT_0) {
//			goto return_error;
//		}
//	}
//	if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE)) {
//		goto return_error;
//	}
//	LeaveCriticalSection(&rx_mutex);
//	if (n <= 0) {
//		return -1;
//	}
//	n--;
//	if (n > len) {
//		n = len;
//	}
//	memcpy(buf, tmpbuf + 1, n);
//	return n;
//
//return_timeout:
//	CancelIo(hid->handle);
//	LeaveCriticalSection(&rx_mutex);
//	return 0;
//return_error:
//	print_win32_err();
//	LeaveCriticalSection(&rx_mutex);
//	return -1;

	return 0;
}

//  rawhid_send - send a packet
//    Inputs:
//	num = device to transmit to (zero based)
//	buf = buffer containing packet to send
//	len = number of bytes to transmit
//	timeout = time to wait, in milliseconds
//    Output:
//	number of bytes sent, or -1 on error
//
int rawhid_send(int num, void *buf, int len, int timeout)
{
//	hid_t *hid;
//	unsigned char tmpbuf[516];
//	OVERLAPPED ov;
//	DWORD n, r;
//
//	if (sizeof(tmpbuf) < len + 1) return -1;
//	hid = get_hid(num);
//	if (!hid || !hid->open) {
//		return -1;
//	}
//	EnterCriticalSection(&tx_mutex);
//	ResetEvent(&tx_event);
//	memset(&ov, 0, sizeof(ov));
//	ov.hEvent = tx_event;
//	tmpbuf[0] = 0;
//	memcpy(tmpbuf + 1, buf, len);
//	if (!WriteFile(hid->handle, tmpbuf, len + 1, NULL, &ov)) {
//		if (GetLastError() != ERROR_IO_PENDING) goto return_error;
//		r = WaitForSingleObject(tx_event, timeout);
//		if (r == WAIT_TIMEOUT) goto return_timeout;
//		if (r != WAIT_OBJECT_0) goto return_error;
//	}
//	if (!GetOverlappedResult(hid->handle, &ov, &n, FALSE)) {
//		goto return_error;
//	}
//	LeaveCriticalSection(&tx_mutex);
//	if (n <= 0) {
//		return -1;
//	}
//	return n - 1;
//return_timeout:
//	CancelIo(hid->handle);
//	LeaveCriticalSection(&tx_mutex);
//	return 0;
//return_error:
//	print_win32_err();
//	LeaveCriticalSection(&tx_mutex);
//	return -1;

	return 0;
}

//  rawhid_open - open 1 or more devices
//
//    Inputs:
//	max = maximum number of devices to open
//	vid = Vendor ID, or -1 if any
//	pid = Product ID, or -1 if any
//	usage_page = top level usage page, or -1 if any
//	usage = top level usage number, or -1 if any
//    Output:
//	actual number of devices opened
//
int rawhid_open(int max, int vid, int pid, int usage_page, int usage)
{
	GUID guid;
	HDEVINFO info;
	DWORD index = 0, reqd_size;
	SP_DEVICE_INTERFACE_DATA iface;
	SP_DEVICE_INTERFACE_DETAIL_DATA *details;
	HIDD_ATTRIBUTES attrib;
	PHIDP_PREPARSED_DATA hid_data;
	HIDP_CAPS capabilities;
	HANDLE h;
	BOOL ret;
	hid_t *hid;
	int count = 0;

	if (first_hid) {
		free_all_hid();
	}
	if (max < 1) {
		return 0;
	}
	if (!rx_event) {
		//rx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
		//tx_event = CreateEvent(NULL, TRUE, TRUE, NULL);
		//InitializeCriticalSection(&rx_mutex);
		//InitializeCriticalSection(&tx_mutex);
	}
	HidD_GetHidGuid(&guid);
	ofLog(OF_LOG_NOTICE, "HID GUID: 0x%08lX, 0x%04X, 0x%04X, 0x%016llX\r\n",
		guid.Data1, guid.Data2, guid.Data3, guid.Data4);
	info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (info == INVALID_HANDLE_VALUE) {
		//ofLog(OF_LOG_ERROR, "Invalid handle value returned while trying to get class devices\r\n");
		return 0;
	}
	for (index = 0; 1; index++) {
		iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		ret = SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface);
		if (!ret) {
			ofLog(OF_LOG_ERROR, "Error while trying to enumerate device interfaces\r\n");
			return count;
		}
		SetupDiGetInterfaceDeviceDetail(info, &iface, NULL, 0, &reqd_size, NULL);
		details = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(reqd_size);
		if (details == NULL) {
			continue;
		}

		memset(details, 0, reqd_size);
		details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		ret = SetupDiGetDeviceInterfaceDetail(info, &iface, details,
			reqd_size, NULL, NULL);
		if (!ret) {
			free(details);
			continue;
		}
		//h = CreateFile(details->DevicePath, GENERIC_READ | GENERIC_WRITE,
		//	FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		//	OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		free(details);
		if (h == INVALID_HANDLE_VALUE) {
			continue;
		}
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		ret = HidD_GetAttributes(h, &attrib);
		//printf("vid: %4x\n", attrib.VendorID);
		if (!ret || (vid > 0 && attrib.VendorID != vid) ||
			(pid > 0 && attrib.ProductID != pid) ||
			!HidD_GetPreparsedData(h, &hid_data)) {
			CloseHandle(h);
			continue;
		}
		if (!HidP_GetCaps(hid_data, &capabilities) ||
			(usage_page > 0 && capabilities.UsagePage != usage_page) ||
			(usage > 0 && capabilities.Usage != usage)) {
			HidD_FreePreparsedData(hid_data);
			CloseHandle(h);
			continue;
		}
		HidD_FreePreparsedData(hid_data);
		hid = (struct hid_struct *)malloc(sizeof(struct hid_struct));
		if (!hid) {
			CloseHandle(h);
			continue;
		}
		hid->handle = h;
		hid->open = 1;
		add_hid(hid);
		count++;
		if (count >= max) {
			return count;
		}
	}
	return count;
}


//  rawhid_close - close a device
//
//    Inputs:
//	num = device to close (zero based)
//    Output
//	(nothing)
//
//void rawhid_close(int num)
//{
//	//hid_t *hid;
//
//	//hid = get_hid(num);
//	//if (!hid || !hid->open) {
//	//	return;
//	//}
//	//hid_close(hid);
//}


int rawhid_isOpen(int num)
{
	//hid_t *hid;
	//hid = get_hid(num);
	//if (!hid || !hid->open) {
	//	return false;
	//}
	//return true;
	return 0;
}



static void add_hid(hid_t *h)
{
	//if (!first_hid || !last_hid) {
	//	first_hid = last_hid = h;
	//	h->next = h->prev = NULL;
	//	return;
	//}
	//last_hid->next = h;
	//h->prev = last_hid;
	//h->next = NULL;
	//last_hid = h;
}


static hid_t * get_hid(int num)
{
	hid_t *p;
	//for (p = first_hid; p && num > 0; p = p->next, num--);
	return p;
}


static void free_all_hid(void)
{
	//hid_t *p, *q;

	//for (p = first_hid; p; p = p->next) {
	//	hid_close(p);
	//}
	//p = first_hid;
	//while (p) {
	//	q = p;
	//	p = p->next;
	//	free(q);
	//}
	//first_hid = last_hid = NULL;
}


//static void rawhid_close(hid_t *hid)
//{
//	//CloseHandle(hid->handle);
//	//hid->handle = NULL;
//}


// List all the HID devices attached to a Mac running Mac OS X
int rawhid_listdevices(int * index, long * vid, long * pid, char ** manufacturer_name, char ** product_name)
{
	HDEVINFO hDevInfo; // reference to hardware device information set
	SP_DEVINFO_DATA deviceInfoData; // device information structure
	//SP_DEVICE_INTERFACE_DATA interfaceData; // device INTERFACE information structure
	//PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = NULL; // device INTERFACE detailed information structure
	DEVPROPTYPE propertyType;
	CONFIGRET status; // function status return
	DWORD dwSize;
	TCHAR deviceInstanceID[MAX_DEVICE_ID_LEN]; // devDesc[1024], hardwareIDs[4096];
	WCHAR buffer[4096];
	char bufferA[4096]; // char buffer for function compatibility
	UINT16 number_of_devices = 0; // number of *VALID* enumerated devices (USB or HID)
	GUID guid;
	//UINT16 device_set; // number assigned to the set device
	bool valid_device = false;
	unsigned long vendorId;
	unsigned long productId;
	wstring manufacturerName, productName;

	// Create a HDEVINFO with all present devices
	hDevInfo = SetupDiGetClassDevs(NULL,
		NULL, // enumerator
		NULL,
		//		DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);
		DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return -1;
	}

	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (unsigned i = 0; ; i++) {
		if (!SetupDiEnumDeviceInfo(hDevInfo, i, &deviceInfoData))
			break;

		// Retrieve device instance ID
		status = CM_Get_Device_ID(deviceInfoData.DevInst, deviceInstanceID, MAX_PATH, 0);
		if (status == CR_SUCCESS) {
			//number_of_devices = i;
			// Debug
			//_tprintf(L"----\r\nDevice instance: %s, current device#: %d \r\n", deviceInstanceID, number_of_devices);
			
			string sourceStr(deviceInstanceID);
			int f1 = sourceStr.find("USB");
			int f2 = sourceStr.find("HID");
			int fVid, fPid;
			// 'USB'-pattern matching...
			if ((f1 < sourceStr.length())) {
				// looking for 'VID_'
				f1 = sourceStr.find("VID_");
				if((f1 < sourceStr.length())) fVid = f1;
				else fVid = -1;
				// looking for 'PID_'
				f2 = sourceStr.find("PID_");
				if ((f2 < sourceStr.length())) fPid = f2;
				else fPid = -1;
			}
			// 'HID'-pattern matching...
			else if ((f2 < sourceStr.length())) {
				// looking for 'VID_'
				f1 = sourceStr.find("VID_");
				if ((f2 < sourceStr.length())) fVid = f1;
				else fVid = -1;
				// looking for 'PID_'
				f2 = sourceStr.find("PID_");
				if ((f2 < sourceStr.length())) fPid = f2;
				else fPid= -1;
			}
			else {
				fVid = -1;
				fPid = -1;
			}

			if((fVid >= 0) && (fPid >= 0)) {
				HidD_GetHidGuid(&guid);
				//ofLog(OF_LOG_NOTICE, "HID GUID: 0x%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\r\n",
				//	guid.Data1, guid.Data2, guid.Data3,
				//	guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				//	guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
				//printf("VID @ %d, PID @ %d\r\n", fVid, fPid);
				string vidStr = sourceStr.substr((fVid+4), 4);
				string pidStr = sourceStr.substr((fPid+4), 4);
				//_tprintf(L"VID = %s, ", vidStr.c_str());
				//_tprintf(L"PID = %s\r\n", pidStr.c_str());
				vendorId = stoul(vidStr, nullptr, 16);
				productId = stoul(pidStr, nullptr, 16);
				//printf("vid = 0x%04x, pid = 0x%04x\r\n", vendorId, productId);
				index[number_of_devices] = number_of_devices;
				vid[number_of_devices] = vendorId;
				pid[number_of_devices] = productId;

				// Retrieve product name
				if (SetupDiGetDevicePropertyW(hDevInfo, &deviceInfoData,
					&DEVPKEY_Device_BusReportedDeviceDesc, &propertyType,
					(BYTE*)buffer, sizeof(buffer), &dwSize, 0)) {
					//_tprintf(L"Device description: %s\r\n", buffer);
					// converting buffer (WCHAR) to char for function compatibility
					unsigned int strLength = wcslen(buffer);
					if (strLength > 254) {
						strLength = 254;
						buffer[255] = '\0';
					}
					//printf("strLengthW: %d\r\n", strLength);
					int ret = wcstombs(bufferA, buffer, sizeof(bufferA));
					if (ret == 4096) bufferA[4095] = '\0';
					//if (ret > 0) printf("bufferA: %s, strLengthA: %d\r\n", bufferA, ret);
					strncpy(&product_name[number_of_devices][0], bufferA, strLength + 1);
					//printf("product name: %s\r\n", product_name[number_of_devices]);
				}
				// Retrieve manufacturer name
				if (SetupDiGetDevicePropertyW(hDevInfo, &deviceInfoData,
					&DEVPKEY_Device_Manufacturer, &propertyType,
					(BYTE*)buffer, sizeof(buffer), &dwSize, 0)) {
					//_tprintf(L"Manufacturer: %s\r\n", buffer);
					// converting buffer (WCHAR) to char for function compatibility
					unsigned int strLength = wcslen(buffer);
					if (strLength > 254) {
						strLength = 254;
						buffer[255] = '\0';
					}
					int ret = wcstombs(bufferA, buffer, 1024);
					if (ret == 1024) bufferA[1023] = '\0';
					strncpy(&manufacturer_name[number_of_devices][0], bufferA, strLength + 1);
					//printf("manufacturer name: %s\r\n", manufacturer_name[number_of_devices]);
				}

				// Print device informations to the console
				printf("Device instance:\r\n  - counter %d\r\n  - vid 0x%04x\r\n  - pid 0x%04x\r\n  - name %s\r\n  - manufacturer %s\r\n",
					index[number_of_devices], pid[number_of_devices], vid[number_of_devices],
					product_name[number_of_devices], manufacturer_name[number_of_devices]);
					
				number_of_devices += 1;
				valid_device = true;
			}
			else {
				//printf("Nothing found...\r\n");
				valid_device = false;
			}
		}
		else {
			printf("----\r\n!!!!Unable to retreive device instance ID!!!!\r\n");
			valid_device = false;
		}
	}
	// Debug
	printf("Number of *VALID* devices found: %d\r\n", number_of_devices);
	if (number_of_devices < 1) {
		return -1;
	}

	return (int)number_of_devices;
}


void print_win32_err(void)
{
	char buf[256];
	//LPWSTR buf;
	DWORD err;

	err = GetLastError();
	//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
	//	0, (WCHAR*)buf, sizeof(buf), NULL);
	printf("err %ld: %s\n", err, buf);
}
