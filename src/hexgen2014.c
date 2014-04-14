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
#   define SIZE Size2D(512, 512)
    Generator generator;
    World world;
    Image image;
    
    if (!ImageInit(&image, SIZE)) { X(ImageInit); }
    if (!GeneratorInit(&generator, 0)) { X(GeneratorInit); }
    GeneratorUseMaskSampler(&generator, SampleCircleGradiant);
    if (!WorldInit(&world, &generator, SIZE)) { X(WorldInit); }
    
    while (1)
    {
        if (!WorldGenerateHeightmap(&world, 1.5, 0.25))
            { X(WorldGenerateHeightmap); }
        
        if (WorldLandmassAtTopEdge(&world))
            { printf("REJECT heightmap: landmass at top edge\n"); continue; }
        
        if (WorldLandmassAtBottomEdge(&world))
            { printf("REJECT heightmap: landmass at bottom edge\n"); continue; }
        
        if (WorldLandmassAtLeftEdge(&world))
            { printf("REJECT heightmap: landmass at left edge\n"); continue; }
        
        if (WorldLandmassAtRightEdge(&world))
            { printf("REJECT heightmap: landmass at right edge\n"); continue; }
        
        break;
    }
    
    // for each season
    // {
    //WorldCalculateDirectSolarRadiation
    // }
    
    WorldRender_Elevation_Raw(&world, &image);
    ImageSaveTo(&image, "elevation-raw.png");
    
    WorldRender_Elevation_Quick(&world, &image);
    ImageSaveTo(&image, "elevation-quick.png");
    
    for (int i = 0; i < 12; i++)
    {
        double month = ((double) i) / 12.0;
        
        WorldCalculateDirectSolarRadiation
        (
            &world.sunlight,
            month, // yearly orbit normalised 0.0 to 1.0
            NORTHERN_SOLSTICE_EARTH, // when northern solstice occurs in orbit
            SEASONAL_TILT_EARTH, // degrees - severity of seasons
            1.0, // where 1.0 is the mean radius of the Earth
            1.0, // distance from sun in astronomical units e.g. 1.0 AU for Earth
            1.0, // luminosity relative to 1.0 for our Sun ~= 3.846 × 10^26 Watts
            GeoCoordinate(GEOCOORDINATE_UK), // land position see wgen/geocoordinates.h
            1000.0 // km from north to south of map
        );
        
        char filename0[256];
        char filename1[256];
        char filename2[256];
        
        sprintf(filename0, "sunlight-raw-%d.png", i);
        sprintf(filename1, "sunlight-quick-%d.png", i);
        sprintf(filename2, "fancy-%d.png", i);
        
        WorldRender_Sunlight_Raw(&world, &image);
        ImageSaveTo(&image, filename0);
        
        WorldRender_Sunlight_Quick(&world, &image);
        ImageSaveTo(&image, filename1);
    }
    
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
