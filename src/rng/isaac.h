/*

 ISAAC: a fast cryptographic random number generator
 
 ------------------------------------------------------------------------------
 
 Original code:
 
 (c) Bob Jenkins, March 1996, Public Domain
 You may use this code in any way you wish, and it is free.  No warrantee.
 
 http://www.burtleburtle.net/bob/rand/isaacafa.html
 
 ------------------------------------------------------------------------------
 
 Modifications:
 
 Ben Golightly, 2013 <golightly.ben@googlemail.com>
 
 The author has released their modifications into the Public Domain.
 The author disclaims all warranty.
 
 ------------------------------------------------------------------------------
 
 ISAAC (Indirection, Shift, Accumulate, Add, and Count) generates 32-bit random
 numbers. Averaged out, it requires 18.75 machine cycles to generate each
 32-bit value. Cycles are guaranteed to be at least 2^40 values long, and
 they are 2^8295 values long on average. The results are uniformly distributed,
 unbiased, and unpredictable unless you know the seed.
 
 ------------------------------------------------------------------------------
 
 IMPORTANT NOTICE: we use a seed of only four bytes when we could potentially
 use a seed of 256 bytes. This may weaken the cryptographic properties of these
 generators. Seeding an RNG securely is a non-trivial problem.
 
*/

#ifndef HG14_ISAAC_H
#   define HG14_ISAAC_H

#include <stdint.h>
#include <stddef.h> // size_t

typedef struct Isaac Isaac;

/* Create or Init an RNG state */
Isaac *IsaacNew(unsigned int seed);
void IsaacFree(Isaac *i);

void IsaacRandomise(Isaac *i, unsigned int seed);

# define IsaacFill(type, i, buf, size) IsaacFill_##type(i, buf, size)
# define IsaacNext(type, i) IsaacNext_##type(i)

void IsaacFill_uint8_t (void *p, uint8_t *buf,  size_t nmemb);
void IsaacFill_uint32_t(void *p, uint32_t *buf, size_t nmemb);
void IsaacFill_float   (void *p, float    *buf, size_t nmemb);
void IsaacFill_double  (void *p, double   *buf, size_t nmemb);

uint8_t  IsaacNext_uint8_t (void *p);
uint32_t IsaacNext_uint32_t(void *p);
float    IsaacNext_float   (void *p);
double   IsaacNext_double  (void *p);

#endif
