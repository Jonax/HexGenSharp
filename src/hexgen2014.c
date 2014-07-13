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
    
    Grapher *grapher = GrapherNew();
    if (!grapher) { X(GrapherNew); }
    
    if (!ImageInit(&image, SIZE)) { X(ImageInit); }
    if (!ImageInit(&image_windsim, Size2D(48, 768))) { X(ImageInit); }
    if (!ImageInit(&image_graph, Size2D(1200, 800))) { X(ImageInit); } // 1800, 800
    
    if (!GeneratorInit(&generator, 0)) { X(GeneratorInit); }
    GeneratorUseMaskSampler(&generator, SampleCircleGradiant);
    GeneratorUseGrapher(&generator, grapher);
    
    if (!WorldInit(&world, &generator, SIZE)) { X(WorldInit); }
    if (!WindsimInit(&windsim, &world, Size3D(4, 4, 24))) { X(WindsimInit); }
    
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
        GeoCoordinate(GEOCOORDINATE_UK),
        Vector3Df(1000 * 1000.0, 1000 * 1000.0, 1350.0) // UK island size
    );
        
    /* WorldDefineArea
    (
        &world,
        GeoCoordinate("0N 0E"),
        Vector3Df(8000 * 1000.0, 8000 * 1000.0, 8850.0) // Tropical continent size
    ); */
    
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
    
    GrapherFree(grapher);
    
    return 1;
    
    err_WindsimInit:
    err_WorldGenerateHeightmap:
    err_WorldInit:
    err_GeneratorInit:
    err_ImageInit:
        GrapherFree(grapher);
    err_GrapherNew:
        return 0;
}

int main_old(void)
{
    if (!example()) { return EXIT_FAILURE; }
    
    return EXIT_SUCCESS;
}


void usage(void) { printf (
"Hero Extant 2014 World Generator (command line)\n\n"
"Usage: " PROGID " [-OPTION VALUE]... -from FILE [PATH]\n"
"Generates a world using the given OPTIONS and defaults from FILE, outputting\n"
"the result to a folder in the current directory or at PATH.\n"
"e.g. " PROGID " -from hexgen2014.txt\n"
"e.g. " PROGID " -from hexgen2014.txt ~/my-worlds\n"
"e.g. " PROGID " -name \"My World\" -detail high -from hexgen2014.txt\n"
"\n"
"Usage: " PROGID " ?OPTION... -from FILE\n"
"Prints acceptable values for a given option\n"
"e.g. " PROGID " ?detail ?planet ?map -from hexgen2014.txt\n"
"\n"
"Options:\n"
"    -dest (PATTERN)  PATTERN is a list of strings and tokens specifying the\n"
"                       name of a created output directory\n"
"                       e.g. -dest (name, \"-\", seed)\n"
"    -detail DETAIL   select all options in FILE matching the given DETAIL type\n"
"                       controlling output detail and weather simulation quality\n"
"    -from FILE       FILE is a file specifying groups of default values and\n"
"                       default values for command line arguments\n"
"    -map MAP         select all options in FILE matching the given MAP type\n"
"                       which controls the shape and weather of a generated map\n"
"    -name NAME       names the generated world. If NAME contains spaces, it\n"
"                       should be quoted e.g. -name \"My World\"\n"
"    -outputs OUTPUTS selects all options in FILE matching the given OUTPUTS\n"
"                       type controlling what is written by the program\n"
"    -period PERIOD   select all options in FILE matching the given PERIOD type\n"
"                       controlling snapshots of months or seasons\n"
"    -planet PLANET   select all options in FILE matching the given PLANET type\n"
"                       modelling the planet and solar system\n"
"    -seed SEED       seed for the random number generator; use random for a\n"
"                       seed based on current time, else any fixed integer\n"
"    -verbosity 0-2   quantity of text printed to stdout, from 0 (minimal) to\n"
"                      1 (normal) or 2 (extra). Defaults to 1\n"
"    -years YRS       integer number of years to record. Defaults to 1\n"
"\n"
"Note that individual options can be overridden (-verbosity 2 is helpful for\n"
"this) e.g. " PROGID " -map tropical -map-energy \"1.5 normalised\" -from...\n"
"\n"
"Information:\n"
"    --help           display this page\n"
"    --version        output version and copyright information and exit\n"
"\n"
"Exit status:\n"
"    0 if OK,\n"
"    1 if unable to parse FILE or command line arguments,\n"
"    2 if unable to get write permissions,\n"
"    3 on other program error\n"
);}


int main(int argc, char *argv[])
{
    UNUSED(argv);
    
    if (argc == 1) { usage(); } else { return main_old(); }
    
    return EXIT_SUCCESS;
}
