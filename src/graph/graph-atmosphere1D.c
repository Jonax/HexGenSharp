/*
 
 src/graph/graph-atmosphere1D.c - draw a stack of atmosphere cells
 
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

#include <math.h>
#include <stdlib.h> // abs
#include <stdio.h>
#include "graph/graph.h"
#include "graph/pen.h"
#include "graph/baked.h"
# define SQ(x) ((x) * (x))
# define EPSILON 0.001

void PlotPixel(Image *buffer, Pen *pen, vector2Df xy)
{
    int x = (int) xy.x;
    int y = (int) xy.y;
    int w = (int) buffer->size.x;
    int h = (int) buffer->size.y;
    
    // TODO antialias
    
    if (x < 0) { return; }
    if (y < 0) { return; }
    if (x >= w) { return; }
    if (y >= h) { return; }
    
    size_t offset = (size_t) ((y * w) + x); offset *= 4; // RGBA
    unsigned char *p = buffer->pixels + offset;
    
    RGBA_WRITE4(p, pen->red, pen->green, pen->blue, pen->alpha);
}


unsigned char GlyphAlpha(Image *g, size_t x, size_t y, size_t c)
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


vector2Df WriteChar(Image *buffer, Image *glyphs, Pen *pen, vector2Df pos, char c)
{
    for (size_t y = 0; y < 16; y++)
    {
        for (size_t x = 0; x < 13; x++)
        {
            unsigned char alpha = GlyphAlpha(glyphs, x, y, (size_t) c);
            
            if (alpha)
            {
                PlotPixel(buffer, pen,
                    Vector2Df(pos.x + (double) x, pos.y + (double) y));
            }
        }
    }
    
    pos.x += 14;
    return pos;
}

vector2Df Write(Image *buffer, Image *glyphs, Pen *pen, vector2Df pos, const char *s)
{
    while (*s != '\0')
    {
        pos = WriteChar(buffer, glyphs, pen, pos, *s);
        s++;
    }
    
    return pos;
}


vector2Df WriteSlant(Image *buffer, Image *glyphs, Pen *pen, vector2Df pos, vector2Df slant, const char *s)
{
    while (*s != '\0')
    {
        pos = WriteChar(buffer, glyphs, pen, pos, *s);
        pos.x += slant.x;
        pos.y += slant.y;
        s++;
    }
    
    return pos;
}


void Plot(Image *buffer, Pen *pen, vector2Df xy)
{
    for (double y = -pen->radius; y < pen->radius + EPSILON; y+= 1.0)
    {
        for (double x = -pen->radius; x < pen->radius + EPSILON; x+= 1.0)
        {
            PlotPixel(buffer, pen, Vector2Df(x + xy.x, y + xy.y));
        }
    }
}


/*
function line(x0, y0, x1, y1)
  dx := abs(x1-x0)
  dy := abs(y1-y0) 
  if x0 < x1 then sx := 1 else sx := -1
  if y0 < y1 then sy := 1 else sy := -1
  err := dx-dy

  loop
    plot(x0,y0)
    if x0 = x1 and y0 = y1 exit loop
    e2 := 2*err
    if e2 > -dy then 
      err := err - dy
      x0 := x0 + sx
    end if
    if e2 < dx then 
      err := err + dx
      y0 := y0 + sy 
    end if
  end loop
  */

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
        Plot(buffer, pen, Vector2Df((double) x0, (double) y0));
        
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
    
    for (double x = -radius; x < radius + EPSILON; x += 0.1)
    {
        double y = sqrt(SQ(radius) - SQ(x));
        
        Plot(buffer, pen, Vector2Df(x + center.x, y + center.y));
        Plot(buffer, pen, Vector2Df(x + center.x, -y + center.y));
    }
}



int PenInit
(
    Pen *pen,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a,
    double radius
)
{
    pen->red   = r;
    pen->green = g;
    pen->blue  = b;
    pen->alpha = a;
    pen->radius = radius;
    
    return 1;
}


/* 
 * GraphAtmosphere1D
 * =================
 * 
 * This renders an upside-down pyramid of cells.
 */
