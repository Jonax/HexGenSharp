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
#include <limits.h>


int ImageInit(Image *i, size2D size)
{
    if (!i)          { X2(bad_arg, "NULL image pointer");      }
    if (size.x == 0) { X2(bad_arg, "size.x must be non-zero"); }
    if (size.y == 0) { X2(bad_arg, "size.y must be non-zero"); }
    
    i->size     = size;
    i->elements = size.x * size.y;
    i->bytes    = i->elements * RGBA_SIZE;
    
    i->pixels = malloc(i->bytes);
    if (!i->pixels) { X(alloc_pixels); }
    
    return 1;
    
    err_alloc_pixels:
    err_bad_arg:
        return 0;
}


void ImageTeardown(Image *i)
{
    if (!i) { X2(bad_arg, "NULL image pointer"); }
    
    /* NOTE that future versions of stb_image may require stb_image_free()
     * to be called instead of just free(). Currently a buffer from any source
     * can be freed normally. */
    
    free(i->pixels);
    i->pixels = NULL;
    
    err_bad_arg:
        return;
}


int ImageLoad(Image *i, const unsigned char *data, size_t size)
{
    if (!i)        { X2(bad_arg,    "NULL image pointer"); }
    if (!data)     { X2(bad_arg,     "NULL data pointer"); }
    if (size == 0) { X2(bad_arg, "size must be non-zero"); }
    
    if (size > INT_MAX) { X2(bad_size,
        "stb_image cannot load more than INT_MAX bytes in any one image"); }
    
    unsigned char *uc;
    int x, y, comp;
    
    uc = stbi_load_from_memory(data, (int) size, &x, &y, &comp, RGBA_CHANNELS);
    if (!uc) { X(stbi_load_from_memory); }
    
    i->size     = Size2D((size_t) x, (size_t) y);
    i->elements = i->size.x * i->size.y;
    i->bytes    = i->elements * RGBA_SIZE;
    
    i->pixels = uc;
    
    return 1;
    
    err_stbi_load_from_memory:
    err_bad_size:
    err_bad_arg:
        return 0;
}


int ImageSaveTo(Image *i, const char *path)
{
    if (!i)     { X2(bad_arg, "NULL image pointer"); }
    if (!path)  { X2(bad_arg, "NULL path pointer");  }
    if (!*path) { X2(bad_arg, "empty path string");  }
    
    if (!stbi_write_png
    (
        path,
        (int) i->size.x,
        (int) i->size.y,
        RGBA_CHANNELS,
        i->pixels,
        (int) (i->size.x) * RGBA_SIZE // row stride in bytes
    ))
    { X2(stbi_write_png, path); }
    
    return 1;
    
    err_stbi_write_png:
    err_bad_arg:
        return 0;
}


int ImageFill
(
    Image *img,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a
)
{
    if (!img) { X2(bad_arg, "NULL image pointer"); }
    
    unsigned char *p = img->pixels;
    
    for (size_t i = 0; i < img->elements; i++)
    {
        RGBA_WRITE4(p, r, g, b, a);
    }
    
    return 1;
    
    err_bad_arg:
        return 0;
}
