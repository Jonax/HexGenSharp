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
    double height = 2000000.0 * (zf / ceil); // m = 2km
    double re = 6371000; // earth radius, m
    double gravity = 9.81 * Square(re /(re + height));
    
    cell->height = height;
    cell->gravity = gravity;
}


int WindsimRun(Windsim *sim, Image *img, int it)
{
    //if (!WindsimSampleWorld(sim)) { X(WindsimSampleWorld); }
    WindsimCellsInit(sim, Cell(1.225, 273.15, 0.0));
    
    it = 1;
    printf("Wind simulation: %d iterations over %dx%dx%d\n",
        it, (int) sim->size.x, (int) sim->size.y, (int) sim->size.z);
    
    for (int i = 0; i < it; i++)
    {
        if (i % 10 == 0) { printf("Windsim: %d/%d\n", i, it); }
        
        for (size_t z = 0; z < sim->size.z; z++)
        {
            WindsimStepCell(sim, z);
        }
        
        if
        (
            (i < 50) ||
            ((i < 100) && (i % 5 == 0)) ||
            ((i >= 100) && (i % 10 == 0))
        )
        {
        char filename0[256];char filename1[256];char filename2[256];char filename3[256];
        sprintf(filename0, ".windsim/gravity/gravity-%d.png", i);
        sprintf(filename1, ".windsim/pressure/windsim-pressure-%d.png", i);
        sprintf(filename2, ".windsim/force/windsim-force-%d.png", i);
        sprintf(filename3, ".windsim/density/windsim-density-%d.png", i);
        
        WindsimRender_Gravity(sim, img);
        ImageSaveTo(img, filename0);
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
