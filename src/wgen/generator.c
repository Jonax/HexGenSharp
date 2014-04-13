#include "base.h"
#include "wgen/wgen.h"

int GeneratorInit(Generator *g, unsigned int seed)
{
    g->rng = RNGNew(RNG_ISAAC, seed);
    if (!g->rng) { X(RNGNew); }
    
    if (!ClockInit(&g->clock)) { X(ClockInit); }
    
    g->mask_sampler = SampleDefault;
    g->mask_sampler_rsc = NULL;
    
    return 1;
    
    err_ClockInit:
        RNGFree(g->rng);
    err_RNGNew:
        return 0;
}


int GeneratorUseMaskSampler
(
    Generator *g,
    double (*sampler)(void *p, double x, double y, double w, double h)
)
{
    g->mask_sampler = sampler;
    return 1;
}
