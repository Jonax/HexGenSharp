/*
 
 src/graph/draw.h - graph drawing function prototypes
 
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

#ifndef HG14_GRAPH_DRAW_H
#   define HG14_GRAPH_DRAW_H

#include "graph/graph.h"
#include "graph/pen.h"

void GraphPlot(Image *buffer, Pen *pen, vector2Df xy);
void GraphDrawLine(Image *buffer, Pen *pen, vector2Df from, vector2Df to);
void GraphDrawCircle(Image *buffer, Pen *pen, vector2Df center, double radius);

vector2Df GraphWrite
(
    Grapher *g,
    Image *buffer,
    int glyphset,
    Pen *pen,
    vector2Df pos,
    const char *s
);

vector2Df GraphWriteSlanted
(
    Grapher *g,
    Image *buffer,
    int glyphset,
    Pen *pen,
    vector2Df pos,
    vector2Df slant,
    const char *s
);

#endif
