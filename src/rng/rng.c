/*

 rng/rng.c - Common Random Number Generator API
 
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
 
 IMPORTANT NOTICE: we use a seed of only four bytes when we could potentially
 use a seed of 256 bytes. This may weaken the cryptographic properties of these
 generators. Seeding an RNG securely is a non-trivial problem.
 
*/

#include "base.h" // exceptions

#include "rng/rng.h"
#include "rng/isaac.h"

#include <sys/time.h>
#include <errno.h>
#include <stdlib.h> // malloc


struct RNG
{
    unsigned int type;
    unsigned int seed;
    
    union
    {
        Isaac *isaac;
    } impl;
    
    void (*fill_uint8_t) (void *, uint8_t *,  size_t);
    void (*fill_uint32_t)(void *, uint32_t *, size_t);
    void (*fill_float)   (void *, float *,    size_t);
    void (*fill_double)  (void *, double *,   size_t);
    
    uint8_t  (*next_uint8_t)(void *);
    uint32_t (*next_uint32_t)(void *);
    float    (*next_float)(void *);
    double   (*next_double)(void *);
    
    void *impl_void_ptr;
};


unsigned int RNGSeed(RNG *r) { return r->seed; }


RNG *RNGNew(unsigned int type, unsigned int seed)
{
    struct timeval t;
    RNG *r;
    
    // Validate type
    switch (type)
    {
        case RNG_ISAAC:
            break;
        default:
            X(invalid_rng_type);
    }
    
    // Use time as the seed when seed is 0.
    while (seed == 0)
    {
        if (0 != gettimeofday(&t, 0))
            { X3(gettimeofday, "error making seed from time", errno); }
        
        seed = (unsigned int) t.tv_usec;
    }
    
    r = malloc(sizeof(RNG));
    if (!r) { X(alloc_RNG); }
    
    r->type = type;
    r->seed = seed;
    
    switch (type)
    {
        case RNG_ISAAC:
            r->impl.isaac = IsaacNew(seed);
            if (!r->impl.isaac) { X2(alloc_impl_RNG, "isaac"); }
            
            r->fill_uint8_t  = &IsaacFill_uint8_t;
            r->fill_uint32_t = &IsaacFill_uint32_t;
            r->fill_float    = &IsaacFill_float;
            r->fill_double   = &IsaacFill_double;
            
            r->next_uint8_t = &IsaacNext_uint8_t;
            r->next_uint32_t = &IsaacNext_uint32_t;
            r->next_float    = &IsaacNext_float;
            r->next_double   = &IsaacNext_double;
            
            r->impl_void_ptr = r->impl.isaac;
            break;
            
        default:
            X(unhandled_rng_type);
    }
    
    return r;
    
    err_alloc_impl_RNG:
    err_unhandled_rng_type:
        free(r);
    err_alloc_RNG:
    err_gettimeofday:
    err_invalid_rng_type:
        return NULL;
}


void RNGRandomise(RNG *r, unsigned int seed)
{
    switch (r->type)
    {
        case RNG_ISAAC:
            IsaacRandomise(r->impl.isaac, seed);
            break;
            
        default:
            X(unkown_rng_type);
    }
    
    r->seed = seed;
    
    return;
    
    err_unkown_rng_type:
        exit(EXIT_FAILURE);
}


void RNGFree(RNG *r)
{
    switch (r->type)
    {
        case RNG_ISAAC:
            IsaacFree(r->impl.isaac);
            break;
            
        default:
            X(unkown_rng_type);
    }
    
    free(r);
    
    return;
    
    err_unkown_rng_type:
        exit(EXIT_FAILURE);
}


void RNGFill_uint8_t(RNG *r, uint8_t *buf, size_t nmemb)
{
    r->fill_uint8_t(r->impl_void_ptr, buf, nmemb);
}


void RNGFill_uint32_t(RNG *r, uint32_t *buf, size_t nmemb)
{
    r->fill_uint32_t(r->impl_void_ptr, buf, nmemb);
}


void RNGFill_float(RNG *r, float *buf, size_t nmemb)
{
    r->fill_float(r->impl_void_ptr, buf, nmemb);
}


void RNGFill_double(RNG *r, double *buf, size_t nmemb)
{
    r->fill_double(r->impl_void_ptr, buf, nmemb);
}


uint8_t RNGNext_uint8_t(RNG *r)
{
    return r->next_uint8_t(r->impl_void_ptr);
}


uint32_t RNGNext_uint32_t(RNG *r)
{
    return r->next_uint32_t(r->impl_void_ptr);
}


float RNGNext_float(RNG *r)
{
    return r->next_float(r->impl_void_ptr);
}


double RNGNext_double(RNG *r)
{
    return r->next_double(r->impl_void_ptr);
}

