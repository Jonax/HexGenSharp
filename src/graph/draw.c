/*
 
 src/graph/draw.c - graph drawing function prototypes
 
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
 
*/

#include "base.h"
#include "graph/graph.h"
#include "graph/draw.h"
#include "graph/pen.h"
#include <math.h>

#define ROUGH_EPSILON 0.001


void GraphPlotPixel(Image *buffer, Pen *pen, vector2Df xy)
{
    int x = (int) xy.x;
    int y = (int) xy.y;
    int w = (int) buffer->size.x;
    int h = (int) buffer->size.y;
    
    // TODO subpixel sampling
    
    if (x < 0)  { return; }
    if (y < 0)  { return; }
    if (x >= w) { return; }
    if (y >= h) { return; }
    
    size_t offset = (size_t) ((y * w) + x); offset *= 4; // RGBA
    unsigned char *p = buffer->pixels + offset;
    
    RGBA_WRITE4(p, pen->red, pen->green, pen->blue, pen->alpha);
}


unsigned char GraphGlyphAlpha(Image *g, size_t x, size_t y, size_t c)
{
    size_t glyphx = (c % 16);
    size_t glyphy = (c / 16);
    size_t offsetx = (glyphx * 14) + x;
    size_t offsety = (glyphy * 17) + y;
    offsety *= g->size.x;
    offsetx *= 4;
    offsety *= 4;
    
    unsigned char *p = g->pixels + offsetx + offsety;
    return (*p);
}


vector2Df GraphWriteChar
(
    Image *buffer,
    Image *glyphs,
    Pen *pen,
    vector2Df pos,
    char c
)
{
    for (size_t y = 0; y < 16; y++)
    {
        for (size_t x = 0; x < 13; x++)
        {
            unsigned char alpha = GraphGlyphAlpha(glyphs, x, y, (size_t) c);
            
            if (alpha)
            {
                GraphPlotPixel(buffer, pen,
                    Vector2Df(pos.x + (double) x, pos.y + (double) y));
            }
        }
    }
    
    pos.x += 14;
    return pos;
}


vector2Df GraphWrite
(
    Grapher *g,
    Image *buffer,
    int glyphset,
    Pen *pen,
    vector2Df pos,
    const char *s
)
{
    Image *glyphs = GrapherGlyphset(g, glyphset);
    if (!glyphs) { X2(GrapherGlyphset, "invalid glyphset"); }
        
    while (*s != '\0')
    {
        pos = GraphWriteChar(buffer, glyphs, pen, pos, *s);
        s++;
    }
    
    return pos;
    
    err_GrapherGlyphset:
        return pos;
}


vector2Df GraphWriteSlanted
(
    Grapher *g,
    Image *buffer,
    int glyphset,
    Pen *pen,
    vector2Df pos,
    vector2Df slant,
    const char *s
)
{
    Image *glyphs = GrapherGlyphset(g, glyphset);
    if (!glyphs) { X2(GrapherGlyphset, "invalid glyphset"); }
    
    while (*s != '\0')
    {
        pos = GraphWriteChar(buffer, glyphs, pen, pos, *s);
        pos.x += slant.x;
        pos.y += slant.y;
        s++;
    }
    
    return pos;
    
    err_GrapherGlyphset:
        return pos;
}


void GraphPlot(Image *buffer, Pen *pen, vector2Df xy)
{
    for (double y = -pen->radius; y < pen->radius + ROUGH_EPSILON; y+= 1.0)
    {
        for (double x = -pen->radius; x < pen->radius + ROUGH_EPSILON; x+= 1.0)
        {
            GraphPlotPixel(buffer, pen, Vector2Df(x + xy.x, y + xy.y));
        }
    }
}


// http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void GraphDrawLine(Image *buffer, Pen *pen, vector2Df from, vector2Df to)
{
    int x0 = (int) from.x;
    int y0 = (int) from.y;
    int x1 = (int) to.x;
    int y1 = (int) to.y;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = -1;
    int sy = -1;
    if (x0 < x1) { sx = 1; }
    if (y0 < y1) { sy = 1; }
    int err = dx - dy;
    
    while (1)
    {
        GraphPlot(buffer, pen, Vector2Df((double) x0, (double) y0));
        
        if ((x0 == x1) && (y0 == y1)) { break; }
        
        int e2 = 2 * err;
        
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


void GraphDrawCircle(Image *buffer, Pen *pen, vector2Df center, double radius)
{
    // x^2 + y^2 = r^2
    
    // x = 0, y = sqrt(radius^2) => y = +/- radius
    // x = n, y = sqrt(radius^2 - n^2)
    
    for (double x = -radius; x < radius + ROUGH_EPSILON; x += 0.1)
    {
        double y = sqrt(square(radius) - square(x));
        
        GraphPlot(buffer, pen, Vector2Df(x + center.x, y + center.y));
        GraphPlot(buffer, pen, Vector2Df(x + center.x, -y + center.y));
    }
}
