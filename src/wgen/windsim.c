/*
 
 src/wgen/windsim.c - wind simulation
 
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
 
 Wind simulation as a celluar automation. For more information, please see
 the series of posts on the Hero Extant development blog:
 
 http://www.heroextant.net/blog/post/news/20140416-lets-simulate-a-planet-atmosphere-air-pressure-air-density.html
 
*/

#include "wgen/wgen.h"
#include <math.h>
#include <stdio.h>

FORCE_INLINE double Square(double a) { return a * a; }
FORCE_INLINE double maxf(double a, double b) { if (a > b) { return a; } else { return b; } }

int WindsimInit(Windsim *sim, World *w, size3D size)
{
    sim->world = w;
    sim->size = size;
    sim->elements = size.x * size.y * size.z;
    sim->height = WINDSIM_HEIGHT;
    
    sim->cell = malloc(sizeof(Windcell) * sim->elements);
    if (!sim->cell) { X(alloc_sim_cells); }
    
    sim->graphAtmosphere1DCell = malloc(sizeof(GraphAtmosphere1DCell) * sim->size.z);
    if (!sim->graphAtmosphere1DCell) { X(alloc_graph_cells); }
    
    return 1;
    
    err_alloc_graph_cells:
        free(sim->cell);
    err_alloc_sim_cells:
        return 0;
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
    if (!c) { X2(bad_argument, "NULL cell pointer"); }
    
    c->altitude    = altitude;
    c->mass        = mass;
    c->temperature = temperature;
    c->moisture    = moisture;
    c->dimension   = dimension;
    
    c->volume = (c->dimension.x * c->dimension.y * c->dimension.z);
    c->volume_reciprocal = 1.0 / c->volume;
    
    return;
    
    err_bad_argument:
        return;
}


double WindcellDensity(Windcell *cell)
{
    // density = mass / volume
    return (cell->mass * cell->volume_reciprocal);
}


double WindcellPressure(Windcell *cell)
{
    // http://www.st-andrews.ac.uk/~dib2/climate/pressure.html
    const double R_specific = 287.0; // should change based on moisture content
    return (WindcellDensity(cell) * cell->temperature * R_specific);
}


/*
void WindcellPressurise(Windcell *cell)
{
    
    cell->pressure = (cell->density * cell->temperature * 287.0);
    // 287.0 should change based on moisture content
}*/

// pressure ~= (0.5 x density) x speed^2
// speed = sqrt(2 * pressure / density)


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
    // Cosine rule: a^2 = b^2 + c^2 - 2bc cosA
    double a = opposite;
    double b = radius;
    double c = radius;
    double cosA = (Square(a) - Square(b) - Square(c)) / (-2.0 * b * c);

    b = radius + extend;
    c = radius + extend;
    a = sqrt(Square(b) + Square(c) - (2.0 * b * c * cosA));
    
    return a;
}


void WindsimCellsInit(Windsim *sim)
{
    for (size_t z = 0; z < sim->size.z; z++)
    {
        double zf       = 0.5 + (double) z;
        double ceiling  = 0.0 + (double) sim->size.z;
        double altitude = sim->height * (zf / ceiling);
        double depth    = sim->height / ceiling;
        
        printf("Windsim cell layer %d/%d, altitude %.2f\n",
            (int) z, (int) sim->size.z -1, altitude);
        
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
        
        printf("Windsim cell volume %.2fx%.2fx%.2f m\n",
                width, height, depth);
        
        for (size_t i = 0; i < sim->size.x * sim->size.y; i++)
        {
            Windcell *cell = WindcellAtZI(sim, z, i);
            
            WindcellInit
            (
                cell,
                altitude,
                0.15 * width * height * depth, // air mass kg
                273.15, // temperature 0 K
                0.0, // moisture kg
                Vector3Df(width, height, depth) // m*m*m
            );
        }
    }
}


