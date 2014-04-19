#include "base.h"
#include "types.h"
#include "wgen/wgen.h"

int WorldRender_Elevation_Raw(World *w, Image *m)
{
    Doubles2D *e = &w->elevation;
    double *v = e->values;
    unsigned char *p = m->pixels;
    
    if (!Size2DEqual(e->size, m->size)) { X(interpolation_not_implemented); }
    
    for (size_t y = 0; y < e->size.y; y++)
    {
        for (size_t x = 0; x < e->size.x; x++)
        {
            unsigned char c = (unsigned char) (255.0 * (*v++));
            RGBA_WRITE1(p, c);
        }
    }
    
    return 1;
    
    err_interpolation_not_implemented:
        return 0;
}

int WorldRender_Elevation_Quick(World *w, Image *m)
{
    Doubles2D *e = &w->elevation;
    double *v = e->values;
    unsigned char *p = m->pixels;
    
    if (!Size2DEqual(e->size, m->size)) { X(interpolation_not_implemented); }
    
    for (size_t y = 0; y < e->size.y; y++)
    {
        for (size_t x = 0; x < e->size.x; x++)
        {
            double h = (*v++);
            
            if (h < SEA_LEVEL)
            {
                unsigned char r = (unsigned char) ( 32.0 + (32.0 * h));
                unsigned char g = (unsigned char) ( 64.0 + (255.0 * h));
                unsigned char b = (unsigned char) (128.0 + (128.0 * h));
                
                RGBA_WRITE3(p, r, g, b);
            }
            else
            {
                unsigned char r = (unsigned char) ( 64.0 + (32.0 * h));
                unsigned char g = (unsigned char) (128.0 + (64.0 * h));
                unsigned char b = 64;
                
                RGBA_WRITE3(p, r, g, b);
            }
        }
    }
    
    return 1;
    
    err_interpolation_not_implemented:
        return 0;
}


int WorldRender_Sunlight_Raw(World *w, Image *m)
{
    Doubles2D *e = &w->sunlight;
    double *v = e->values;
    unsigned char *p = m->pixels;
    
    if (!Size2DEqual(e->size, m->size)) { X(interpolation_not_implemented); }
    
    for (size_t y = 0; y < e->size.y; y++)
    {
        for (size_t x = 0; x < e->size.x; x++)
        {
            unsigned char c = (unsigned char) (255.0 * (*v++));
            RGBA_WRITE1(p, c);
        }
    }
    
    return 1;
    
    err_interpolation_not_implemented:
        return 0;
}


int WorldRender_Sunlight_Quick(World *w, Image *m)
{
    double *e = w->elevation.values;
    double *s = w->sunlight.values;
    unsigned char *p = m->pixels;
    
    if (!Size2DEqual( w->elevation.size, m->size)) { X(interpolation_not_implemented); }
    
    for (size_t y = 0; y <  w->elevation.size.y; y++)
    {
        for (size_t x = 0; x <  w->elevation.size.x; x++)
        {
            double h = (*e++);
            double i = (*s++);
            
            if (h < SEA_LEVEL)
            {
                RGBA_WRITE3(p, 32.0, 64.0, 128.0);
            }
            else
            {
                unsigned char r = (unsigned char) (255.0 * i);
                unsigned char g = (unsigned char) (128.0 * i);
                unsigned char b = (unsigned char) (64.0 * i);
                
                RGBA_WRITE3(p, r, g, b);
            }
        }
    }
    
    return 1;
    
    err_interpolation_not_implemented:
        return 0;
}




/*
int WindsimRender_Gravity(Windsim *w, Image *m)
{
    unsigned int square = (unsigned int) (m->size.y / w->size.z);
    unsigned char *p = m->pixels;

    for (size_t z = 0; z < w->size.z; z++)
    {
        size_t index = w->size.z - 1 - z; // grows upwards
        Windcell *cell = &w->cell[index];
        
        //unsigned char r = (unsigned char) (255.0 * cell->height / 100000.0);
        unsigned char g = (unsigned char) (255.0 * (cell->gravity - 9.0));
        unsigned char r = g;
        unsigned char b = g;
        
        /printf("z %d: height %.2f m, gravity %.2f m/s^2\n",
            (int) index, cell->height, cell->gravity);/
        
        for (size_t i = 0; i < m->size.x * square; i++)
        {
            RGBA_WRITE3(p, r, g, b);
        }
        
    }
    
    return 1;
}
*/


