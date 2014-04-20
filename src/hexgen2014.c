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
    Windsim windsim;
    Image image, image_windsim, image_graph;
    
    if (!ImageInit(&image, SIZE)) { X(ImageInit); }
    if (!ImageInit(&image_windsim, Size2D(48, 768))) { X(ImageInit); }
    if (!ImageInit(&image_graph, Size2D(1200, 800))) { X(ImageInit); } // 1800, 800
    
    if (!GeneratorInit(&generator, 0)) { X(GeneratorInit); }
    GeneratorUseMaskSampler(&generator, SampleCircleGradiant);
    if (!WorldInit(&world, &generator, SIZE)) { X(WorldInit); }
    if (!WindsimInit(&windsim, &world, Size3D(64, 64, 24))) { X(WindsimInit); }
    
    WorldDefinePlanet
    (
        &world,
        6371000.0, // radius
        9.81, // g
        1.0, // distance from sun in AU
        1.0 // relative solar luminosity
    );
    
    WorldDefineSeasons
    (
        &world,
        SEASONAL_TILT_EARTH,
        NORTHERN_SOLSTICE_EARTH
    );
    
    WorldDefineArea
    (
        &world,
        GeoCoordinate("0N 0E"),
        Vector3Df(12000.0, 12000.0, 10000.0)
    );
    
    WindsimRun(&windsim,  &image_windsim, &image_graph, 100);
    
    while (1)
    {
        if (!WorldGenerateHeightmap(&world, 1.5, 0.25)) //1.5, 0.25))
            { X(WorldGenerateHeightmap); }
        
        if (WorldLandmassAtTopEdge(&world))
            { printf("REJECT heightmap: landmass at top edge\n"); continue; }
        
        if (WorldLandmassAtBottomEdge(&world))
            { printf("REJECT heightmap: landmass at bottom edge\n"); continue; }
        
        if (WorldLandmassAtLeftEdge(&world))
            { printf("REJECT heightmap: landmass at left edge\n"); continue; }
        
        if (WorldLandmassAtRightEdge(&world))
            { printf("REJECT heightmap: landmass at right edge\n"); continue; }
        
        if (WorldLandMassProportion(&world) < 0.10) // TODO user limit
            { printf("REJECT heightmap: landmass proportion too low\n"); continue; }
        
        if (WorldLandMassProportion(&world) > 1.00) // TODO user limit
            { printf("REJECT heightmap: landmass proportion too high\n"); continue; }
        
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
        
        WorldCalculateDirectSolarRadiation(&world, month);
        
        char filename0[256];
        char filename1[256];
        
        sprintf(filename0, "sunlight-raw-%d.png", i);
        sprintf(filename1, "sunlight-quick-%d.png", i);
        
        WorldRender_Sunlight_Raw(&world, &image);
        ImageSaveTo(&image, filename0);
        
        WorldRender_Sunlight_Quick(&world, &image);
        ImageSaveTo(&image, filename1);
        
        //if (i == 5) { WindsimRun(&windsim, &image_windsim); }
    }
    
    return 1;
    
    err_WindsimInit:
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
