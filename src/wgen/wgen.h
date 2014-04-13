/*
 
 src/wgen/wgen.h - common wgen header
 
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

#ifndef HG14_WGEN_H
#   define HG14_WGEN_H

# define WORLD_NAME_SIZE  64
# define WORLD_DESC_SIZE  512
# define SEASON_NAME_SIZE 32
# define SEASON_DESC_SIZE 256

# define SEA_PROPORTION 0.6
# define SEA_LEVEL      0.15

#include "wgen/geocoordinates.h"
#include "rng/rng.h"
#include "clock.h"
#include "types.h"
#include "image.h"

typedef struct Generator Generator;
typedef struct Season    Season;
typedef struct Doubles2D Doubles2D;
typedef struct World     World;

struct Doubles2D
{
    size_t bytes;
    size_t elements;
    size2D size;
    double *values;
    double minimum, maximum; // for denormalising
};

struct Season
{
    char name[SEASON_NAME_SIZE];
    char desc[SEASON_DESC_SIZE];
    
    double rainfall;        // normalised around 1.0
    double tilt;            // normalised around 0.0, shifts latitude up/down
    double wind_direction;  // normalised around 1.0, shifts wind
    
    Doubles2D temperature;  // normalised 0.0 - 1.0
    
    Season *next;
};

struct World
{
    char name[WORLD_NAME_SIZE];
    char desc[WORLD_DESC_SIZE];
    Generator *generator;
    
    double axial_tilt; // degrees; earth is 23.4°
    double latitude[2]; // top and bottom; try 55°N to 50°N for UK weather
    double radius; // normalised 1.0 is earth
    
    // array of values for every point
    Doubles2D elevation;  // normalised 0.0 - 1.0
    Doubles2D sunlight;   // direct radiance normalised 0.0 - 1.0
    
    Season *seasons;
};

struct Generator
{
    RNG *rng;
    Clock clock;
    
    World world;
    
    double (*mask_sampler)(void *rsc, double x, double y, double w, double h);
    void *mask_sampler_rsc; // used for custom heightmap shapes
};


// === doubles2D.c ===
int Doubles2DInit(Doubles2D *d, size2D size);
void Doubles2DTeardown(Doubles2D *d);
void Doubles2DZeroFill(Doubles2D *d);
void Doubles2DNormalise(Doubles2D *d);
void Doubles2DNormaliseMaximum(Doubles2D *d);
void Doubles2DClampFloorTo(Doubles2D *d, double min, double to);
void Doubles2DClampCeilingTo(Doubles2D *d, double max, double to);
void Doubles2DSquare(Doubles2D *d);

// === generator.c ===
int GeneratorInit(Generator *g, unsigned int seed);
int GeneratorUseMaskSampler(Generator *g, double (*sampler)(void *p, double x, double y, double w, double h));

// === world.c ===
int WorldInit(World *w, Generator *g, size2D size);
int WorldGenerateHeightmap(World *w, double energy, double turbulence);
int WorldCalculateDirectSolarRadiation
(
    Doubles2D *buffer,
    double orbit,               // yearly orbit normalised 0.0 to 1.0
    double axial_tilt,          // degrees - severity of seasons (-180 to 180; Earth is 23.5)
    double planet_radius,       // where 1.0 is the mean radius of the Earth
    double distance_from_sun,   // in astronomical units e.g. 1.0 AU for Earth
    double solar_luminosity,    // 1.0 for our Sun ~= 3.846 × 10^26 Watts
    geocoordinate map_center,   // see wgen/geocoordinates.h
    double mapsize              // kilometres between Northern- and Southern-most points
);

// === mask.c ====
double SampleDefault(void *p, double x, double y, double w, double h);
double SampleCircleGradiant(void *p, double x, double y, double w, double h);

// === render.c ===
int WorldRender_Elevation_Raw(World *w, Image *i);
int WorldRender_Elevation_Quick(World *w, Image *m);
int WorldRender_Elevation_Nice(World *w, Image *m);

int WorldRender_Sunlight_Raw(World *w, Image *i);
int WorldRender_Sunlight_Quick(World *w, Image *i);
int WorldRender_Sunlight_Nice(World *w, Image *i);

#endif