int GraphAtmosphere1D
(
    const char *title,
    Image *buffer,
    size_t layers,
    GraphAtmosphere1DCell *cells,
    double planet_radius,
    double planet_gravity,
    double simulation_height
)
{
    Pen pen; PenInit(&pen, PEN_BLACK, 1.0);
    Pen thinpen; PenInit(&thinpen, PEN_BLACK, 0.5);
    
    double height = (double) (buffer->size.y) * 1.15;
    double earth_height = 0.2;
    
    Image glyphs;
    ImageFill(buffer, 255, 255, 255, 255);
    if (!ImageLoad(&glyphs, glyphs_13x16_png, glyphs_13x16_png_len))
        { X(load_glyphs); }
    
    // Earth's radius
    GraphDrawCircle
    (
        buffer,
        &pen,
        Vector2Df
        (
            0,
            (double) (buffer->size.y)
        ),
        earth_height * height
    );
    
    // two rays from the center of the earth, making a cone
    {
        double r = 1.0 * height;
        double x0 = r * sin(radians(30));
        double y0 = (double) (buffer->size.y) - r * sin(radians(60));
        double x1 = r * sin(radians(60));
        double y1 = (double) (buffer->size.y) - r * sin(radians(30));
        
        GraphDrawLine
        (
            buffer,
            &pen,
            Vector2Df
            (
                0,
                (double) (buffer->size.y)
            ),
            Vector2Df
            (
                x0,
                y0
            )
        );
        
        GraphDrawLine
        (
            buffer,
            &pen,
            Vector2Df
            (
                0,
                (double) (buffer->size.y)
            ),
            Vector2Df
            (
                x1,
                y1
            )
        );
    }
    
    // Making a triangle at the radius of the earth and each cell
    for (size_t i = 0; i <= layers; i++)
    {
        double offset = ((double) i) / ((double) layers);
        double r = earth_height * height;
        r += offset * (1.0 - earth_height) * height;
        
        double x0 = r * sin(radians(30));
        double y0 = (double) (buffer->size.y) - r * sin(radians(60));
        double x1 = r * sin(radians(60));
        double y1 = (double) (buffer->size.y) - r * sin(radians(30));
        double xmax = height * sin(radians(60));
        
        GraphDrawLine
        (
            buffer,
            &pen,
            Vector2Df
            (
                x0,
                y0
            ),
            Vector2Df
            (
                x1,
                y1
            )
        );
        
        // And extend a horizontal line
        if (i < layers)
        {
            GraphAtmosphere1DCell cell = cells[i];
            
            GraphDrawLine
            (
                buffer,
                &thinpen,
                Vector2Df
                (
                    x1,
                    y1
                ),
                Vector2Df
                (
                    (double) (buffer->size.x),
                    y1
                )
            );
        
            // And write on it
            char str0[16]; char str1[512];
            sprintf(str0, "h=%.1f", cell.altitude / 1000.0);
            sprintf(str1,
                "p=%.2f "
                "P=%.0f "
                "v=%.2f "
                ,
                cell.density,
                cell.pressure,
                cell.velocity
            );
            
            WriteSlant(buffer, &glyphs, &pen, Vector2Df(x0 + 16.0, y0 - 7.5), Vector2Df(-4.0, 10), str0);
            Write(buffer, &glyphs, &pen, Vector2Df(xmax + 16.0, y1 - 15.0), str1);
        }
    }
    
    // print title, subtitle & key
    {
    char str0[512];
    sprintf(str0, "planet gravity: %.2f m/s^2", planet_gravity);
    Write(buffer, &glyphs, &pen, Vector2Df(16.0, 16.0), title);
    Write(buffer, &glyphs, &pen, Vector2Df(16.0, 48.0), str0);
    Write(buffer, &glyphs, &pen,
        Vector2Df(16.0 + (0.2 * (double) buffer->size.y) * 1.25, -64.0 + (double) buffer->size.y),
        "h=altitude (km), p=density (kg/m^3), P=pressure (N/m^2)");
    }
    
    // print planet radius and simulation total height
    {
        char str0[64]; char str1[64];
        sprintf(str0, "%.0f km", planet_radius / 1000.0);
        sprintf(str1, "%.0f km", simulation_height / 1000.0);
        
        WriteSlant(buffer, &glyphs, &pen,
            Vector2Df(56.0, -24.0 + (double) buffer->size.y),
            Vector2Df(-4.0, -6.5),
            str0);
        
        WriteSlant(buffer, &glyphs, &pen,
            Vector2Df(200.0, -395.0 + (double) buffer->size.y),
            Vector2Df(-4.0, -18),
            str1);
    }
    
    // print cell sizes
    {
    char str0[512];
    GraphAtmosphere1DCell bottom = cells[0];
    GraphAtmosphere1DCell top = cells[layers-1];
    
    sprintf(str0,
        "cell size: bottom %.3fx%.3fx%.3f km, top %.3fx%.3fx%.3f km",
        bottom.size.x / 1000.0,
        bottom.size.y / 1000.0,
        bottom.size.z / 1000.0,
        top.size.x / 1000.0,
        top.size.y / 1000.0,
        top.size.z / 1000.0
    );
    
    Write(buffer, &glyphs, &pen,
        Vector2Df(16.0 + (0.2 * (double) buffer->size.y) * 1.25,
                  -32.0 + (double) buffer->size.y),
        str0);
    }
    
    return 1;
    
    err_load_glyphs:
        return 0;
}











