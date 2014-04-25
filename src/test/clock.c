// BSAG 2014. public domain.

#include "clock.h"
#include "test/_test.h"
#include <stdio.h>

int test_clock_error(void)
{
    START;
    
    TEST(!ClockInit(NULL));
    TEST(!ClockReset(NULL));
    TEST(!ClockSync(NULL));
    TEST(!ClockTime(NULL));
    TEST(!ClockMonotonicTime(NULL));
    TEST(!ClockDiff(NULL, NULL));
    TEST(!ClockMonotonicDiff(NULL, NULL));
    TEST(ClockFramesPerSecond( 0) > 0.0);
    TEST(ClockFramesPerSecond(-1) > 0.0);
    TEST(ClockFramesPerSecond(+1) > 0.0);
    
    END;
}


static int SetTime100(struct timeval *tv)
{
    tv->tv_sec = 100;
    tv->tv_usec = 0;
    
    return 0;
}


static int SetTime200(struct timeval *tv)
{
    tv->tv_sec = 200;
    tv->tv_usec = 0;
    
    return 0;
}


int test_clock_time_warp(void)
{
    START;
    
    /* This test verifies that the clock implementation only ever ticks
     * forwards, even when the time changes. This means that it never
     * reports a negative time value. */
    
    Clock t;
    
#   define MILLISECS 1000
    
    TEST_FATAL(ClockInit(&t));
    
    // Set time to 100 seconds (fixed) and reset clock
    t.get_time_of_day = SetTime100;
    TEST_FATAL(ClockReset(&t));
    
    // After sync, relative time is 0 elapsed from start
    TEST_FATAL(ClockSync(&t));
    TEST(0 == ClockTime(&t));
    TEST(0 == ClockMonotonicTime(&t));
    
    // Set time to 200 seconds (fixed)
    t.get_time_of_day = SetTime200;
    
    // After sync, relative time is 100 seconds elapsed from start
    TEST_FATAL(ClockSync(&t));
    TEST(100 * MILLISECS == ClockTime(&t));
    TEST(100 * MILLISECS == ClockMonotonicTime(&t));
    
    // Set time to 100 seconds (fixed)
    t.get_time_of_day = SetTime100;
    
    // After sync, relative time is 100 seconds earlier, i.e. the same as start
    TEST_FATAL(ClockSync(&t));
    
    // relative time is the same as start
    TEST(0 == ClockTime(&t));
    
    // but monotonic time can only tick forwards...
    TEST(100 * MILLISECS == ClockMonotonicTime(&t));
    
    END;
}
