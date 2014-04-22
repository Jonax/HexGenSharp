/*
 
 src/wgen/atmosphere.c - atmosphere climate & weather simulation
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2014 Ben Golightly <golightly.ben@googlemail.com>

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
 
 Atmosphere simulation as a cellular automation. For more information, please
 see the series of posts on the Hero Extant Development Blog:
 
 http://www.heroextant.net/blog/post/news/20140416-lets-simulate-a-planet-atmosphere-air-pressure-air-density.html
 
 This factors things like direct solar radiation, albedo of surface material,
 etc, into a simulation of a 3D atmosphere considering air temperature, air
 pressure, air density, wind speed, etc.
 
*/

#include "wgen/wgen.h"
#include "graph/graph.h" // pretty graphs
#include <math.h>
#include <stdio.h>


int WindsimInit(Windsim *sim, World *w, size3D size)
{
    if (!sim)       { X2(bad_arg, "NULL sim pointer"); }
    if (!w)         { X2(bad_arg, "NULL world pointer"); }
    if (size.x < 1) { X2(bad_arg, "size.x must be > 0"); }
    if (size.y < 1) { X2(bad_arg, "size.y must be > 0"); }
    if (size.z < 1) { X2(bad_arg, "size.z must be > 0"); }
    
    size2D xy = Size2D(size.x, size.y);
    
    sim->world = w;
    sim->size = size;
    sim->elements = size.x * size.y * size.z;
    sim->height = WINDSIM_HEIGHT;
    
    /* a 3D grid of cells covering the map surface */
    sim->cell = malloc(sizeof(Windcell) * sim->elements);
    if (!sim->cell) { X(alloc_sim_cells); }
    
    /* a 1D grid of cells extending along the z axis away from the surface
     * of the planet, for graphing simulation state by altitude. */
    sim->graphAtmosphere1DCell = malloc(sizeof(GraphAtmosphere1DCell) * sim->size.z);
    if (!sim->graphAtmosphere1DCell) { X(alloc_graph_cells); }
    
    /* 2D buffers for sampling information from the world map at a lower
     * resolution */
    if (!Doubles2DInit(&sim->elevation, xy)) { X(Doubles2DInit_elevation); }
    if (!Doubles2DInit(&sim->sunlight,  xy)) { X(Doubles2DInit_sunlight);  }
    if (!Doubles2DInit(&sim->albedo,    xy)) { X(Doubles2DInit_albedo);    }
    
    return 1;
    
    err_Doubles2DInit_albedo:
        Doubles2DTeardown(&sim->sunlight);
    err_Doubles2DInit_sunlight:
        Doubles2DTeardown(&sim->elevation);
    err_Doubles2DInit_elevation:
        free(sim->graphAtmosphere1DCell);
    err_alloc_graph_cells:
        free(sim->cell);
    err_alloc_sim_cells:
    err_bad_arg:
        return 0;
}


void WindsimTeardown(Windsim *sim)
{
    if (!sim) { X2(bad_arg, "NULL sim pointer"); }
    
    Doubles2DTeardown(&sim->albedo);
    Doubles2DTeardown(&sim->sunlight);
    Doubles2DTeardown(&sim->elevation);
    free(sim->graphAtmosphere1DCell);
    free(sim->cell);
    
    return;
    
    err_bad_arg:
        return;
}


void WindcellInit
(
    Windcell *c,
    double altitude,    // m
    double mass,        // kg
    double temperature, // K
    double moisture,    // kg
    vector3Df dimension // m*m*m
)
{
    if (!c)              { X2(bad_arg,            "NULL cell pointer"); }
    if (altitude    < 0) { X2(bad_arg,    "altitude must be positive"); }
    if (mass        < 0) { X2(bad_arg,        "mass must be positive"); }
    if (temperature < 0) { X2(bad_arg, "temperature must be positive (Kelvin)"); }
    if (moisture    < 0) { X2(bad_arg,    "moisture must be positive"); }
    if (dimension.x < 0) { X2(bad_arg, "dimension.x must be positive"); }
    if (dimension.y < 0) { X2(bad_arg, "dimension.y must be positive"); }
    if (dimension.z < 0) { X2(bad_arg, "dimension.z must be positive"); }
    
    c->altitude    = altitude;
    c->mass        = mass;
    c->temperature = temperature;
    c->moisture    = moisture;
    c->dimension   = dimension;
    
    c->volume = (c->dimension.x * c->dimension.y * c->dimension.z);
    c->volume_reciprocal = 1.0 / c->volume;
    
    c->velocity.x = 0.0;
    c->velocity.y = 0.0;
    c->velocity.z = 0.0;
    
    c->dimension_reciprocal.x = 1.0 / c->dimension.x;
    c->dimension_reciprocal.y = 1.0 / c->dimension.y;
    c->dimension_reciprocal.z = 1.0 / c->dimension.z;
    
    return;
    
    err_bad_arg:
        return;
}