void WindsimStepCell(Windsim *sim, size_t z, size_t i)
{
    Windcell *cell = WindcellAtZI(sim, z, i);
    GraphAtmosphere1DCell *graphCell = &sim->graphAtmosphere1DCell[z];
    
    double re       = sim->world->radius;
    double altitude = cell->altitude;
    
    // Gravity at altitude by the inverse square law
    double gravity = sim->world->gravity * Square(re /(re + altitude));
    
    // Cell weight by F=ma
    cell->weight = cell->mass * gravity;
    
    double pressure     = WindcellPressure(cell);
    double area_force_z = pressure * (cell->dimension.x * cell->dimension.y);
    cell->force_up      = area_force_z;
    cell->force_down    = area_force_z + cell->weight;
    
    if (i == 0)
    {
        //printf("cell layer %d dimension %d");
    }
    /*
    printf("z%02d: h %.2f m, g %.2f m/s^2, density %.2f KG/m^3, mass %.2f kg, force of g %.2f N\n",
        (int) z, height, gravity, cell->density, mass, force);
    printf("     pressure: %.2f N/m^2 => %2f N vertical => %.2f N up, %.2f N down\n\n", cell->pressure,
        cell->pressure * (1000.0 * 1000.0), upforce, downforce);
    
    printf("cell z%0d: h %.2f m, density %.2f KG/M^3, pressure %.2f\n",
        (int) z, height, cell->density, cell->pressure);
    */
    
    if (i == 0)
    {
        graphCell->size     = cell->dimension;
        graphCell->density  = WindcellDensity(cell);
        graphCell->pressure = WindcellPressure(cell);
        graphCell->altitude = cell->altitude;
    }
    
    // cell->torque = ||r|| ||F||;
    // moment = force × distance
    // at surface, moment = radius
    // m_a + m_b = m_c => I_a + I_b = I_c
    
    // http://eesc.columbia.edu/courses/ees/climate/lectures/atm_dyn.html
}


double ChangeMass(Windcell *cell, double ammount)
{
    cell->mass += ammount;
    if (cell->mass < 0) { ammount -= cell->mass; cell->mass = 0; }
    
    return ammount;
}


void WindsimResolveCell(Windsim *sim, size_t z, size_t i)
{
    Windcell *above = WindcellAtZI(sim, z + 1, i);
    Windcell *below = WindcellAtZI(sim, z, i);
    Windcell *from, *to;
    
    double diff = (above->force_down - below->force_up);
    
    if (diff > 0.0) { from = above; to = below; }
    else { from = below; to = above; }
    // > 0.0 means above pushes down
    
    // diff is a vector force in N
    
    // f = ma
    // so a = f/m
    // now assume time = 1 second and completely make up some physics that
    // probably isn't correct, but convert acceleration into distance
    // and use that distance as a proportion of height to estimate what moves
    double mass = from->mass;
    if (mass < 1.0) { return; }
    
    double h = from->dimension.z;
    double d = diff / mass;
    double proportion = d / h;
    double transfer = fabs(proportion * mass);
    transfer *= 10; // speed up
    
    if (i == 0)
    {
    //    printf("%d->%d: force %.2f N means a transfer of proportion %.6f which = mass %.0f ton of %.0f ton\n",
      //  (int) z + 1, (int) z, diff, proportion, transfer / 1000.0, mass / 1000.0);
    }
    if (transfer > mass) { transfer = mass; }
    ChangeMass(from, -transfer);
    ChangeMass(to, transfer);
    
    /*
    
    if (above->density < STEP_SIZE) { return; }
    if (below->density < STEP_SIZE) { return; }
    
    if (diff > 0.0) { above->density -= STEP_SIZE; below->density += STEP_SIZE; }
    else { above->density += STEP_SIZE; below->density -= STEP_SIZE; }
    */
}


#include "graph/graph.h"
int WindsimRun(Windsim *sim, Image *img, Image *graph, int iterations)
{
    UNUSED(img);
    //if (!WindsimSampleWorld(sim)) { X(WindsimSampleWorld); }
    WindsimCellsInit(sim);
    
    iterations = 2501;
    
    printf("Wind simulation: %d iterations over %dx%dx%d cells\n",
        iterations, (int) sim->size.x, (int) sim->size.y, (int) sim->size.z);
    
    for (int iteration = 0; iteration < iterations; iteration++)
    {
        if (iteration % 10 == 0) { printf("Windsim: %d/%d\n", iteration, iterations - 1); }
        
        for (size_t i = 0; i < sim->size.x * sim->size.y; i++)
        {
            for (size_t z = 0; z < sim->size.z; z++)
            {
                WindsimStepCell(sim, z, i);
            }
            
            for (size_t z = 0; z < sim->size.z - 1; z++)
            {
                WindsimResolveCell(sim, z, i);
            }
        }
        
        if (iteration % 250 == 0)
        //if (1)
        {
            char title[256];
            char filegraph[256];
            sprintf(title, "iteration %d/%d", iteration, iterations - 1);
            sprintf(filegraph, ".windsim/graph/graph-%d.png", iteration);
            
            GraphAtmosphere1D(title, graph, sim->size.z, sim->graphAtmosphere1DCell,
                sim->world->radius, sim->world->gravity, sim->height);
            ImageSaveTo(graph, filegraph);
        }
    }
    
    return 1;
}




