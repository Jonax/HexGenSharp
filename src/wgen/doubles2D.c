/*
 
 src/wgen/doubles2D.h - operations on a 2D array of doubles
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2010, 2013, 2014 Ben Golightly <golightly.ben@googlemail.com>

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
#include "wgen/wgen.h"
#include <string.h> // memset
#include <float.h> // DBL_MAX


int Doubles2DInit(Doubles2D *d, size2D s)
{
    if (!d) { X2(bad_arg, "NULL buffer pointer"); }
    
    d->size = s;
    d->elements = (s.x * s.y);
    d->bytes = (s.x * s.y) * sizeof(double);
    
    d->values = malloc(d->bytes);
    if (!d->values) { X(alloc_values); }
    
    return 1;
    
    err_alloc_values:
    err_bad_arg:
        return 0;
}


void Doubles2DTeardown(Doubles2D *d)
{
    if (!d)         { X2(bad_arg, "NULL buffer pointer"); }
    if (!d->values) { X2(bad_arg, "buffer uninitialised"); }
    
    free(d->values);
    return;
    
    err_bad_arg:
        return;
}


void Doubles2DZeroFill(Doubles2D *d) // set all values to 0.
{
    if (!d)         { X2(bad_arg, "NULL buffer pointer"); }
    if (!d->values) { X2(bad_arg, "buffer uninitialised"); }
    
    // does memset(&x, 0, sizeof(double)) give (x == 0)?
    
#   ifdef __STDC_IEC_559__ // yes
    
    memset(d->values, 0, d->bytes);
    
#   else // otherwise set each element manually.
    
    for (size_t i = 0; i < d->elements; i++)
    {
        d->values[i] = 0.0;
    }
    
#   endif
    
    return;
    
    err_bad_arg:
        return;
}
#include <stdio.h>

void Doubles2DNormalise(Doubles2D *d) // normalise all values between 0.0 & 1.0
{
    if (!d)         { X2(bad_arg, "NULL buffer pointer"); }
    if (!d->values) { X2(bad_arg, "buffer uninitialised"); }
    
    double min = (double) (DBL_MAX);
    double max = (double) (-DBL_MAX);
    
    for (size_t i = 0; i < d->elements; i++)
    {
        if (d->values[i] < min) { min = d->values[i]; }
        if (d->values[i] > max) { max = d->values[i]; }
    }
    
    double diff = max - min;
    
    for (size_t i = 0; i < d->elements; i++)
    {
        d->values[i] -= min;
        d->values[i] /= diff;
    }
    
    d->minimum = min;
    d->maximum = max;
    
    return;
    
    err_bad_arg:
        return;
}


void Doubles2DNormaliseMaximum(Doubles2D *d)
{
    // normalise all values between 0.0 & 1.0 but maintaining the full range
    if (!d)         { X2(bad_arg, "NULL buffer pointer"); }
    if (!d->values) { X2(bad_arg, "buffer uninitialised"); }
    
    double min = (double) (DBL_MAX);
    double max = (double) (-DBL_MAX);
    
    for (size_t i = 0; i < d->elements; i++)
    {
        if (d->values[i] < 0.0) { d->values[i] = 0.0; }
        if (d->values[i] < min) { min = d->values[i]; }
        if (d->values[i] > max) { max = d->values[i]; }
    }
    
    for (size_t i = 0; i < d->elements; i++)
    {
        d->values[i] /= max;
    }
    
    d->minimum = min;
    d->maximum = max;
    
    return;
    
    err_bad_arg:
        return;
}


void Doubles2DClampFloorTo(Doubles2D *d, double min, double to)
{
    for (size_t i = 0; i < d->elements; i++)
    {
        if (d->values[i] < min) { d->values[i] = to; }
    }
}


void Doubles2DClampCeilingTo(Doubles2D *d, double max, double to)
{
    for (size_t i = 0; i < d->elements; i++)
    {
        if (d->values[i] > max) { d->values[i] = to; }
    }
}


void Doubles2DSquare(Doubles2D *d)
{
    for (size_t i = 0; i < d->elements; i++)
    {
        d->values[i] *= d->values[i];
    }
}
