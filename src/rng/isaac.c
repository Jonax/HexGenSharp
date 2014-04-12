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
 
 IMPORTANT NOTICE: we use a seed of only four bytes when we could potentially
 use a seed of 256 bytes. This may weaken the cryptographic properties of these
 generators. Seeding an RNG securely is a non-trivial problem.
 
*/

#include "base.h" // exceptions
#include "rng/isaac.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h> // rand

#define RANDSIZL   (8) // default 8; recommend RANDSIZL=4 for noncryptography.
#define RANDSIZ    (1<<RANDSIZL)

typedef uint8_t  ub1;
typedef uint16_t ub2;
typedef uint32_t ub4;

typedef struct randctx randctx;

struct randctx
{
  ub4 randcnt;
  ub4 randrsl[RANDSIZ];
  ub4 randmem[RANDSIZ];
  ub4 randa;
  ub4 randb;
  ub4 randc;
};

struct Isaac
{
    randctx ctx;
};




static void isaac_init(randctx *ctx);
static void isaac_fill(randctx *ctx);



Isaac *IsaacNew(unsigned int seed)
{
    Isaac *i = malloc(sizeof(Isaac));
    if (!i) { return NULL; }
    
    IsaacRandomise(i, seed);
    
    return i;
}


void IsaacFree(Isaac *i)
{
    free(i);
}


void IsaacRandomise(Isaac *i, unsigned int seed)
{
    i->ctx.randa = i->ctx.randb = i->ctx.randc = i->ctx.randcnt = (ub4)0;
    
    srand(seed);
    
    for (size_t j = 0; j < RANDSIZ; j++)
    {
        int r = rand();
        i->ctx.randrsl[j]=(ub4)r;
    }
    
    isaac_init(&i->ctx);
}


void IsaacFill_uint8_t(void *p, uint8_t *buf, size_t nmemb)
{
    Isaac *i = (Isaac *)p;
    randctx *ctx = &i->ctx;
    
    while (nmemb >= 4)
    {
        while ((nmemb >= 4) && ctx->randcnt)
        {
            nmemb -= 4;
            ctx->randcnt--;
            
            uint32_t u = ctx->randrsl[ctx->randcnt];
            buf[0] = (uint8_t) (u >> 24);
            buf[1] = (uint8_t) (u >> 16);
            buf[2] = (uint8_t) (u >> 8);
            buf[3] = (uint8_t) (u);
            
            buf+=4;
        }
        
        if (!ctx->randcnt)
        {
            isaac_fill(ctx);
            ctx->randcnt=RANDSIZ;
        }
    }
    
    /* cleanup remaining */
    while (nmemb)
    {
        while (nmemb && ctx->randcnt)
        {
            nmemb --;
            ctx->randcnt--;
            
            *buf = (uint8_t) ctx->randrsl[ctx->randcnt];
            buf++;
        }
    
        if (!ctx->randcnt)
        {
            isaac_fill(ctx);
            ctx->randcnt=RANDSIZ;
        }
    }
}

void IsaacFill_uint32_t(void *p, uint32_t *buf, size_t nmemb)
{
    Isaac *i = (Isaac *)p;
    randctx *ctx = &i->ctx;
    
    while (nmemb)
    {
        while (nmemb && ctx->randcnt)
        {
            nmemb--;
            ctx->randcnt--;
            
            *buf = ctx->randrsl[ctx->randcnt];
            buf++;
        }
        
        if (!ctx->randcnt)
        {
            isaac_fill(ctx);
            ctx->randcnt=RANDSIZ;
        }
    }
}


void IsaacFill_float(void *p, float *buf, size_t nmemb)
{
    Isaac *i = (Isaac *)p;
    UNUSED(i); UNUSED(buf); UNUSED(nmemb);
    X(not_implemented);
    err_not_implemented:
    exit(1);
}


