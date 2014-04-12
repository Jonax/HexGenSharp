/*

 src/clock.c - Implements a monotonic clock
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2013 - 2014 Ben Golightly <golightly.ben@googlemail.com>

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
 
*/

#include "base.h"
#include "clock.h"

#include <stdlib.h>

#define clock not_really_clock // avoid -Wshadow
#define time  not_really_time // avoid -Wshadow


int ClockInit(Clock *clock)
{
    if (!ClockReset(clock)) { X(reset_clock); }
    
    return 1;
    
    err_reset_clock:
        return 0;
}


int ClockReset(Clock *clock)
{
    clock->now = 0;
    clock->max_reported = 0;
    
    if (0 != gettimeofday(&clock->start, NULL))
    {
        X(get_time);
    }
    
    clock->state.tv_sec  = clock->start.tv_sec;
    clock->state.tv_usec = clock->start.tv_usec;
    
    return 1;
    
    err_get_time:
        return 0;
}


int ClockSync(Clock *clock)
{
    if (0 != gettimeofday(&clock->state, NULL))
    {
        X(get_time);
    }
    
    int64_t seconds      = (int64_t) (clock->state.tv_sec)  - (int64_t) (clock->start.tv_sec);
    int64_t microseconds = (int64_t) (clock->state.tv_usec) - (int64_t) (clock->start.tv_usec);
    
    clock->now = (int64_t) ((seconds * 1000L) + (microseconds / 1000L));
    
    return 1;
    
    err_get_time:
        return 0;
}


Millisecs ClockTime(Clock *clock)
{
    return clock->now;
}


Millisecs ClockMonotonicTime(Clock *clock)
{
    if (clock->now > clock->max_reported)
    {
        clock->max_reported = clock->now;
        return clock->now;
    }
    
    return clock->max_reported;
}


Millisecs ClockDiff(Clock *earliest, Clock *latest)
{
    return (ClockTime(latest) - ClockTime(earliest));
}


Millisecs ClockMonotonicDiff(Clock *earliest, Clock *latest)
{
    return (ClockMonotonicTime(latest) - ClockMonotonicTime(earliest));
}


double ClockFramesPerSecond(Millisecs frametime)
{
    double fps = 9999.0;
    
    if (frametime != 0)
    {
        fps = 1000.0 / (double) frametime;
    }
    
    return fps;
}

