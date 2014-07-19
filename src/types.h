/*
 
 types.h - basic multidimensional types
 
 ------------------------------------------------------------------------------
 
 2013 - 2014 Ben Golightly <golightly.ben@googlemail.com>
 
 This file has is placed in the public domain. All copyright is disclaimed by
 the author.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 ------------------------------------------------------------------------------
 
*/

#ifndef BSE_TYPES_H
#   define BSE_TYPES_H

#include <stdlib.h> // size_t
#include <math.h>

#ifndef HEADER_FUNC
#   define BSE_TYPES_H_DEFINED_HEADER_FUNC
#   ifdef __GNUC__
#       define HEADER_FUNC static __attribute__((always_inline)) __inline__
#   else
#       warning Using default inline keyword (expected GNUC)
#       define HEADER_FUNC static __inline__
#   endif
#endif


/* === types === */
typedef signed int        vector1D;
typedef struct vector2D   vector2D;
typedef struct vector3D   vector3D;
typedef struct vector4D   vector4D;

typedef double            vector1Df;
typedef struct vector2Df  vector2Df;
typedef struct vector3Df  vector3Df;
typedef struct vector4Df  vector4Df;

typedef unsigned int      scalar1D;
typedef struct scalar2D   scalar2D;
typedef struct scalar3D   scalar3D;
typedef struct scalar4D   scalar4D;

typedef size_t            size1D;
typedef struct size2D     size2D;
typedef struct size3D     size3D;
typedef struct size4D     size4D;

typedef struct rect       rect;
typedef struct urect      urect;
typedef struct rectf      rectf;

typedef struct line2D     line2D;
typedef struct line3D     line3D;


/* === declarations === */
struct vector2D{int x, y;};
struct vector3D{int x, y, z;};
struct vector4D{int x, y, z, w;};

struct vector2Df{double x, y;};
struct vector3Df{double x, y, z;};
struct vector4Df{double x, y, z, w;};

struct scalar2D{unsigned int x, y;};
struct scalar3D{unsigned int x, y, z;};
struct scalar4D{unsigned int x, y, z, w;};

struct size2D{size_t x, y;};
struct size3D{size_t x, y, z;};
struct size4D{size_t x, y, z, w;};

struct  rect{int x0, y0, x1, y1;};
struct urect{unsigned int x0, y0, x1, y1;};
struct rectf{double x0, y0, x1, y1;};

struct line2D{int x0, y0, x1, y1;};
struct line3D{int x0, y0, z0, x1, y1, z1;};


/* === implementation: assignment === */

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

HEADER_FUNC vector4Df Vector4Df(double x, double y, double z, double w)
{
    vector4Df v;
    
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    
    return v;
}

HEADER_FUNC scalar2D Scalar2D(unsigned int x, unsigned int y)
{
    scalar2D s;
    
    s.x = x;
    s.y = y;
    
    return s;
}

HEADER_FUNC scalar3D Scalar3D(unsigned int x, unsigned int y, unsigned int z)
{
    scalar3D s;
    
    s.x = x;
    s.y = y;
    s.z = z;
    
    return s;
}

HEADER_FUNC scalar4D Scalar4D(unsigned int x, unsigned int y,
                              unsigned int z, unsigned int w)
{
    scalar4D s;
    
    s.x = x;
    s.y = y;
    s.z = z;
    s.w = w;
    
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

HEADER_FUNC size4D Size4D(size_t x, size_t y, size_t z, size_t w)
{
    size4D s;
    
    s.x = x;
    s.y = y;
    s.z = z;
    s.w = w;
    
    return s;
}

HEADER_FUNC rect RectXYXY(int x0, int y0, int x1, int y1)
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

HEADER_FUNC rect RectVV(vector2D start, vector2D end)
{
    rect r;
    
    r.x0 = start.x;
    r.y0 = start.y;
    r.x1 = end.x;
    r.y1 = end.y;
    
    return r;
}

HEADER_FUNC urect URectXYXY(unsigned int x0, unsigned int y0,
                            unsigned int x1, unsigned int y1)
{
    urect r;
    
    r.x0 = x0;
    r.y0 = y0;
    r.x1 = x1;
    r.y1 = y1;
    
    return r;
}

HEADER_FUNC urect URectXYWH(unsigned int x, unsigned int y,
                            unsigned int w, unsigned int h)
{
    urect r;
    
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + w;
    r.y1 = y + h;
    
    return r;
}

HEADER_FUNC urect URectVV(scalar2D start, scalar2D end)
{
    urect r;
    
    r.x0 = start.x;
    r.y0 = start.y;
    r.x1 = end.x;
    r.y1 = end.y;
    
    return r;
}

HEADER_FUNC rectf RectfXYXY(double x0, double y0, double x1, double y1)
{
    rectf r;
    
    r.x0 = x0;
    r.y0 = y0;
    r.x1 = x1;
    r.y1 = y1;
    
    return r;
}

HEADER_FUNC rectf RectfXYWH(double x, double y, double w, double h)
{
    rectf r;
    
    r.x0 = x;
    r.y0 = y;
    r.x1 = x + w;
    r.y1 = y + h;
    
    return r;
}

HEADER_FUNC rectf RectfVV(vector2Df start, vector2Df end)
{
    rectf r;
    
    r.x0 = start.x;
    r.y0 = start.y;
    r.x1 = end.x;
    r.y1 = end.y;
    
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

HEADER_FUNC line3D Line3D(int x0, int y0, int z0, int x1, int y1, int z1)
{
    line3D l;
    
    l.x0 = x0;
    l.y0 = y0;
    l.z0 = z0;
    l.x1 = x1;
    l.y1 = y1;
    l.z1 = z1;
    
    return l;
}


/* === implementation: auxiliary === */

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

HEADER_FUNC size_t size_t_max(size_t a, size_t b)
{
    if (a > b) { return a; } else { return b; }
}

HEADER_FUNC int size2D_equal(size2D a, size2D b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

HEADER_FUNC int size3D_equal(size3D a, size3D b)
{
    return ((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
}

HEADER_FUNC int vector2D_has_magnitude(vector2D v)
{
    return (v.x | v.y);
}

HEADER_FUNC int vector3D_has_magnitude(vector3D v)
{
    return (v.x | v.y | v.z);
}

HEADER_FUNC int vector4D_has_magnitude(vector4D v)
{
    return (v.x | v.y | v.z | v.w);
}

HEADER_FUNC vector2Df vector2Df_normalise(vector2Df v)
{
    double mag = sqrt(square(v.x) + square(v.y));
    v.x = v.x / mag;
    v.y = v.y / mag;
    return v;
}

HEADER_FUNC vector3Df vector3Df_normalise(vector3Df v)
{
    double mag = sqrt(square(v.x) + square(v.y) + square(v.z));
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    return v;
}

HEADER_FUNC vector4Df vector4Df_normalise(vector4Df v)
{
    double mag = sqrt(square(v.x) + square(v.y) + square(v.z) + square(v.w));
    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
    v.w /= mag;
    return v;
}

HEADER_FUNC vector2D line2D_start(line2D l)
{
    return Vector2D(l.x0, l.y0);
}

HEADER_FUNC vector2D line2D_end(line2D l)
{
    return Vector2D(l.x1, l.y1);
}

#ifdef BSE_TYPES_H_DEFINED_HEADER_FUNC
#   undef HEADER_FUNC
#   undef BSE_TYPES_H_DEFINED_HEADER_FUNC
#endif

#endif


