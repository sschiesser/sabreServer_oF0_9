/*
 *  threadedOSC.h
 * 
 *  SABRe-server
 *
 *  Copyright © 2012/2013 Zurich University of the Arts. All Rights Reserved.
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
 *  @date 20130617
 *
 */

#ifndef _THREADED_OSC_OBJECT
#define _THREADED_OSC_OBJECT

#include "ofMain.h"
#include "ofxOsc.h"

#include "threadedSerial.h"


#define MAXNUM 64

class threadedOSC : public ofThread
{
	
public:
	
	threadedOSC();
	~threadedOSC();
	
	void start();
	void stop();
	void threadedFunction(); 

	void sendOSC();
    
    threadedSerial	* serialObject;         // pointer to the serial thread object
    
	ofxXmlSettings	XML;
	string str1;
	
	ofTrueTypeFont TTF;
	ofxOscSender oscSender;
    ofxOscMessage oscMsg[MAXNUM];

    
    string		sendIP;
	int			sendport;
	
	char		bytesRead[3];				// data from serial, we will be trying to read 3
	char		bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
	int			nBytesRead;					// how much did we read?
	int			nTimesRead;					// how many times did we read?
	float		readTime;					// when did we last read?		
	
    unsigned char input[4][MAXNUM];         // for the four different types of packets
	bool		haveInput[4];				// flags to signal a full packet was parsed successfully
	bool        status;
	
	int			numKeyAddr;
	int			numImuAddr;
	int			numButtonAddr;
	int			numAirAddr;

    
	bool		senderStatus;
    int         OSCInterval;
};

#endif
