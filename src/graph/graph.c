/*
 
 src/graph/graph.c - graph plotting
 
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

#include "graph/baked.h"
#include "graph/graph.h"
#include "graph/pen.h"

struct Grapher
{
    Image glyphset[GRAPH_GLYPHSET_COUNT]; // TODO Glyphset struct with sizes
    Pen default_pen;
};


Grapher *GrapherNew(void)
{
    Grapher *g = malloc(sizeof(Grapher));
    if (!g) { X(alloc_grapher); }
    
    if (!PenInit(&g->default_pen, PEN_BLACK, 0.5)) { X(init_pen); }
    
#   define GLYPHSET_LOAD(type, file) \
        if (!ImageLoad(&g->glyphset[GRAPH_GLYPHSET_##type], file, file ## _len)) \
            { X2(glyphset_load_##type, __STRING(file)); }
    
    GLYPHSET_LOAD(NORMAL, glyphs_13x16_png);
    
    return g;
    
    err_glyphset_load_NORMAL:
    err_init_pen:
        free(g);
    err_alloc_grapher:
        return NULL;
}


void GrapherFree(Grapher *g)
{
    if (!g) { X2(bad_arg, "NULL grapher pointer"); }
    
    for (size_t i = 0; i < GRAPH_GLYPHSET_COUNT; i++)
    {
        ImageTeardown(&g->glyphset[i]);
    }
    
    free(g);
    
    return;
    
    err_bad_arg:
        return;
}


Image *GrapherGlyphset(Grapher *g, int index)
{
    if (!g) { X2(bad_arg, "NULL grapher pointer"); }
    
    switch (index)
    {
        case GRAPH_GLYPHSET_NORMAL:
            return &g->glyphset[index];
        
        default:
            X(invalid_glyphset_id);
    }
    
    err_invalid_glyphset_id:
    err_bad_arg:
        return NULL;
}
