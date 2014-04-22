/*

 src/clock.h - Implements a monotonic clock
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2013-2014 Ben Golightly <golightly.ben@googlemail.com>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 ------------------------------------------------------------------------------
 
 Many clock implementations have the problem that they can even go backwards for
 various reasons.
 
 This ClockMonotonic implementation should always go forwards.
 
 Note: This has not been tested AT ALL.
 
*/



#ifndef HG14_CLOCK_H
#   define HG14_CLOCK_H

#include <sys/time.h>
#include <stdint.h>

typedef int64_t Millisecs;

struct Clock
{
    Millisecs now;
    Millisecs max_reported;
    
    struct timeval start;
    struct timeval state;
};

typedef struct Clock Clock;

int  ClockInit(Clock *clock);

int  ClockReset(Clock *clock);
int  ClockSync(Clock *clock);

Millisecs ClockTime(Clock *clock);
Millisecs ClockMonotonicTime(Clock *clock);

Millisecs ClockDiff(Clock *earliest, Clock *latest);
Millisecs ClockMonotonicDiff(Clock *earliest, Clock *latest);

double ClockFramesPerSecond(Millisecs frametime);

#endif

