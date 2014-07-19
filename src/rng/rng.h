/*

 rng/rng.h - Common Random Number Generator API
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2013-  2014 Ben Golightly <golightly.ben@googlemail.com>

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
 
 IMPORTANT NOTICE: creating a NewRNG or calling RNGRandomise may clobber the
 seed and behaviour of the stdlib rand() function. Recommendation: avoid stdlib
 rand altogether.
 
 These RNG implementations are used for several reasons:
 
    (1) stdlib rand() output for the same seed will vary across implementations
    (2) stdlib rand() may be of very low quality on some systems
    (3) stdlib rand() doesn't store any state, so is nondeterministic in a
        non-trivial program where multiple places want to call rand().
    (4) ... and rand_r() isn't portable either
 
 IMPORTANT NOTICE: obviously not thread safe. Allocate individual RNGs for each
 task or thread, or synchronise with mutexes appropriately.
 
*/

#ifndef HG14_RNG_H
#   define HG14_RNG_H

# include <stdint.h>
# include <stddef.h> // size_t

# define RNG_ISAAC   1 // Cryptographic, fast, min period 2^40, avg period 2^8295
# define RNG_ISAAC64 2 // Reserved.

typedef struct RNG RNG;

RNG *RNGNew(unsigned int type, unsigned int seed); // use seed==0 to use time
void RNGFree(RNG *r);

unsigned int RNGSeed(RNG *r);
void RNGRandomise(RNG *r, unsigned int seed);

# define RNGFill(type, rng, buf, nmemb) RNGFill_##type(rng, buf, nmemb)
# define RNGNext(type, rng) RNGNext_##type(rng)

uint8_t  RNGNext_uint8_t (RNG *r);
uint32_t RNGNext_uint32_t(RNG *r);
float    RNGNext_float   (RNG *r);
double   RNGNext_double  (RNG *r);

void RNGFill_uint8_t (RNG *r, uint8_t  *buf, size_t nmemb);
void RNGFill_uint32_t(RNG *r, uint32_t *buf, size_t nmemb);
void RNGFill_float   (RNG *r, float    *buf, size_t nmemb);
void RNGFill_double  (RNG *r, double   *buf, size_t nmemb);

#endif
