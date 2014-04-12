
#include "rng/rng.h"
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    RNG *rng = RNGNew(RNG_ISAAC, 0);
    if (!rng) { return 1; }
    
    printf("%u %u %u\n",
        RNGNext(uint32_t, rng),
        RNGNext(uint32_t, rng),
        RNGNext(uint32_t, rng)
    );
    
    RNGFree(rng);
    
    return 0;
}
