/*
 
 src/image.c - image handling
 
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

#include "base.h" // exceptions
#include "image.h"
#include "vendor/stb_image/stb_image.h"
#include "vendor/stb_image/stb_image_write.h"
#include <stdio.h>

int ImageInit(Image *i, size2D s)
{
    i->size = s;
    i->bytes = (s.x * s.y); // FIXME safe to multiply by four?
                            // TODO have elements and bytes separately
    
    i->pixels = malloc(i->bytes * 4); // RGBA
    if (!i->pixels) { X(alloc_pixels); }
    
    return 1;
    
    err_alloc_pixels:
        return 0;
}


int ImageLoad(Image *i, const unsigned char *data, size_t size)
{
    int x, y, comp;
    unsigned char *uc = stbi_load_from_memory(data, (int) size, &x, &y, &comp, 4);
    
    if (!uc) { X(stbi_load_from_memory); }
    
    i->size = Size2D((size_t) x, (size_t) y);
    i->bytes = (i->size.x * i->size.y);
    
    i->pixels = uc;
    
    return 1;
    
    err_stbi_load_from_memory:
        return 0;
}


int ImageSaveTo(Image *i, const char *path)
{
    if (!stbi_write_png
    (
        path,
        (int) i->size.x,
        (int) i->size.y,
        4, // RGBA
        i->pixels,
        (int) (i->size.x) * 4 // stride in bytes
    ))
    { X2(stbi_write_png, path); }
    
    return 1;
    
    err_stbi_write_png:
        return 0;
}


int ImageFill(Image *img, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char *p = img->pixels;
    
    for (size_t i = 0; i < img->bytes; i++) // FIXME elements instead
    {
        RGBA_WRITE4(p, r, g, b, a);
    }
    
    return 1;
}