void IsaacFill_double(void *p, double *buf, size_t nmemb)
{
    Isaac *i = (Isaac *)p;
    UNUSED(i); UNUSED(buf); UNUSED(nmemb);
    X(not_implemented);
    err_not_implemented:
    exit(1);
}


uint8_t IsaacNext_uint8_t(void *p)
{
    return (uint8_t) IsaacNext_uint32_t(p);
}


uint32_t IsaacNext_uint32_t(void *p)
{
    Isaac *i = (Isaac *)p;
    randctx *ctx = &i->ctx;
    
    if (!ctx->randcnt)
    {
        isaac_fill(ctx);
        ctx->randcnt=RANDSIZ - 1;
    }
    
    return ctx->randrsl[ctx->randcnt--];
}


float IsaacNext_float(void *p)
{
    UNUSED(p);
    X(not_implemented);
    err_not_implemented:
    exit(1);
    return 0.0f;
}


double IsaacNext_double(void *p)
{
    UNUSED(p);
    X(not_implemented);
    err_not_implemented:
    exit(1);
    return 0.0;
}

#define ind(mm,x)  (*(ub4 *)((ub1 *)(mm) + ((x) & ((RANDSIZ-1)<<2))))
#define rngstep(mix,a,b,mm,m,m2,r,x) \
{ \
  x = *m;  \
  a = (a^(mix)) + *(m2++); \
  *(m++) = y = ind(mm,x) + a + b; \
  *(r++) = b = ind(mm,y>>RANDSIZL) + x; \
}

static void isaac_fill(randctx *ctx)
{
   register ub4 a,b,x,y,*m,*mm,*m2,*r,*mend;
   mm=ctx->randmem; r=ctx->randrsl;
   a = ctx->randa; b = ctx->randb + (++ctx->randc);
   for (m = mm, mend = m2 = m+(RANDSIZ/2); m<mend; )
   {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
   }
   for (m2 = mm; m2<mend; )
   {
      rngstep( a<<13, a, b, mm, m, m2, r, x);
      rngstep( a>>6 , a, b, mm, m, m2, r, x);
      rngstep( a<<2 , a, b, mm, m, m2, r, x);
      rngstep( a>>16, a, b, mm, m, m2, r, x);
   }
   ctx->randb = b; ctx->randa = a;
}


#define mix(a,b,c,d,e,f,g,h) \
{ \
   a^=b<<11; d+=a; b+=c; \
   b^=c>>2;  e+=b; c+=d; \
   c^=d<<8;  f+=c; d+=e; \
   d^=e>>16; g+=d; e+=f; \
   e^=f<<10; h+=e; f+=g; \
   f^=g>>4;  a+=f; g+=h; \
   g^=h<<8;  b+=g; h+=a; \
   h^=a>>9;  c+=h; a+=b; \
}

static void isaac_init(randctx *ctx)
{
   unsigned int i;
   ub4 a,b,c,d,e,f,g,h;
   ub4 *m,*r;
   ctx->randa = ctx->randb = ctx->randc = 0;
   m=ctx->randmem;
   r=ctx->randrsl;
   a=b=c=d=e=f=g=h=0x9e3779b9;  /* the golden ratio */

   for (i=0; i<4; ++i)          /* scramble it */
   {
     mix(a,b,c,d,e,f,g,h);
   }

   //if (flag) 
   //{
     /* initialize using the contents of r[] as the seed */
     for (i=0; i<RANDSIZ; i+=8)
     {
       a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
       e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];
       mix(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
     /* do a second pass to make all of the seed affect all of m */
     for (i=0; i<RANDSIZ; i+=8)
     {
       a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
       e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];
       mix(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   /*}
   else
   {
     // fill in m[] with messy stuff
     for (i=0; i<RANDSIZ; i+=8)
     {
       mix(a,b,c,d,e,f,g,h);
       m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
       m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
   }*/

   isaac_fill(ctx);            /* fill in the first set of results */
   ctx->randcnt=RANDSIZ;  /* prepare to use the first set of results */
}

