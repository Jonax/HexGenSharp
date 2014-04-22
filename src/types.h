/*

 src/types.h - core typedefs
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2013 - 2014 Ben Golightly <golightly.ben@googlemail.com>

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



#ifndef HG14_TYPES_H
#   define HG14_TYPES_H

#include <stdlib.h> // size_t
#include "base.h" // HEADER_FUNC (static force inline)

#define PI 3.141592654

/*
 * Handy coordinates/vectors/scalars
 */

typedef signed int        vector;
typedef struct vector2D   vector2D;
typedef struct vector3D   vector3D;
typedef struct vector4D   vector4D;
typedef struct vector2Df  vector2Df;
typedef struct vector3Df  vector3Df;
typedef unsigned int      scalar;
typedef struct scalar2D   scalar2D;
typedef struct scalar3D   scalar3D;
typedef struct size2D     size2D;
typedef struct size3D     size3D;
typedef struct rect       rect;
typedef struct urect      urect;
typedef struct line2D     line2D;

struct vector2D
{
    int x, y;
};

struct vector3D
{
    int x, y, z;
};

struct vector4D
{
    int x, y, z, w;
};

struct vector2Df
{
    double x, y;
};

struct vector3Df
{
    double x, y, z;
};

struct scalar2D
{
    unsigned int x, y;
};

struct scalar3D
{
    unsigned int x, y, z;
};

struct size2D
{
    size_t x, y;
};

struct size3D
{
    size_t x, y, z;
};

struct rect
{
    int x0, y0, x1, y1;
};

struct urect
{
    unsigned int x0, y0, x1, y1;
};

struct line2D
{
    int x0, y0, x1, y1;
};




/* Inline implementations */
HEADER_FUNC vector2D Vector2D(int x, int y)
{
    vector2D v;
    
    v.x = x;
    v.y = y;
    
    return v;
}

HEADER_FUNC vector3D Vector3D(int x, int y, int z)
{
    vector3D v;
    
    v.x = x;
    v.y = y;
    v.z = z;
    
    return v;
}

HEADER_FUNC vector4D Vector4D(int x, int y, int z, int w)
{
    vector4D v;
    
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    
    return v;
}

HEADER_FUNC vector2Df Vector2Df(double x, double y)
{
    vector2Df v;
    
    v.x = x;
    v.y = y;
    
    return v;
}

HEADER_FUNC vector3Df Vector3Df(double x, double y, double z)
{
    vector3Df v;
    
    v.x = x;
    v.y = y;
    v.z = z;
    
    return v;
}

HEADER_FUNC scalar2D Scalar2D(unsigned int x, unsigned int y)
{
    scalar2D s;
    
    s.x = x;
    s.y = y;
    
    return s;
}

HEADER_FUNC unsigned int Scalar2D_y(scalar2D s)
{
    return s.y;
}

HEADER_FUNC scalar3D Scalar3D(unsigned int x, unsigned int y, unsigned int z)
{
    scalar3D s;
    
    s.x = x;
    s.y = y;
    s.z = z;
    
    return s;
}

HEADER_FUNC size2D Size2D(size_t x, size_t y)
{
    size2D z;
    
    z.x = x;
    z.y = y;
    
    return z;
}

HEADER_FUNC size3D Size3D(size_t x, size_t y, size_t z)
{
    size3D s;
    
    s.x = x;
    s.y = y;
    s.z = z;
    
    return s;
}

HEADER_FUNC rect Rect(int x0, int y0, int x1, int y1)
{
    rect r;
    
    r.x0 = x0;
    r.y0 = y0;
    r.x1 = x1;
    r.y1 = y1;
    
    return r;
}

HEADER_FUNC rect RectXYWH(int x, int y, int w, int h)
{
    rect r;
    
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + w;
    r.y1 = y + h;
    
    return r;
}

HEADER_FUNC urect URect(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
{
    urect r;
    
    r.x0 = x0;
    r.y0 = y0;
    r.x1 = x1;
    r.y1 = y1;
    
    return r;
}

HEADER_FUNC urect URectXYWH(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    urect r;
    
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + w;
    r.y1 = y + h;
    
    return r;
}

HEADER_FUNC line2D Line2D(int x0, int y0, int x1, int y1)
{
    line2D l;
    
    l.x0 = x0;
    l.y0 = y0;
    l.x1 = x1;
    l.y1 = y1;
    
    return l;
}


/* Auxiliary functions */
HEADER_FUNC double square(double a)
{
    return a * a;
}

HEADER_FUNC double radians(double deg)
{
    return (deg * 3.141592654 / 180.0);
}

HEADER_FUNC double degrees(double rad)
{
    return (rad * 180.0 / 3.141592654);
}

HEADER_FUNC size_t max_size_t(size_t a, size_t b)
{
    if (a > b) { return a; } else { return b; }
}

HEADER_FUNC int Size2DEqual(size2D a, size2D b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

HEADER_FUNC int Vector2D_HasMagnitude(vector2D v)
{
    return (v.x | v.y);
}

HEADER_FUNC int Vector3D_HasMagnitude(vector3D v)
{
    return (v.x | v.y | v.z);
}


#endif

