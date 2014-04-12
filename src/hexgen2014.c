#include "clock.h"
#include "rng/rng.h"
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    RNG *rng = RNGNew(RNG_ISAAC, 0);
    if (!rng) { return 1; }
    
    Clock c; ClockInit(&c);
    
    printf("%u %u %u\n",
        RNGNext(uint32_t, rng),
        RNGNext(uint32_t, rng),
        RNGNext(uint32_t, rng)
    );
    
    int d = 0;
    for (int i = 0; i < 1000000; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            if (RNGNext(uint32_t, rng) % 100 == 0) { d++; }
        }
    }
    
    ClockSync(&c);
    printf("ClockMonotonicTime: %d ms\n", (int) ClockMonotonicTime(&c));
    printf("d: %d\n", d);
    
    RNGFree(rng);
    
    return 0;
}
