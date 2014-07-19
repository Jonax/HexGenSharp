/*
 
 src/wgen/world.c - world model
 
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
#include "noise/simplexnoise1234.h"
#include <stdio.h>
#include <math.h> // log

#define PI 3.14159265359


int WorldInit(World *w, Generator *g, size2D size)
{
    if (!w) { X2(bad_arg, "NULL world pointer");}
    if (!g) { X2(bad_arg, "NULL generator pointer");}
    if (size.x == 0) { X2(bad_arg, "bad size"); }
    if (size.y == 0) { X2(bad_arg, "bad size"); }
    
    *w->name = '\0';
    *w->desc = '\0';
    w->generator = g;
    
    w->defined_planet = 0;
    w->defined_seasons = 0;
    w->defined_area = 0;
    
    if (!Doubles2DInit(&w->elevation, size)) { X(Doubles2DInit_elevation); }
    if (!Doubles2DInit(&w->sunlight,  size)) { X(Doubles2DInit_sunlight);  }
    
    return 1;
    
    err_Doubles2DInit_sunlight:
        Doubles2DTeardown(&w->elevation);
    err_Doubles2DInit_elevation:
    err_bad_arg:
        return 0;
}


int WorldDefinePlanet
(
    World *w,
    double radius,       // in metres e.g. 6371000.0
    double gravity,      // gravity at surface, in e.g. 9.81 m/s^-2
    double distance_sun, // in astronomical units e.g. 1.0 AU for Earth
    double solar_luminosity // normalised relative to our sun. 1.0 => 3.846×10^26 Watts
)
{
    if (!w)                      { X2(bad_arg, "NULL world pointer");  }
    if (radius <= 0.0)           { X2(bad_arg, "radius must be > 0");  }
    if (gravity <= 0.0)          { X2(bad_arg, "gravity must be > 0"); }
    if (distance_sun <= 0.0)     { X2(bad_arg, "distance from sun must be > 0"); }
    if (solar_luminosity <= 0.0) { X2(bad_arg, "solar_luminosity must be > 0");  }
    
    w->radius            = radius;
    w->gravity           = gravity;
    w->distance_from_sun = distance_sun;
    w->solar_luminosity  = solar_luminosity;
    w->defined_planet    = 1;
    
    return 1;
    
    err_bad_arg:
        return 0;
}


int WorldDefineSeasons
(
    World *w,
    double seasonal_tilt,    // degrees - severity of seasons (-180 to 180; Earth is 23.5)
    double northern_solstice // point in orbit (0.0 to 1.0) where this occurs (Earth is at 0.222)
)
{
    if (!w)                        { X2(bad_arg, "NULL world pointer");   }
    if (seasonal_tilt < -180.0)    { X2(bad_arg, "tilt must be >= -180"); }
    if (seasonal_tilt > +180.0)    { X2(bad_arg, "tilt must be <= 180");  }
    if (northern_solstice <= -1.0) { X2(bad_arg, "northern solstice must be > -1.0"); }
    if (northern_solstice >= +1.0) { X2(bad_arg, "northern solstice must be < 1.0");  }
    
    w->axial_tilt        = seasonal_tilt;
    w->northern_solstice = northern_solstice;
    w->defined_seasons   = 1;
    
    return 1;
    
    err_bad_arg:
        return 0;
}


int WorldDefineArea
(
    World *w,
    geocoordinate center,   // xy center of map on sphere
    vector3Df dimension     // surface width from top/bottom, left/right, floor/ceil
)
{
    if (!w)                          { X2(bad_arg,   "NULL world pointer");      }
    if (!GeoCoordinateValid(center)) { X2(bad_arg,   "geocoordinate invalid");   }
    if (dimension.x <= 0.0)          { X2(bad_arg,   "dimension.x must be > 0"); }
    if (dimension.y <= 0.0)          { X2(bad_arg,   "dimension.y must be > 0"); }
    if (dimension.z <= 0.0)          { X2(bad_arg,   "dimension.z must be > 0"); }
    if (!w->defined_planet)          { X2(bad_state, "WorldDefinePlanet first"); }
    
    double circumference = 2.0 * PI * w->radius;
    if (circumference < dimension.x)
        { X2(bad_dimension, "dimension.x is too large for the planet radius"); }
    if (circumference < dimension.y)
        { X2(bad_dimension, "dimension.y is too large for the planet radius"); }
    if (dimension.z > 50 * 1000.0)
        { X2(bad_dimension, "dimension.z is too large for an atmosphere simulation"); }
    
    w->center       = center;
    w->dimension    = dimension;
    w->defined_area = 1;
    
    return 1;
    
    err_bad_dimension:
    err_bad_state:
    err_bad_arg:
        return 0;
}


static void WorldApplyNoise
(
    World *w,
    double energy,          // 0.6 to 3.0.  Higher == more islands
    double turbulance       // -0.5 to 1.0. Controls shape and contrast
)
{
    Doubles2D *elevation = &w->elevation;
    
    // longest length
    size_t length = size_t_max(elevation->size.x, elevation->size.y);
    
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
            (w->generator->mask_sampler(w->generator->mask_sampler_rsc,
                                         x, y, width, height));
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
    WorldApplyNoise(w, energy, turbulance);
    
    /* Normalise elevation between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    /* Create a sea floor by clamping anything below 0.6 */
    Doubles2DClampFloorTo(&w->elevation, SEA_PROPORTION, SEA_PROPORTION);
    
    /* Normalise elevation again between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    return 1;
}


int WorldLandmassAtTopEdge(World *w)
{
    double *v = w->elevation.values;
    
    for (size_t x = 0; x < w->elevation.size.x; x++)
    {
        if (*v++ >= SEA_LEVEL) return 1;
    }
    
    return 0;
}


int WorldLandmassAtBottomEdge(World *w)
{
    double *v = w->elevation.values;
    v += (w->elevation.size.x * (w->elevation.size.y - 1));
    
    for (size_t x = 0; x < w->elevation.size.x; x++)
    {
        if (*v++ >= SEA_LEVEL) return 1;
    }
    
    return 0;
}


int WorldLandmassAtLeftEdge(World *w)
{
    double *v = w->elevation.values;
    
    for (size_t y = 0; y < w->elevation.size.y; y++)
    {
        if (*v >= SEA_LEVEL) return 1;
        
        v += w->elevation.size.x;
    }
    
    return 0;
}


int WorldLandmassAtRightEdge(World *w)
{
    double *v = w->elevation.values;
    v += w->elevation.size.x - 1;
    
    for (size_t y = 0; y < w->elevation.size.y; y++)
    {
        if (*v >= SEA_LEVEL) return 1;
        
        v += w->elevation.size.x;
    }
    
    return 0;
}


double WorldLandMassProportion(World *w)
{
    unsigned int count = 0;
    double *v = w->elevation.values;
    
    for (size_t i = 0; i < w->elevation.elements; i++, v++)
    {
        if (*v > SEA_LEVEL) { count++; }
    }
    
    return (((double) count) / ((double) w->elevation.elements));
}


int WorldCalculateDirectSolarRadiation(World *world, double orbit)
{
    if (orbit < 0.0)             { X2(bad_arg,   "orbit must be >= 0.0");     }
    if (orbit > 1.0)             { X2(bad_arg,   "orbit must be >= 1.0");     }
    if (!world->defined_planet)  { X2(bad_state, "WorldDefinePlanet first");  }
    if (!world->defined_seasons) { X2(bad_state, "WorldDefineSeasons first"); }
    if (!world->defined_area)    { X2(bad_state, "WorldDefineArea first");    }
    
    return PlanetCalculateDirectSolarRadiation
    (
        &world->sunlight,
        orbit,
        world->northern_solstice,
        world->axial_tilt,
        world->radius,
        world->distance_from_sun,
        world->solar_luminosity,
        world->center,
        world->dimension.y
    );
    
    err_bad_state:
    err_bad_arg:
        return 0;
}


/* Model the direct solar radiation over the map at any given time (direct i.e.
   sunshine). This EXCLUDES diffuse radiation (e.g. atmospheric scattering)
   as this depends on current local cloud formation.
   
    With reference to:
    
    1. W. B. Stine and M. Geyer, "The Sun's Energy" in
       *Power From The Sun*, (Online), 2001.
       http://www.powerfromthesun.net/Book/chapter02/chapter02.html
*/
int PlanetCalculateDirectSolarRadiation
(
    Doubles2D *buffer,
    double orbit,               // yearly orbit normalised 0.0 to 1.0
    double northern_solstace,   // point in orbit where this occurs (Earth is at 0.222)
    double axial_tilt,          // degrees - severity of seasons (-180 to 180; Earth is 23.5)
    double planet_radius,       // in metres
    double distance_from_sun,   // in astronomical units e.g. 1.0 AU for Earth
    double solar_luminosity,    // 1.0 for our Sun ~= 3.846 × 10^26 Watts
    geocoordinate map_center,   // see wgen/geocoordinates.h
    double mapsize              // kilometres between Northern- and Southern-most points
)
{
    double *v = buffer->values;
    
    // Denormalise orbit to a specific day
    double day = orbit;
    
    // Denormalise to SI units (metres, watts, etc).
    double si_solar_luminosity  = solar_luminosity * 3.846 * pow(10.0, 26.0);
    double si_distance_from_sun = distance_from_sun * 149600000000.0;
    //double si_planet_surface_area = (4.0 * PI * si_planet_radius * si_planet_radius);
    double si_surface_distance_from_sun = si_distance_from_sun - planet_radius;
    
    // Radiation on an imaginary surface at the planet's edge, perpendicular
    // to the direction of rays from the sun, using Inverse Square Law.
    double incident_radiance = si_solar_luminosity /
        (4.0 * PI * si_surface_distance_from_sun * si_surface_distance_from_sun);
    
    for (size_t y = 0; y < buffer->size.y; y++)
    {
        // latitude of a point
        double yrange = (double) y / (double) buffer->size.y; // 0.0 to 1.0
        double yoffset = (yrange - 0.5) * mapsize;
        
        geocoordinate ypoint = GeoCoordinateTranslate(map_center,
            planet_radius, yoffset, 180.0);
        
        // Radiation at a point
        double phi = radians(LatitudeToDouble(GeoCoordinateLatitude(ypoint)));
        double delta = asin(
            sin(radians(axial_tilt)) *
            sin(2 * PI * (day - northern_solstace))
        );
        
        double angle = radians(90.0) - phi + delta;
        double point_radiance = incident_radiance * sin(angle);
        
        for (size_t x = 0; x < buffer->size.x; x++)
        {
            *v++ = point_radiance;
        }
    }
    
    Doubles2DNormaliseMaximum(buffer);
    
    printf
    (
        "Orbit %.1f/1.0 radiance (W/m^2): "
        "low=%.2f "
        "high=%.2f "
        "(incident=%.2f)"
        "\n",
        day,
        buffer->minimum,
        buffer->maximum,
        incident_radiance
    );
    
    return 1;
}

