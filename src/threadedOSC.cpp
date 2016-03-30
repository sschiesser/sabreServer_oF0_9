/*
 *  threadedOSC.cpp
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
 *  @date 20121030
 *
 */

#include "threadedOSC.h"

threadedOSC::threadedOSC()
{
    OSCInterval = 4; // default value
}

threadedOSC::~threadedOSC()
{	
	if( isThreadRunning() ) { 
		stopThread();
	}
}

void threadedOSC::start()
{
	startThread(true, false);   // blocking, verbose
//    printf("OSC thread started\n");
}

void threadedOSC::stop()
{
	stopThread();
}

//--------------------------
void threadedOSC::threadedFunction() 
{
	while( isThreadRunning() != 0 ){
		if( lock() ){
            if(serialObject->fullspeedOSC == 0){
                sendOSC();
            }
            ofSleepMillis(OSCInterval);
			unlock();
		}
	}
}

void threadedOSC::sendOSC()
{
    
// timestamps & keys
//	if(serialObject->haveInput[0] || serialObject->haveInput[1]) { // both hands triggers sending

        // Keys
		oscMsg[61].clear();
		oscMsg[61].setAddress( serialObject->timestampAddressRight ); // timestamp Right
        oscMsg[61].addIntArg( serialObject->timestampRight );
		oscSender.sendMessage( oscMsg[61] );
        
        oscMsg[61].clear();
		oscMsg[61].setAddress( serialObject->timestampAddressLeft ); // timestamp Left
        oscMsg[61].addIntArg( serialObject->timestampLeft );
		oscSender.sendMessage( oscMsg[61] );
    
        oscMsg[62].clear();
		oscMsg[62].setAddress( serialObject->timestampAddressServer ); // timestamp
        oscMsg[62].addIntArg( serialObject->systemTimestamp );
		oscSender.sendMessage( oscMsg[62] );
        
		for(int i = 0; i < 25; i++) { // continuous key values
			if(serialObject->keys[i].changed) {
				oscMsg[i+16].clear();
				oscMsg[i+16].setAddress( serialObject->keys[i].oscaddress+"/continuous");
				oscMsg[i+16].addFloatArg( serialObject->keys[i].continuous );
				oscSender.sendMessage( oscMsg[i+16] );
			}
		}
		for(int i = 0; i < 25; i++) { // binary key values
			if(serialObject->keys[i].binaryChanged) {
				oscMsg[i+16].clear();
				oscMsg[i+16].setAddress( serialObject->keys[i].oscaddress+"/down");
				oscMsg[i+16].addIntArg( serialObject->keys[i].binary );
				oscSender.sendMessage( oscMsg[i+16] );
                serialObject->keys[i].binaryChanged = false;

			}
		}
		for(int i = 0; i < 25; i++) { // raw key values
			if(serialObject->keys[i].changed) {
				oscMsg[i+16].clear();
				oscMsg[i+16].setAddress( serialObject->keys[i].oscaddress+"/raw");
				oscMsg[i+16].addIntArg( serialObject->keys[i].raw );
				oscSender.sendMessage( oscMsg[i+16] );
                serialObject->keys[i].changed = false;
			}
		}	
		if(serialObject->keycodeChanged) { // keycode
			oscMsg[42].clear();
			oscMsg[42].setAddress( serialObject->keycodeaddress );
			oscMsg[42].addIntArg( serialObject->keycode );
			oscSender.sendMessage( oscMsg[42] );
			serialObject->keycodeChanged = false;
			// printf("sending keycode %d\n", keycode);
		}
		if(serialObject->validMidiNote) {	
			oscMsg[43].clear();	// midinote derived from keycode
			oscMsg[43].setAddress( serialObject->midinoteaddress );
			oscMsg[43].addIntArg( serialObject->midinote );
			oscSender.sendMessage( oscMsg[43] );
			serialObject->validMidiNote = false;
		}
		for(int i = 0; i < 3; i++) { // buttons
			if(serialObject->buttonChanged[i]) {
				oscMsg[i+44].clear();
				oscMsg[i+44].setAddress( serialObject->buttonaddresses[2-i] );
				oscMsg[i+44].addIntArg( serialObject->button[i] );
				oscSender.sendMessage( oscMsg[i+44] );
                serialObject->buttonChanged[i] = false;
			}
		}

        // IMU
		oscMsg[0].clear();
		oscMsg[0].setAddress( serialObject->imuaddresses[0] ); // IMU accelero scaled
		oscMsg[0].addFloatArg( serialObject->IMU[0] );
		oscMsg[0].addFloatArg( serialObject->IMU[1] );
		oscMsg[0].addFloatArg( serialObject->IMU[2] );
		oscSender.sendMessage( oscMsg[0] );	
		
		oscMsg[1].clear();
		oscMsg[1].setAddress( serialObject->imuaddresses[1] ); // IMU gyro scaled
		oscMsg[1].addFloatArg( serialObject->IMU[3] );
		oscMsg[1].addFloatArg( serialObject->IMU[4] );
		oscMsg[1].addFloatArg( serialObject->IMU[5] );
		oscSender.sendMessage( oscMsg[1] );	
		
		oscMsg[2].clear();
		oscMsg[2].setAddress( serialObject->imuaddresses[2] ); // IMU magneto scaled
		oscMsg[2].addFloatArg( serialObject->IMU[6] );
		oscMsg[2].addFloatArg( serialObject->IMU[7] );
		oscMsg[2].addFloatArg( serialObject->IMU[8] );
		oscSender.sendMessage( oscMsg[2] );	
		
		oscMsg[3].clear();
		oscMsg[3].setAddress( serialObject->imuaddresses[3] ); // IMU accelero raw
		oscMsg[3].addFloatArg( serialObject->rawIMU[0] );
		oscMsg[3].addFloatArg( serialObject->rawIMU[1] );
		oscMsg[3].addFloatArg( serialObject->rawIMU[2] );
		oscSender.sendMessage( oscMsg[3] );	
		
		oscMsg[4].clear();
		oscMsg[4].setAddress( serialObject->imuaddresses[4] ); // IMU gyro raw
		oscMsg[4].addFloatArg( serialObject->rawIMU[3] );
		oscMsg[4].addFloatArg( serialObject->rawIMU[4] );
		oscMsg[4].addFloatArg( serialObject->rawIMU[5] );
		oscSender.sendMessage( oscMsg[4] );	
		
		oscMsg[5].clear();
		oscMsg[5].setAddress( serialObject->imuaddresses[5] ); // IMU magneto raw
		oscMsg[5].addFloatArg( serialObject->rawIMU[6] );
		oscMsg[5].addFloatArg( serialObject->rawIMU[7] );
		oscMsg[5].addFloatArg( serialObject->rawIMU[8] );
		oscSender.sendMessage( oscMsg[5] );	
		
		oscMsg[6].clear();
		oscMsg[6].setAddress( serialObject->imuaddresses[6] ); // IMU accelero summed
		oscMsg[6].addFloatArg( serialObject->summedIMU[0] );
		oscSender.sendMessage( oscMsg[6] );	
		
		oscMsg[7].clear();
		oscMsg[7].setAddress( serialObject->imuaddresses[7] ); // IMU gyro summed
		oscMsg[7].addFloatArg( serialObject->summedIMU[1] );
		oscSender.sendMessage( oscMsg[7] );	
		
		oscMsg[8].clear();
		oscMsg[8].setAddress( serialObject->imuaddresses[8] ); // IMU magneto summed
		oscMsg[8].addFloatArg( serialObject->summedIMU[2] );
		oscSender.sendMessage( oscMsg[8] );
		
		oscMsg[9].clear();
		oscMsg[9].setAddress( serialObject->imuaddresses[10] ); // IMU heading from accelerometer
		oscMsg[9].addFloatArg( serialObject->heading );
		oscSender.sendMessage( oscMsg[9] );	
		
		oscMsg[10].clear();
		oscMsg[10].setAddress( serialObject->imuaddresses[11] ); // IMU tilt from accelerometer
		oscMsg[10].addFloatArg( serialObject->tilt );
		oscSender.sendMessage( oscMsg[10] );
		
		oscMsg[11].clear();
		oscMsg[11].setAddress( serialObject->imuaddresses[9] ); // IMU temperature in degreee celsius 
		oscMsg[11].addFloatArg( serialObject->IMU[9] );
		oscSender.sendMessage( oscMsg[11] );
        
        oscMsg[12].clear();
		oscMsg[12].setAddress( serialObject->batteryAddressMain ); // battery level main
		oscMsg[12].addIntArg( serialObject->batteryLevelRight );
		oscSender.sendMessage( oscMsg[12] );
        
        oscMsg[13].clear();
		oscMsg[13].setAddress( serialObject->linkQualityAddressLeft ); // left link quality
		oscMsg[13].addIntArg( serialObject->linkQualityLeft );
		oscSender.sendMessage( oscMsg[13] );
        
        oscMsg[14].clear();
		oscMsg[14].setAddress( serialObject->linkQualityAddressRight ); // right link quality
		oscMsg[14].addIntArg( serialObject->linkQualityRight );
		oscSender.sendMessage( oscMsg[14] );
        
        // reset flags
    if(serialObject->haveInput[0]) {
        serialObject->haveInput[0] = false;
    }
    if(serialObject->haveInput[1]) {
        serialObject->haveInput[1] = false;
    }
    

//    }
//	if(serialObject->haveInput[2]) { // AirMems packet
    
        oscMsg[63].clear();
		oscMsg[63].setAddress( serialObject->timestampAddressAir ); // timestamp
        oscMsg[63].addIntArg( serialObject->timestampAir );
		oscSender.sendMessage( oscMsg[63] );
		
		oscMsg[48].clear();
		oscMsg[48].setAddress( serialObject->airaddresses[0] ); // air pressure
		oscMsg[48].addFloatArg( serialObject->air[0]);
		oscSender.sendMessage( oscMsg[48] );
		
		oscMsg[49].clear();
		oscMsg[49].setAddress( serialObject->airaddresses[1] ); // air temperature
		oscMsg[49].addFloatArg( serialObject->air[1]);
		oscSender.sendMessage( oscMsg[49] );
        
		oscMsg[50].clear();
		oscMsg[50].setAddress( serialObject->batteryAddressAir ); // air battery
		oscMsg[50].addIntArg( serialObject->batteryLevelAir);
		oscSender.sendMessage( oscMsg[50] );
        
        oscMsg[51].clear();
		oscMsg[51].setAddress( serialObject->linkQualityAddressAir ); // air link quality
		oscMsg[51].addIntArg( serialObject->linkQualityAir);
		oscSender.sendMessage( oscMsg[51] );
        
        // reset flag
		serialObject->haveInput[2] = false;
//	}
}
