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

# define NORTHERN_SOLSTICE_EARTH 0.222
# define SEASONAL_TILT_EARTH 23.5

# define WINDSIM_HEIGHT 100000.0 // 100km

#include "wgen/geocoordinates.h"
#include "rng/rng.h"
#include "clock.h"
#include "types.h"
#include "image.h"
#include "graph/graph.h"

typedef struct Generator Generator;
typedef struct Season    Season;
typedef struct Doubles2D Doubles2D;
typedef struct World     World;
typedef struct Windsim   Windsim;
typedef struct Windcell  Windcell;

struct Doubles2D
{
    size_t bytes;
    size_t elements;
    size2D size;
    double *values;
    double minimum, maximum; // for denormalising
};

struct World
{
    char name[WORLD_NAME_SIZE];
    char desc[WORLD_DESC_SIZE];
    Generator *generator;
    
    int defined_seasons;
    double axial_tilt;          // degrees; earth is 23.4°
    double northern_solstice;   // seasonal lag; point in orbit where this occurs (Earth is at 0.222)
    
    int defined_planet;
    double radius;              // metres
    double gravity;             // m/s^2
    double distance_from_sun;   // in AU
    double solar_luminosity;    // normalised relative to our sun at 1.0. Watts.
    
    int defined_area;
    geocoordinate center;       // xy center of map on sphere
    vector3Df dimension;        // surface width from top/bottom, left/right, floor/ceil
    
    // array of values for every point
    Doubles2D elevation;  // normalised 0.0 - 1.0
    Doubles2D sunlight;   // direct radiance normalised 0.0 - 1.0
};

struct Generator
{
    RNG *rng;
    Clock clock;
    
    World world;
    
    double (*mask_sampler)(void *rsc, double x, double y, double w, double h);
    void *mask_sampler_rsc; // used for custom heightmap shapes
};

struct Windcell
{
    // "Intrinsic"
    double mass;
    double temperature;
    double moisture;
    double altitude;
    vector3Df dimension; // of any cell
    vector3Df velocity;
    
    // Derived
    double volume;
    double volume_reciprocal;
    double weight;
    
    // Forces
    double force_up;
    double force_down;
    double force_north_south;
    double force_east_west;
};

struct Windsim
{
    World *world;
    size3D size;
    size_t elements;
    double height; // metres
    
    Windcell *cell;
    
    GraphAtmosphere1DCell *graphAtmosphere1DCell;
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
int Doubles2DDownsample(Doubles2D *dest, Doubles2D *src);

// === generator.c ===
int GeneratorInit(Generator *g, unsigned int seed);
int GeneratorUseMaskSampler(Generator *g, double (*sampler)(void *p, double x, double y, double w, double h));

// === windsim.c ===
int WindsimInit(Windsim *sim, World *w, size3D size);
int WindsimSampleWorld(Windsim *sim);
int WindsimRun(Windsim *sim, Image *img, Image *graph, int it);
int Windsim1D(Windsim *sim, World *w, size3D size);

// === world.c ===
int WorldInit(World *w, Generator *g, size2D size);

int WorldDefinePlanet
(
    World *w,
    double radius,       // in metres e.g. 6371000.0
    double gravity,      // gravity at surface, in e.g. 9.81 m/s^-2
    double distance_sun, // in astronomical units e.g. 1.0 AU for Earth
    double solar_luminosity // normalised relative to our sun. 1.0 => 3.846×10^26 Watts
);

int WorldDefineSeasons
(
    World *w,
    double seasonal_tilt,    // degrees - severity of seasons (-180 to 180; Earth is 23.5)
    double northern_solstice // point in orbit (0.0 to 1.0) where this occurs (Earth is at 0.222)
);

int WorldDefineArea
(
    World *w,
    geocoordinate center,   // xy center of map on sphere
    vector3Df dimension     // surface width from top/bottom, left/right, floor/ceil
);

int WorldGenerateHeightmap(World *w, double energy, double turbulence);
int WorldLandmassAtTopEdge(World *w);
int WorldLandmassAtBottomEdge(World *w);
int WorldLandmassAtLeftEdge(World *w);
int WorldLandmassAtRightEdge(World *w);
double WorldLandMassProportion(World *w);

int WorldCalculateDirectSolarRadiation(World *w, double orbit);

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

int WindsimRender_Test(Windsim *w, Image *m);
int WindsimRender_Gravity(Windsim *w, Image *m);
int WindsimRender_Force(Windsim *w, Image *m);
int WindsimRender_Pressure(Windsim *w, Image *m);
int WindsimRender_Density(Windsim *w, Image *m);
int WindsimRender_Temperature(Windsim *w, Image *m);

#endif
