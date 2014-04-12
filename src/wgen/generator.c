#include "base.h"
#include "wgen/wgen.h"

int GeneratorInit(Generator *g, unsigned int seed)
{
    g->rng = RNGNew(RNG_ISAAC, seed);
    if (!g->rng) { X(RNGNew); }
    
    if (!ClockInit(&g->clock)) { X(ClockInit); }
    
    g->seasons = NULL;
    g->sampler_rsc = NULL;
    
    return 1;
    
    err_ClockInit:
        RNGFree(g->rng);
    err_RNGNew:
        return 0;
}
