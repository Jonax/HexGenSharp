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
            
            if (h < 0.15)
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
            
            if (h < 0.05)
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