/*
int WindsimRender_Test(Windsim *w, Image *m)
{
    Doubles2D *e = &w->elevation;
    double *v = e->values;
    unsigned char *p = m->pixels;
    
    if (!Size2DEqual(e->size, m->size)) { X(interpolation_not_implemented); }
    
    for (size_t y = 0; y < e->size.y; y++)
    {
        for (size_t x = 0; x < e->size.x; x++)
        {
            unsigned char c = (unsigned char) (255.0 * (*v++));
            RGBA_WRITE1(p, c);
        }
    }
    
    return 1;
    
    err_interpolation_not_implemented:
        return 0;
}


#include <math.h>
static double Windsim_MaxDensity(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double max = 0.0;
    
    //for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double density = (c++)->density;
                if (density > max) { max = density; }
            }
        }
    //}
    
    return max;
}


static double Windsim_MinPressure(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double min = 1500000000.0;
    
    //for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double pressure = (c++)->pressure;
                if (pressure < min) { min = pressure; }
            }
        }
    //}
    
    return min;
}


static double Windsim_MaxPressure(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double max = 0.0;
    
//    for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double pressure = (c++)->pressure;
                if (pressure > max) { max = pressure; }
            }
        }
//    }
    
    return max;
}



static double Windsim_MaxForce(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double max = 0.0;
    
//    for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double force = fabs((c++)->force);
                if (force > max) { max = force; }
            }
        }
    //}
    
    return max;
}


static double Windsim_MinForce(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double min = 1000000000000.0;
    
    //for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double force = fabs((c++)->force);
                if (force < min) { min = force; }
            }
        }
    //}
    
    return min;
}


double Windsim_MaxTemp(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double max = -1000.0;
    
    //for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double temp = (c++)->temperature;
                if (temp > max) { max = temp; }
            }
        }
    //}
    
    return max;
}


double Windsim_MinTemp(Windsim *w, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    double min = 100000.0;
    
    //for (size_t z = 0; z < w->size.z; z++)
    //{
        for (size_t y = 0; y < w->size.y; y++)
        {
            for (size_t x = 0; x < w->size.x; x++)
            {
                double temp = (c++)->temperature;
                if (temp < min) { min = temp; }
            }
        }
    //}
    
    return min;
}


int WindsimRender_Density(Windsim *w, Image *m, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    unsigned char *p = m->pixels;
    
    double max = Windsim_MaxDensity(w, layer);
    
    for (size_t y = 0; y < w->size.y; y++)
    {
        for (size_t x = 0; x < w->size.x; x++)
        {
            double v = c->density;
            
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;
            
            if (v > max) { b = 255; r = 255; g = 255; }
            else if (v < 0) { b = 255; r = 0; g = 128; }
            else
            {
                r = (unsigned char) (255.0 * v / max);
                b = (unsigned char) (255.0 * (1.0 - (v / max)));
            }
            
            RGBA_WRITE3(p, r, g, b);
            c++;
        }
    }
    
    return 1;
}



int WindsimRender_Pressure(Windsim *w, Image *m, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    unsigned char *p = m->pixels;
    
    double min = Windsim_MinPressure(w, layer);
    double max = Windsim_MaxPressure(w, layer);
    //printf("min/max pressure: %f %f\n", min, max);
    
    for (size_t y = 0; y < w->size.y; y++)
    {
        for (size_t x = 0; x < w->size.x; x++)
        {
            double v = (c->pressure - min) / (max - min);
            //double v = (c->pressure - 90000.0) / 10000.0;
            unsigned char r, g, b;
            
            if (v > 1.0)
            {
                r = 255;
                g = 128;
                b = 128;
            }
            else if (v < 0.0)
            {
                b = 255.0;
                g = 128;
                r = 128;
            }
            else
            {
                r = (unsigned char) (255.0 * v);
                g = 0;
                b = 0;
            }
            RGBA_WRITE3(p, r, g, b);
            c++;
        }
    }
    
    return 1;
}

#include <math.h> // fabs
#include <stdio.h>
int WindsimRender_Force(Windsim *w, Image *m, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    unsigned char *p = m->pixels;
    
    double max = 250000000.0;
    double min = Windsim_MinForce(w, layer);
    max = Windsim_MaxForce(w, w->size.z - 1 - layer);
    //printf("MIN %f MAX %f\n", min, max);
    
    for (size_t y = 0; y < w->size.y; y++)
    {
        for (size_t x = 0; x < w->size.x; x++)
        {
            //double v = fabs(c->force) / max;
            double v = //(c->force - min) / (max - min);
            fabs(c->force) / max;
            
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;
            
            if (v >= 1.0) { b = 255; r = 255; g = 255; }
            else if (v < 0) { b = 255; r = 0; g = 0; }
            else
            {
                r = (unsigned char) (255.0 * v); // v/ max
                //b = (unsigned char) (255.0 * (1.0 - (v))); // v/max
            }
            
            RGBA_WRITE3(p, r, g, b);
            c++;
        }
    }
    
    return 1;
}



int WindsimRender_Temperature(Windsim *w, Image *m, size_t layer)
{
    Windcell *c = &w->cell[w->size.x * w->size.y * (w->size.z - 1 - layer)];
    unsigned char *p = m->pixels;
    
    double min = 270;
    double max = 500;
    
    for (size_t y = 0; y < w->size.y; y++)
    {
        for (size_t x = 0; x < w->size.x; x++)
        {
            double v = (c->temperature - min) / (max - min);
            
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;
            
            if (v >= 1.0) { b = 255; r = 255; g = 255; }
            else if (v < 0) { b = 0; r = 0; g = 0; }
            else
            {
                r = (unsigned char) (255.0 * v); // v/max
                b = (unsigned char) (255.0 * (1.0 - (v))); // v/max
            }
            
            RGBA_WRITE3(p, r, g, b);
            c++;
        }
    }
    
    return 1;
}
*/
