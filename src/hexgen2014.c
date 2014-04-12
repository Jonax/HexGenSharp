/*
 
 src/hexgen2014.c - main entry point
 
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
#include "wgen/wgen.h"
#include <stdio.h>

int example(void)
{
#   define SIZE Size2D(1024, 1024)
    Generator generator;
    World world;
    Image image;
    
    if (!ImageInit(&image, SIZE)) { X(ImageInit); }
    if (!GeneratorInit(&generator, 0)) { X(GeneratorInit); }
    if (!WorldInit(&world, &generator, SIZE)) { X(WorldInit); }
    if (!WorldGenerateHeightmap(&world, 1.5, 0.25)) { X(WorldGenerateHeightmap); }
    
    WorldRender_Elevation_Raw(&world, &image);
    ImageSaveTo(&image, "elevation-raw.png");
    
    WorldRender_Elevation_Quick(&world, &image);
    ImageSaveTo(&image, "elevation-quick.png");
    
    return 1;
    
    err_WorldGenerateHeightmap:
    err_WorldInit:
    err_GeneratorInit:
    err_ImageInit:
        return 0;
}


int main(void)
{
    if (!example()) { return EXIT_FAILURE; }
    
    return EXIT_SUCCESS;
}
