#include "base.h"
#include "wgen/wgen.h"
#include "noise/simplexnoise1234.h"
#include <math.h>
#include <stdio.h>

int WorldInit(World *w, Generator *g, size2D size)
{
    if (!w) { X2(bad_arg, "NULL world pointer");}
    if (!g) { X2(bad_arg, "NULL generator pointer");}
    if (size.x == 0) { X2(bad_arg, "bad size"); }
    if (size.y == 0) { X2(bad_arg, "bad size"); }
    
    *w->name = '\0';
    *w->desc = '\0';
    w->generator = g;
    
    if (!Doubles2DInit(&w->elevation, size))   { X(Doubles2DInit_elevation); }
    if (!Doubles2DInit(&w->temperature, size)) { X(Doubles2DInit_temperature); }
    
    return 1;
    
    err_Doubles2DInit_temperature:
        Doubles2DTeardown(&w->elevation);
    err_Doubles2DInit_elevation:
    err_bad_arg:
        return 0;
}


static double SampleCircleGradiant
(
    void *p,
    double x,
    double y,
    double w,
    double h
)
{
    UNUSED(p);
    
    /* Distance from center */
    double dx = (w*0.5) - x;
    double dy = (h*0.5) - y;
    
    /* Max radius for spherical falloff */
    double maxr = w * 0.5;
    
    /* Distance of point from center (basic pythag) */
    double r = sqrt((dx*dx) + (dy*dy));
    
    if (r > maxr) { return -1.0; }
    double scale = -(r / maxr);
    
    return scale;
}



static void WorldApplyNoise
(
    World *w,
    double energy,          // 0.6 to 3.0.  Higher == more islands
    double turbulance,      // -0.5 to 1.0. Controls shape and contrast
    double (*sample)(void *p, double x, double y, double w, double h)
)
{
    Doubles2D *elevation = &w->elevation;
    
    // longest length
    size_t length = max_size_t(elevation->size.x, elevation->size.y);
    
    /* This function applies multiple octaves of noise in order to generate an
     * interesting heightmap.
     * 
     * We want the smallest octave of noise to have either pixel or sub-pixel
     * interpolation for detail.
     * 
     * The last octave is at 2^(iterations-1)
     * Desired detail: for length/2^(iterations-1) to be <= 1 pixel
     * therefore iterations = log_2(length) + 1, rounded up
     */
    unsigned int iterations = (unsigned int)
        (1.5 + (log((double) length) / log(2.0)));
    
    printf("Octaves of noise for %ux%u : %u\n",
        (unsigned int) length, (unsigned int) length, iterations);
    
    double width  = (double) elevation->size.x;
    double height = (double) elevation->size.y;
    
    /* How chaotic should the first octave of noise be? For a low value we are
     * "zoomed out" and interpolating between two noise values. This will give
     * us a smooth gradient between points. This is good for generating
     * strongly defined continents.
     * 
     * For a high value we are zoomed in and will cross more distinct values.
     * This is good for generating land that is broken up.
     * 
     * Try values from 0.5 to 3.0.
     */
    double scale = (energy / (double) length);
    double scale_r = 1.0 / scale; // reciprocal
    
    /* The first octave is what dominates the shape of the map. Further octaves
     * give finer detail. Therefore if you want a stand-alone continent in the
     * centre of the map, you can set the first octave to be a circular
     * gradient. Or you can use your own image to get a custom shape.
     */
    for (size_t i = 0; i < elevation->elements; i++)
    {
        double y = (double) ((unsigned int) (i / elevation->size.x));
        double x = (double) ((unsigned int) (i % elevation->size.x));
        
        elevation->values[i] = scale_r *
            sample(w->generator->sampler_rsc, x, y, width, height);
    }
    
    /* Octaves of noise */
    for (unsigned int i = 1; i < iterations; i++)
    {
        scale *= 2.0;   // double the scale for each octave
        scale_r *= 0.5; // this halves the reciprocal
    
        for (size_t j = 0; j < elevation->elements; j++)
        {
            double y = (double) ((unsigned int) (j / elevation->size.x));
            double x = (double) ((unsigned int) (j % elevation->size.x));
            
            double turb0 = fabs(snoise2(x * scale * 3.0, y * scale * 3.0));
            double turb1 = fabs(snoise2(x * scale * 7.0, y * scale * 7.0));
            double turb = turb0 * turb1 * turbulance;
            
            elevation->values[j] += scale_r *
                (snoise2(x * scale, y * scale) + turb);
        }
    }
}


int WorldGenerateHeightmap
(
    World *w,
    double energy,          // 0.6 to 3.0.  Higher == more islands
    double turbulance       // -0.5 to 1.0. Controls shape and contrast
)
{
    turbulance /= energy;
    
    /* Seed the state used by the noise function */
    snoise_randomise(w->generator->rng);
    
    /* Apply several octaves of noise at a specific scale */
    WorldApplyNoise(w, energy, turbulance, SampleCircleGradiant);
    
    /* Normalise elevation between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    /* Create a sea floor by clamping anything below 0.6 */
    Doubles2DClampFloorTo(&w->elevation, 0.6, 0.6);
    
    /* Normalise elevation again between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    return 1;
}
