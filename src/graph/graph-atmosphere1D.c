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
#include "graph/draw.h"
#include "graph/pen.h"
#include "graph/baked.h"



/* 
 * GraphAtmosphere1D
 * =================
 * 
 * This renders an upside-down pyramid of cells.
 */
int GraphAtmosphere1D
(
    Grapher *g,
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
    
    int font = GRAPH_GLYPHSET_NORMAL;
    double height = (double) (buffer->size.y) * 1.15;
    double earth_height = 0.2;
    
    ImageFill(buffer, 255, 255, 255, 255);
    
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
            
            GraphWriteSlanted(g, buffer, font, &pen,
                Vector2Df(x0 + 16.0, y0 - 7.5), Vector2Df(-4.0, 10), str0);
            GraphWrite(g, buffer, font, &pen,
                Vector2Df(xmax + 16.0, y1 - 15.0), str1);
        }
    }
    
    // print title, subtitle & key
    {
    char str0[512];
    sprintf(str0, "planet gravity: %.2f m/s^2", planet_gravity);
    GraphWrite(g, buffer, font, &pen, Vector2Df(16.0, 16.0), title);
    GraphWrite(g, buffer, font, &pen, Vector2Df(16.0, 48.0), str0);
    GraphWrite(g, buffer, font, &pen,
        Vector2Df(16.0 + (0.2 * (double) buffer->size.y) * 1.25, -64.0 + (double) buffer->size.y),
        "h=altitude (km), p=density (kg/m^3), P=pressure (N/m^2)");
    }
    
    // print planet radius and simulation total height
    {
        char str0[64]; char str1[64];
        sprintf(str0, "%.0f km", planet_radius / 1000.0);
        sprintf(str1, "%.0f km", simulation_height / 1000.0);
        
        GraphWriteSlanted(g, buffer, font, &pen,
            Vector2Df(56.0, -24.0 + (double) buffer->size.y),
            Vector2Df(-4.0, -6.5),
            str0);
        
        GraphWriteSlanted(g, buffer, font, &pen,
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
    
    GraphWrite(g, buffer, font, &pen,
        Vector2Df(16.0 + (0.2 * (double) buffer->size.y) * 1.25,
                  -32.0 + (double) buffer->size.y),
        str0);
    }
    
    return 1;
}











