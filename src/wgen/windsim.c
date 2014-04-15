#include "wgen/wgen.h"

int WindsimInit(Windsim *sim, World *w, size3D size)
{
    sim->world = w;
    sim->size = size;
    sim->elements = size.x * size.y * size.z;
    sim->cell = NULL;
    
    if (!Doubles2DInit(&sim->elevation, Size2D(size.x, size.y)))
        { X(Doubles2DInit_elevation); }
    
    if (!Doubles2DInit(&sim->sunlight, Size2D(size.x, size.y)))
        { X(Doubles2DInit_sunlight); }
    
    sim->cell = malloc(sizeof(Windcell) * sim->elements);
    if (!sim->cell) { X(alloc_sim_cells); }
    
    return 1;
    
    err_alloc_sim_cells:
        Doubles2DTeardown(&sim->sunlight);
    err_Doubles2DInit_sunlight:
        Doubles2DTeardown(&sim->elevation);
    err_Doubles2DInit_elevation:
        return 0;
}


int WindsimSampleWorld(Windsim *sim)
{
    World *w = sim->world;
    
    if (!Doubles2DDownsample(&sim->elevation, &w->elevation))
        { X(Doubles2DDownsample_elevation); }
    
    if (!Doubles2DDownsample(&sim->sunlight,  &w->sunlight))
        { X(Doubles2DDownsample_sunlight); }
    
    return 1;
    
    err_Doubles2DDownsample_elevation:
    err_Doubles2DDownsample_sunlight:
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
void WindsimStepCell(Windsim *sim, size_t i3D, size_t i2D, int x, int y, int z)
{
    Windcell *cell = &sim->cell[i3D];
    
    UNUSED(x);
    UNUSED(y);
    UNUSED(z);
    
    if (z == (int) (sim->size.z - 1)) // TODO change to elevation bracket
    {
        // lets for now convert direct radiance into kelvin
        // and multiply it by 0.6 for water's heat capacity.
        double elevation = sim->elevation.values[i2D];
        double kelvin = sim->sunlight.values[i2D] * sim->sunlight.maximum;
        kelvin = sqrt(kelvin);
        
        if (elevation < SEA_LEVEL) { kelvin *= 0.6; }
        kelvin += 270;
        
        // for thermal conductivity see http://www.engineeringtoolbox.com/air-properties-d_156.html
        // http://en.wikipedia.org/wiki/Fourier%27s_law#Fourier.27s_law
        // 0.0243;
        
        // do it simply for now. This is super inaccurate
        if (kelvin > cell->temperature)
        {
            cell->temperature += 1.0;
        }
        else
        {
            cell->temperature -= 1.0;
        }
        // TODO should cool the land beneath as well.
        WindcellPressurise(cell);
    }
    
    // Pressure gradient force
    // http://en.wikipedia.org/wiki/Primitive_equations#Forces_that_cause_atmospheric_motion
    // http://san.hufs.ac.kr/~gwlee/session4/pgf.html
    
    // assuming 2km per pixel for now
    // Pressure Gradient Force (PGF) = (-1/density) * (delta pressure / delta distance)
    
    // Pressure force
    cell->force = 0.0;
    //int zw = 0;
    for (int zw = -1; zw <= +1; zw++) {
    for (int yw = -1; yw <= +1; yw++) {
    for (int xw = -1; xw <= +1; xw++) {
        int x2 = x; int y2 = y; int z2 = z;
        
        if ((xw == -1) && (x == 0)) { x2 = (int) sim->size.x; }
        if ((yw == -1) && (y == 0)) { y2 = (int) sim->size.y; }
        if ((zw == -1) && (z == 0)) { continue; }
        if ((xw == +1) && (x == (int) sim->size.x - 1)) { x2 = -1; }
        if ((yw == +1) && (y == (int) sim->size.y - 1)) { y2 = -1; }
        if ((zw == +1) && (z == (int) sim->size.z - 1)) { continue; }
        if ((xw == 0) && (yw == 0) && (zw ==0)) { continue; }
        
        Windcell *neighbour = &sim->cell
            [((zw + z2) * (int) sim->size.x * (int) sim->size.y) +
             ((yw + y2) * (int) sim->size.x) +
             (xw + x2)];
        
        double force =  (-1/cell->density) * (cell->pressure - neighbour->pressure);
        
        // By Boyle's law P_1 x V_1 = P_2 x V_2
        // density = mass / volume
        // so P_1 x mass_1 / V_1 = P_2 x mass_2 / V_2
        // we know P_1, P_2
        // if P_1 > P_2, mass_1 down, mass_2 up
        // if P_1 < P_2, mass_1 up, mass_2 down
        // P_1 / P_2 = mass_2 / mass_1
        // http://en.wikipedia.org/wiki/Work_(thermodynamics)#Pressure-volume_work
        
        // but for now
        if (cell->density >= 0.01)
        {
            double sign = 1;
            if (force < 0.0) { sign = -1; }
            //cell->density += 0.1 * sign;
            //neighbour->density += 0.1 * -sign;
            
            /*
            double temp_diff = cell->temperature - neighbour->temperature;
            sign = 1;
            if (temp_diff < 0.0) { sign = -1; }
            cell->temperature += 0.1 * sign;
            neighbour->temperature -= 0.1 * sign;
            */
            
            WindcellPressurise(cell);
            WindcellPressurise(neighbour);
        }
        // else { neighbour->density -= 0.05; cell->density += 0.05; }
        
        cell->force += maxf(cell->force, fabs(force));
    }}}
    
    //cell->force = fabs(cell->force);
}

#include <stdio.h>
int WindsimRun(Windsim *sim, Image *img)
{
    if (!WindsimSampleWorld(sim)) { X(WindsimSampleWorld); }
    WindsimCellsInit(sim, Cell(1.225, 273.15, 0.0));
    
    printf("Wind simulation: 500 iterations over %dx%dx%d\n",
        (int) sim->size.x, (int) sim->size.y, (int) sim->size.z);
    
    for (int i = 0; i < 500; i++)
    {
        if (i % 10 == 0) { printf("Windsim: %d/300\n", i); }
        
        size_t index3D = 0;
        
        for (int z = 0; z < (int) sim->size.z; z++)
        {
            size_t index2D = 0;
            
            for (int y = 0; y < (int) sim->size.y; y++)
            {
                for (int x = 0; x < (int) sim->size.x; x++)
                {
                    WindsimStepCell(sim, index3D, index2D, x, y, z);
                    index2D++;
                    index3D++;
                }
            }
        }
        
        if
        (
            (i < 50) ||
            ((i < 100) && (i % 5 == 0)) ||
            ((i >= 100) && (i % 10 == 0))
        )
        {
        char filename1[256];char filename2[256];char filename3[256];
        char filename4[256];char filename5[256];char filename6[256];
        char filename7[256];char filename8[256];
        sprintf(filename1, ".windsim/pressure/windsim-pressure-%d.png", i);
        sprintf(filename2, ".windsim/force/windsim-force-%d.png", i);
        sprintf(filename3, ".windsim/density/windsim-density-%d.png", i);
        sprintf(filename4, ".windsim2/pressure/windsim-pressure-%d.png", i);
        sprintf(filename5, ".windsim2/force/windsim-force-%d.png", i);
        sprintf(filename6, ".windsim2/density/windsim-density-%d.png", i);
        sprintf(filename7, ".windsim/temperature/windsim-temperature-%d.png", i);
        sprintf(filename8, ".windsim2/temperature/windsim-temperature-%d.png", i);
        
        WindsimRender_Pressure(sim, img, 0);
        ImageSaveTo(img, filename1);
        WindsimRender_Pressure(sim, img, 1);
        ImageSaveTo(img, filename4);
        
        WindsimRender_Force(sim, img, 0);
        ImageSaveTo(img, filename2);
        WindsimRender_Force(sim, img, 1);
        ImageSaveTo(img, filename5);
        
        WindsimRender_Density(sim, img, 0);
        ImageSaveTo(img, filename3);
        WindsimRender_Density(sim, img, 1);
        ImageSaveTo(img, filename6);
        
        WindsimRender_Temperature(sim, img, 0);
        ImageSaveTo(img, filename7);
        WindsimRender_Temperature(sim, img, 1);
        ImageSaveTo(img, filename8);
        }
    }
    
    return 1;
    
    err_WindsimSampleWorld:
        return 0;
}