FORCE_INLINE double DesiredMassByDensity(double density, vector3Df dimension)
{
    // mass = density * volume
    double volume = dimension.x * dimension.y * dimension.z;
    return (density * volume);
}


FORCE_INLINE double WindcellDensity(Windcell *cell)
{
    // density = mass / volume
    return (cell->mass * cell->volume_reciprocal);
}


FORCE_INLINE double WindcellPressure(Windcell *cell)
{
    // http://www.st-andrews.ac.uk/~dib2/climate/pressure.html
    const double R_specific = 287.0; // should change based on moisture content
    return (WindcellDensity(cell) * cell->temperature * R_specific);
}


FORCE_INLINE Windcell *WindcellAtXYZ(Windsim *sim, size_t x, size_t y, size_t z)
{
    return &sim->cell
    [
        (z * sim->size.y * sim->size.x) +
        (y * sim->size.x) +
        (x)
    ];
}


FORCE_INLINE Windcell *WindcellAtZI(Windsim *sim, size_t z, size_t i)
{
    return &sim->cell
    [
        (z * sim->size.y * sim->size.x) + i
    ];
}


static double TriangleExtendedOpposite
(
    double radius,
    double opposite,
    double extend
)
{
    /* Given a triangle, with an unknown angle A, an opposite side of known
     * length, a, and known adjacent sides, b and c, both of size radius,
     * find the new length of side a in a triangle formed by extending the
     * radius by a known distance.
     * 
     * In other words, we have a triangle formed by approximating an arc from
     * the center of a planet of known radius, for a certain known size
     * on the surface (given by the opposite argument). We want to know how
     * much bigger the triangle at the top of the arc becomes at higher
     * altitudes, given by the extend argument.
     * 
     * This is used to find the width of a simulation cell at higher altitudes
     * from the surface of the planet. This is simply the cosine rule. */
    
    // Cosine rule: a^2 = b^2 + c^2 - 2bc cosA
    double a = opposite;
    double b = radius;
    double c = radius;
    double cosA = (square(a) - square(b) - square(c)) / (-2.0 * b * c);

    b = radius + extend;
    c = radius + extend;
    a = sqrt(square(b) + square(c) - (2.0 * b * c * cosA));
    
    return a;
}


void WindsimCellsInit(Windsim *sim)
{
    if (!sim) { X2(bad_arg, "NULL sim pointer"); }
    
    /* Initialises a stack of cells over the simulation, the depth of each cell
     * (I will generally use "depth" to describe height in the Z axis, reserving
     * "height" for describing the Y axis) being smaller at lower altitudes
     * in order to give greater accuracy at the more important area. Heights
     * increase in proportion: 1, 2, 3, ... n.
     * 
     * Each cell is initialised with an approximation so that the simulation
     * converges to realistic point as soon as possible.
     * */
    double altitude = 0;
    double n = (double) sim->size.z + 1;
    
    // for each layer
    for (size_t z = 0; z < sim->size.z; z++)
    {
        // smaller depths nearer the surface for accuracy
        // using depths of nx for 
        // and x = 2h/((n)(n-1))
        double x = 2.0 * sim->height / (n * (n - 1));
        double depth = x * (1.0 + (double) z);
        altitude += depth * 0.5; // midpoint
        
        // Find width of cell by extending a triangle from planet radius to
        // height of layer.
        double width = TriangleExtendedOpposite
        (
            sim->world->radius,
            sim->world->dimension.x / (double) sim->size.x,
            altitude
        );
        
        double height = TriangleExtendedOpposite
        (
            sim->world->radius,
            sim->world->dimension.y / (double) sim->size.y,
            altitude
        );
        
        // for width and height
        for (size_t i = 0; i < sim->size.x * sim->size.y; i++)
        {
            Windcell *cell = WindcellAtZI(sim, z, i);
            
            // initialise with an approximation
            WindcellInit
            (
                cell,
                altitude,
                0.15 * width * height * depth, // air mass kg
                273.15, // temperature in Kelvin (0 C)
                0.0, // moisture kg
                Vector3Df(width, height, depth) // m*m*m
            );
        }
        
        altitude += depth * 0.5; // move beyond midpoint
    }
    
    return;
    
    err_bad_arg:
        return;
}


void WindsimStepForce(Windsim *sim, size_t z, size_t i)
{
    Windcell *cell = WindcellAtZI(sim, z, i);
    GraphAtmosphere1DCell *graphCell = &sim->graphAtmosphere1DCell[z];
    
    double re       = sim->world->radius;
    double altitude = cell->altitude;
    
    // Gravity at altitude by the inverse square law
    double gravity = sim->world->gravity * square(re /(re + altitude));
    
    // Cell weight by F=ma
    cell->weight = cell->mass * gravity;
    
    // Pressure by surface area
    double pressure     = WindcellPressure(cell);
    double area_force_z = pressure * (cell->dimension.x * cell->dimension.y);
    cell->force_up      = area_force_z;
    cell->force_down    = area_force_z + cell->weight;
    
    // Graph
    if (i == 0)
    {
        graphCell->size     = cell->dimension;
        graphCell->density  = WindcellDensity(cell);
        graphCell->pressure = WindcellPressure(cell);
        graphCell->altitude = cell->altitude;
        graphCell->velocity = cell->velocity.z;
    }
}


