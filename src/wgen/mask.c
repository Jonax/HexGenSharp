#include "wgen/wgen.h"
#include <math.h> // sqrt

double SampleDefault
(
    void *p,
    double x,
    double y,
    double w,
    double h
)
{
    UNUSED(p);
    UNUSED(x);
    UNUSED(y);
    UNUSED(w);
    UNUSED(h);
    
    return 1.0;
}


double SampleCircleGradiant
(
    void *p,
    double x,
    double y,
    double w,
    double h
)
{
    UNUSED(p);
    
    /* Distance from center */
    double dx = (w*0.5) - x;
    double dy = (h*0.5) - y;
    
    /* Max radius for spherical falloff */
    double maxr = w * 0.5;
    
    /* Distance of point from center (basic pythag) */
    double r = sqrt((dx*dx) + (dy*dy));
    
    if (r > maxr) { return 0.0; }
    double scale = 1.0 - (r / maxr);
    
    return scale; // 0.0 to 1.0, where 1.0 is opaque
}


double SampleCircleInverseGradiant
(
    void *p,
    double x,
    double y,
    double w,
    double h
)
{
    UNUSED(p);
    
    /* Distance from center */
    double dx = (w*0.5) - x;
    double dy = (h*0.5) - y;
    
    /* Max radius for spherical falloff */
    double maxr = w * 0.5;
    
    /* Distance of point from center (basic pythag) */
    double r = sqrt((dx*dx) + (dy*dy));
    
    if (r > maxr) { return 1.0; }
    double scale = (r / maxr);
    
    return scale; // 0.0 to 1.0, where 1.0 is opaque
}
