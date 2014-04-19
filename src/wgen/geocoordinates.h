/*

 wgen/geocoordinates.h - Longitude and Latitude conversions
 
 ------------------------------------------------------------------------------
 
 Copyright (c) 2013 - 2014 Ben Golightly <golightly.ben@googlemail.com>

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



#ifndef HG14_GEOCOORDS_H
#   define HG14_GEOCOORDS_H

#include "base.h"

/* Some "representative" geocoordinates based on major cities */
# define GEOCOORDINATE_UK           "51°30′N 0°08′W"
# define GEOCOORDINATE_RUSSIA       "55°45′N 37°37′E"
# define GEOCOORDINATE_USA          "38°53′42.4″N 77°02′12.0″W"
# define GEOCOORDINATE_AUSTRALIA    "35°18.48′S 149°7.47′E"
# define GEOCOORDINATE_BRAZIL       "15°47′S 47°52′W"
# define GEOCOORDINATE_SOUTH_AFRICA "26°12′16″S 28°2′44″E"
# define GEOCOORDINATE_CHINA        "39°55′N 116°23′E"
# define GEOCOORDINATE_GREECE       "37°58′N 23°43′E"
# define GEOCOORDINATE_ITALY        "41°54′N 12°29′E"
# define GEOCOORDINATE_ICELAND      "64°08′N 21°56′W"
# define GEOCOORDINATE_EGYPT        "30°2′N 31°13′E"
# define GEOCOORDINATE_KAZAKHSTAN   "51°10′N 71°25′E"
# define GEOCOORDINATE_INDIA        "28°36.8′N 77°12.5′E"

typedef struct geocoordinate geocoordinate;
typedef struct sexagesimal sexagesimal;

struct sexagesimal
{
    int degree;
    int minute;
    int second;
    int fraction;
    char cardinal_direction; // N E S W
};

struct geocoordinate
{
    sexagesimal longitude;
    sexagesimal latitude;
    int valid;
};



HEADER_FUNC
sexagesimal GeoCoordinateLongitude(geocoordinate p)
{
    return p.longitude;
}


HEADER_FUNC
sexagesimal GeoCoordinateLatitude(geocoordinate p)
{
    return p.latitude;
}


HEADER_FUNC
int GeoCoordinateValid(geocoordinate p)
{
    return p.valid;
}


void GeoCoordinatePrint(geocoordinate g);


/* Turns e.g. 12°34′56″N 12°34′56″E into a geocoordinate.
   Also accepts the format +123.456 -987.654 */
geocoordinate GeoCoordinate(const char *utf8string);

sexagesimal Sexagesimal
(
    int degree,
    int minute,
    int second,
    int fraction,
    char cardinal_direction
);

double LatitudeToDouble(sexagesimal s);  // -90.0 to 90.0
double LongitudeToDouble(sexagesimal s); // -180.0 to 180.0

double GeoCoordinateDistance(double radius, geocoordinate a, geocoordinate b);

geocoordinate GeoCoordinateTranslate
(
    geocoordinate a,
    double radius,
    double distance,
    double bearing
);

#endif

