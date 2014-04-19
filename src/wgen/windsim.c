#include "wgen/wgen.h"

int Windsim1D(Windsim *sim, World *w, size3D size)
{
    sim->world = w;
    sim->size = size;
    sim->elements = size.z; //size.x * size.y * size.z;
    sim->cell = NULL;
    
    // cells from 0 to z-1, where 0 is the bottom
    sim->cell = malloc(sizeof(Windcell) * sim->elements);
    if (!sim->cell) { X(alloc_sim_cells); }
    
    return 1;
    
    err_alloc_sim_cells:
        return 0;
}


static FORCE_INLINE Windcell Cell
(
    double density,     // kg/m3
    double temperature, // K
    double moisture
)
{
    Windcell c;
    c.density = density;
    c.temperature = temperature;
    c.moisture = moisture;
    c.pressure = 0;
    c.force = 0;
    c.gravity = 0;
    c.height = 0;
    return c;
}

void WindcellPressurise(Windcell *cell)
{
    // http://www.st-andrews.ac.uk/~dib2/climate/pressure.html
    cell->pressure = (cell->density * cell->temperature * 287.0);
    // 287.0 should change based on moisture content
}
// pressure ~= (0.5 x density) x speed^2
// speed = sqrt(2 * pressure / density)


void WindsimCellsInit(Windsim *sim, Windcell template)
{
    for (size_t i = 0; i < sim->elements; i++)
    {
        sim->cell[i] = template;
        WindcellPressurise(&sim->cell[i]);
    }
}




double maxf(double a, double b) { if (a > b) { return a; } else { return b; } }


#include <math.h>
#include <stdio.h>


FORCE_INLINE double Square(double a)
{ return a * a; }

void WindsimStepCell(Windsim *sim, size_t z)
{
    Windcell *cell = &sim->cell[z];
    
    double zf = (double) z; zf += 0.5;
    double ceil = (double)(sim->size.z - 1); ceil += 0.5;
    double height = 100000.0 * (zf / ceil); // m = 100km
    double re = 6371000; // earth radius, m
    double gravity = 9.81 * Square(re /(re + height));
    
    cell->height = height;
    cell->gravity = gravity;
    
    // mass = volume * density
    // force = mass * gravity
    
    // 1km by 1km by (100km / zcells ~= 100/16 km)
    double mass = cell->density * (100000.0 / ceil) * 1000.0 * 1000.0;
    
    double force = mass * cell->gravity;
    
    WindcellPressurise(cell);
    
    double downforce = cell->pressure * (1000.0 * 1000.0) + force;
    double upforce = cell->pressure * (1000.0 * 1000.0);
    
    /*
    printf("z%02d: h %.2f m, g %.2f m/s^2, density %.2f KG/m^3, mass %.2f kg, force of g %.2f N\n",
        (int) z, height, gravity, cell->density, mass, force);
    printf("     pressure: %.2f N/m^2 => %2f N vertical => %.2f N up, %.2f N down\n\n", cell->pressure,
        cell->pressure * (1000.0 * 1000.0), upforce, downforce);
    */
    
    printf("cell z%0d: h %.2f m, density %.2f KG/M^3, pressure %.2f\n",
        (int) z, height, cell->density, cell->pressure);
    
    cell->upforce = upforce;
    cell->downforce = downforce;
    
    // re radius
    double a = 1000.0; // 1km
    double b = re;
    double c = re;
    double theta; // angle between points 1km apart, radians
#   define SQ(a) ((a) * (a))
    theta =
    acos(
        (SQ(a) - SQ(b) - SQ(c)) /
        (-2 * b * c)
    );
    printf("theta: %f\n", theta);
    
    // distance between two points at the same angle, 100km above
    double h = 100.0 * 1000.0;
    b += h;
    c += h;
    a = sqrt(SQ(b) + SQ(c) - (2.0 * b * c * cos(theta))); // never actually need acos
    
    printf("100km above, %fm apart\n", a);
    
    // cell->torque = ||r|| ||F||;
    // moment = force × distance
    // at surface, moment = radius
    // m_a + m_b = m_c => I_a + I_b = I_c
    
    // http://eesc.columbia.edu/courses/ees/climate/lectures/atm_dyn.html
}


void WindsimResolveCell(Windsim *sim, size_t z)
{
    Windcell *above = &sim->cell[z+1];
    Windcell *below = &sim->cell[z];
    
    double diff = (above->downforce - below->upforce);
    // > 0.0 means above pushes down
    
# define STEP_SIZE 0.05
    
    if (above->density < STEP_SIZE) { return; }
    if (below->density < STEP_SIZE) { return; }
    
    if (diff > 0.0) { above->density -= STEP_SIZE; below->density += STEP_SIZE; }
    else { above->density += STEP_SIZE; below->density -= STEP_SIZE; }
}


#include "graph/graph.h"
int WindsimRun(Windsim *sim, Image *img, Image *graph, int it)
{
    //if (!WindsimSampleWorld(sim)) { X(WindsimSampleWorld); }
    WindsimCellsInit(sim, Cell(1.225, 273.15, 0.0));
    
    it = 401;
    //it = 1;
    printf("Wind simulation: %d iterations over %dx%dx%d\n",
        it, (int) sim->size.x, (int) sim->size.y, (int) sim->size.z);
    
    for (int i = 0; i < it; i++)
    {
        if (i % 10 == 0) { printf("Windsim: %d/%d\n", i, it); }
        
        printf("\n\n=== PRESSURISE CELLS === \n");
        for (size_t z = 0; z < sim->size.z; z++)
        {
            WindsimStepCell(sim, z);
        }
        
        for (size_t z = 0; z < sim->size.z - 1; z++)
        {
            WindsimResolveCell(sim, z);
        }
        
        if
        (
            (i < 50) ||
            ((i < 100) && (i % 5 == 0)) ||
            ((i >= 100) && (i % 10 == 0))
        )
        {
        char filename0[256];char filename1[256];char filename2[256];char filename3[256];
        char filegraph[256];
        sprintf(filename0, ".windsim/gravity/gravity-%d.png", i);
        sprintf(filename1, ".windsim/pressure/windsim-pressure-%d.png", i);
        sprintf(filename2, ".windsim/force/windsim-force-%d.png", i);
        sprintf(filename3, ".windsim/density/windsim-density-%d.png", i);
        sprintf(filegraph, ".windsim/graph/graph-%d.png", i);
        
        WindsimRender_Gravity(sim, img);
        ImageSaveTo(img, filename0);
        
        if ((i % 50) == 0)
        {
            char title[256];
            sprintf(title, "iteration %d/%d", i, it - 1);
        
            GraphAtmosphere1D(title, graph, 16, Vector3Df(1000, 1000, 100 * 1000));
            ImageSaveTo(graph, filegraph);
        }
        
        //WindsimRender_Pressure(sim, img);
        //ImageSaveTo(img, filename1);
        //WindsimRender_Force(sim, img);
        //ImageSaveTo(img, filename2);
        //WindsimRender_Density(sim, img);
        //ImageSaveTo(img, filename3);
        }
    }
    
    return 1;
    
    err_WindsimSampleWorld:
        return 0;
}
