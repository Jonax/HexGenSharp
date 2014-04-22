/*
 
 src/image.h - image handling
 
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

#ifndef HG14_IMAGE_H
#   define HG14_IMAGE_H

#include "types.h"

# define RGBA_SIZE 4
# define RGBA_CHANNELS 4
# define RGBA_WRITE1(p,c) \
    *(p++) = c; *p++ = c; *p++ = c; *p++ = 255;
# define RGBA_WRITE3(p,r,g,b) \
    *(p++) = r; *p++ = g; *p++ = b; *p++ = 255;
# define RGBA_WRITE4(p,r,g,b,a) \
    *(p++) = r; *p++ = g; *p++ = b; *p++ = a;

typedef struct Image Image;

struct Image
{
    unsigned char *pixels;
    size2D size;
    size_t elements;
    size_t bytes;
};

int  ImageInit(Image *i, size2D size);
void ImageTeardown(Image *i);

int ImageLoad(Image *i, const unsigned char *data, size_t size);
int ImageSaveTo(Image *i, const char *path);
int ImageFill
(
    Image *i,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a
);

#endif