void WindsimStepVelocity(Windsim *sim, size_t z, size_t i)
{
    if (z == 0)
    {
        // assumption: force of air rushing downwards is always met by the
        // surface of the earth, so we start each step at the floor with the
        // cell velocity at zero before acceleration is considered normally.
        Windcell *floorcell = WindcellAtZI(sim, z, i);
        if (floorcell->velocity.z > 0.0) { floorcell->velocity.z = 0.0; }
        
        // steps are evaluated from in order from z to z-1.
        // shere z=0, there is no z-1.
        return;
    }
    
    Windcell *above = WindcellAtZI(sim, z, i);
    Windcell *below = WindcellAtZI(sim, z - 1, i);
    Windcell *from, *to;
    
    double force = (above->force_down - below->force_up);
    
    if (force > 0.0) { from = above; to = below; }
    else             { from = below; to = above; }
    
    // Newton's Laws of Motion
    // change in force = change in velocity = acceleration
    // force = mass * acceleration: F=ma, a = F/m
    from->velocity.z += force / from->mass;
    to->velocity.z   += force / to->mass;
}


FORCE_INLINE void WindcellChangeMass(Windcell *cell, double ammount)
{
    cell->mass += ammount;
    ASSERT(cell->mass >= 0);
}


void WindsimStepMass(Windsim *sim, size_t z, size_t i)
{
    // Transfer of mass and momentum due to velocity
    Windcell *from  = WindcellAtZI(sim, z, i);
    double transfer = from->mass * from->velocity.z * from->dimension_reciprocal.z;
    
    if ((transfer > 0.0) && (z == 0)) { return; }
    if ((transfer < 0.0) && (z == sim->size.z - 1)) { return; }
    
    //assert(transfer >= -from->mass);
    //assert(transfer <= from->mass);
    
    Windcell *to;
    if (transfer > 0.0) { to = WindcellAtZI(sim, z-1, i); }
    else                { to = WindcellAtZI(sim, z+1, i); }
    
    // when transferring mass, it is travelling at a velocity
    // so we want to transfer momentum to a cell
    // p = mv
    double momentum1 = (fabs(transfer) * from->velocity.z);
    double momentum2 = (to->mass * to->velocity.z);
    to->velocity.z += (momentum1 + momentum2) / to->mass;
    
    // and an equal force in the opposite direction accordingly
    momentum2 = (from->mass * from->velocity.z);
    from->velocity.z -= (momentum1 + momentum2) / from->mass;
    
    WindcellChangeMass(from, -fabs(transfer));
    WindcellChangeMass(to,    fabs(transfer));
}


int WindsimRun(Windsim *sim, Image *img, Image *graph, int iterations)
{
    UNUSED(img);
    //if (!WindsimSampleWorld(sim)) { X(WindsimSampleWorld); }
    WindsimCellsInit(sim);
    
    //iterations = 2501;
    //iterations = 3;
    //iterations = 50;
    iterations = 10001;
    
    printf("Wind simulation: %d iterations over %dx%dx%d cells\n",
        iterations, (int) sim->size.x, (int) sim->size.y, (int) sim->size.z);
    
    for (int iteration = 0; iteration < iterations; iteration++)
    {
        if (iteration % 100 == 0) { printf("Windsim: %d/%d\n", iteration, iterations - 1); }
        
        for (size_t i = 0; i < sim->size.x * sim->size.y; i++)
        {
            for (size_t z = 0; z < sim->size.z; z++)
            {
                // Calculate Forces due to pressure and gravity
                WindsimStepForce(sim, z, i);
            }
            
            for (size_t z = 0; z < sim->size.z; z++)
            {
                // Calculate change in velocity due to forces
                WindsimStepVelocity(sim, z, i);
            }
            
            for (size_t z = 0; z < sim->size.z; z++)
            {
                // Transfer of mass and momentum due to velocity
                WindsimStepMass(sim, z, i);
            }
        }
        
        if (iteration % 500 == 0)
        //if ((iteration <= 50) || (iteration % 50 == 0))
        {
            char title[256];
            char filegraph[256];
            sprintf(title, "iteration %d/%d", iteration, iterations - 1);
            sprintf(filegraph, ".windsim/graph/graph-%d.png", iteration);
            
            GraphAtmosphere1D(sim->world->generator->grapher, title, graph,
                sim->size.z, sim->graphAtmosphere1DCell,
                sim->world->radius, sim->world->gravity, sim->height);
            ImageSaveTo(graph, filegraph);
        }
    }
    
    return 1;
}

