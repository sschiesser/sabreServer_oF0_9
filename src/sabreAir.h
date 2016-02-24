/*
 *  sabreAir.h
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
 *  @author Jan Schacher
 *  @date 20130620
 *
 */

#pragma once

/* Sample numbers at which airMEMS starts, resp.
 * stops the ambient pressure measurement for
 * point 0 calibration
 */
#define AIR_CALIBSTART 5
#define AIR_CALIBSTOP 505

class sabreAir
{
	
public:
	sabreAir(void) {
		scaled = 0;
		minimum = 32768.0;
		maximum = -32768.0;
		range = 1.0;
        offset = 0.0;
        relative = 0.0;
        calibrationFlag = 0;
        calibrationCounter = 0;
        calibrationValue = 0.0;
        calibratePressureRange = 0;
	}

	virtual ~sabreAir(){};

	double	scaled;		// container for scaled key-values
	double	minimum;
	double	maximum;
	double	range;			// 1.0 / (max - min)
	double  offset;
    double  relative;
    
    bool        calibrationFlag;
    int         calibrationCounter;
    double      calibrationValue;
    bool        calibratePressureRange;
};