/* WIND:
 * 
 * http://www.srh.noaa.gov/jetstream/synoptic/wind.htm
 * http://www.aos.wisc.edu/~aalopez/aos101/wk11.html
 * http://eesc.columbia.edu/courses/ees/climate/lectures/atm_dyn.html
 * 
 * For day/night cycle (not implemented in hexgen2014: see Hero Extant proper).
 * http://en.wikipedia.org/wiki/Mountain_breeze_and_valley_breeze
 * http://en.wikipedia.org/wiki/Anabatic_wind
 *
 * Related to altitude & pressure: 
 * http://en.wikipedia.org/wiki/Katabatic_wind
 * http://en.wikipedia.org/wiki/Foehn_wind
 * http://en.wikipedia.org/wiki/Rain_shadow
 * http://en.wikipedia.org/wiki/Density_of_air
 * * http://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_atmospheric_pressure_variation
 * 
 * Global:
 * http://en.wikipedia.org/wiki/Intertropical_convergence_zone
 * http://en.wikipedia.org/wiki/Trade_wind
 * http://en.wikipedia.org/wiki/Prevailing_winds
 * http://en.wikipedia.org/wiki/Prevailing_winds#Effect_on_precipitation
 * http://en.wikipedia.org/wiki/Wind_speed
 *
 * Local:
 * http://en.wikipedia.org/wiki/Sea_breeze
 * http://en.wikipedia.org/wiki/Prevailing_winds#Circulation_in_elevated_regions
 * 
 * RAIN:
 * 
 * http://en.wikipedia.org/wiki/Rain#Causes
 * http://en.wikipedia.org/wiki/Dry_season
 * http://en.wikipedia.org/wiki/Tropical_rain_belt
 * 
 * MECHANICS:
 * http://www.st-andrews.ac.uk/~dib2/climate/pressure.html (p = R r T)
 * http://en.wikipedia.org/wiki/Primitive_equations
 * http://en.wikipedia.org/wiki/Barometric_formula
 * http://en.wikipedia.org/wiki/Bernoulli%27s_equation
 * http://en.wikipedia.org/wiki/Boyle's_law
 * 
 * BIOMES:
 * http://en.wikipedia.org/wiki/K%C3%B6ppen_climate_classification
 * http://en.wikipedia.org/wiki/Holdridge_life_zones
 * 
 */

/*
 * Let's first model the Tropical rain belt across the solar equator.
 * This mechanism is convection.
 * 
 * Convection knowledge:
 * Air is heated by solar radiation, especially along the solar equator.
 * Hot air is less dense than cold air. Hot air rises. Cold air falls.
 * Warmer air is able to retain more moisture than colder air.
 * Moist air is less dense than dry air.
 * When the warmer air is replaced by colder air, the colder air cannot absorb
 *     moisture. The excess moisture coalesces and then falls as rain due to
 *     gravity.
 * 
 * -- So due to tilt of the earth, we have a rain belt that moves to follow the
 *    seasonal solar equator.
 * 
 * Altitude knowledge:
 * Temperature falls with altitude.
 * Air pressure falls linearly with altitude.
 * Air density generally falls with altitude.
 * Rising air cools, cool air increases in density, and condenses causing rain.
 * Density of air is inversely proportional to temperature (cold = dense)
 * This is more pronounced when a mountain forces air to rise quickly.
 * 
 * High density air from a high elevation going down a slope causes
 * Katabatic winds due to acceleration due to gravity.
 * These can be cold and intense. But sinking air warms which means it can
 * hold more water.
 * 
 * -- So we can model the effects of mountains which give interesting wind
 *    speeds, temperatures, and rain shadows.
 * 
 * Simulating air will need require measure of air moisture, air density, air
 * temperature, air speed, and a 3D position.
 * 
 * We will need to model what happens when air collides - friction,
 * elastic collision, etc.
 * 
 * Individual particles do not have density -- only a mass of air as a whole.
 * If we have enough particles, density is implicit (i.e. density = particles
 * divided by volume).
 * 
 * Let's try a simple model: cells across a grid of size 128x128x8
 * 
 * Convection:
 * -- Each cell can be heated/cooled by the land beneath it.
 * -- Each cell can receive moisture from the land beneath it.
 * -- Hot air increases pressure, so pushes out & up, reducing density in the
 *        current cell and increasing density in adjacent and (mainly) above
 *        cells (direction can be implicit due to adjacent cells of equal
 *        pressure all wanting to push up to a low pressure area
 *        instead of adjacently).
 * -- Cold air is denser, so pushes into less dense cells, reducing temperature.
 *        This is due to gravity. Pressure caused by density can be compared
 *        to pressure caused by kinetic energy from gravitational acceleration.
 * -- Cold cells can hold less moisture, so rain sooner and rain colder.
 * -- Raining reduces moisture held in a cell.
 * 
 * The key point is WIND MOVEMENT IS CAUSED BY DIFFERENCES IN PRESSURE
 * 
 * And at the end we want to display:
 * -- Wind speed at the surface cells.
 * -- Air pressure.
 * -- Rainfall quantity & temperature.
 */




