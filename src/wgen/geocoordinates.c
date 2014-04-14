/*

 wgen/geocoordinates.c - Longitude and Latitude conversions
 
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

#include <stdlib.h> // div
#include <string.h> // memcpy
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "base.h"
#include "utf8.h"
#include "types.h" // radians

#include "wgen/geocoordinates.h"

sexagesimal Sexagesimal
(
    int degree,
    int minute,
    int second,
    int fraction,
    char cardinal_direction
)
{
    sexagesimal s;
    
    s.degree = degree;
    s.minute = minute;
    s.second = second;
    s.fraction = fraction;
    s.cardinal_direction = cardinal_direction;
    
    return s;
}



static void SexagesimalPrint(sexagesimal s)
{
    printf("%d°%d′%d.%d″%c\n",
        s.degree,
        s.minute,
        s.second,
        s.fraction,
        s.cardinal_direction);
}


void GeoCoordinatePrint(geocoordinate g)
{
    printf("GeoCoordinate:\n");
    SexagesimalPrint(g.latitude);
    SexagesimalPrint(g.longitude);
}


double LatitudeToDouble(sexagesimal s)
{
    double degree = (double) (s.degree % 90);
    double minute = (double) (s.minute % 60); minute /= 60.0;
    double second = (double) (s.second & 60); second /= 360.0;
    double fraction = (double) (s.fraction); fraction /= 3600.0;
    
    double result = degree + minute + second + fraction;
    result = fmod(result, 90.0);
    
    switch (s.cardinal_direction)
    {
        case 'N': break;
        case 'S': result = -result; break;
        default: X(invalid_cardinal_direction);
    }
    
    return result;
    
    err_invalid_cardinal_direction:
        return 0.0;
}


double LongitudeToDouble(sexagesimal s)
{
    double degree = (double) (s.degree % 180);
    double minute = (double) (s.minute % 60); minute /= 60.0;
    double second = (double) (s.second & 60); second /= 360.0;
    double fraction = (double) (s.fraction); fraction /= 3600.0;
    
    double result = degree + minute + second + fraction;
    result = fmod(result, 180.0);
    
    switch (s.cardinal_direction)
    {
        case 'E': break;
        case 'W': result = -result; break;
        default: X(invalid_cardinal_direction);
    }
    
    return result;
    
    err_invalid_cardinal_direction:
        return 0.0;
}


double GeoCoordinateDistance(double radius, geocoordinate a, geocoordinate b)
{
    // phi: latitude
    // lambda: longitude
    // d: distance along the surface of a sphere
    // r: radius
    
    double r = radius;
    double phi1 = radians(LatitudeToDouble(GeoCoordinateLatitude(a)));
    double phi2 = radians(LatitudeToDouble(GeoCoordinateLatitude(b)));
    double lambda1 = radians(LongitudeToDouble(GeoCoordinateLongitude(a)));
    double lambda2 = radians(LongitudeToDouble(GeoCoordinateLongitude(b)));
    
    // http://andrew.hedges.name/experiments/haversine/
    // http://www.movable-type.co.uk/scripts/latlong.html
    
    double phid = phi2 - phi1;
    double lambdad = lambda2 - lambda1;
    
    double ha = (sin(phid/2) * sin(phid/2)) +
        (cos(phi1) * cos(phi2) * (sin(lambdad/2)) * (sin(lambdad/2)));
    
    assert(ha >= 0.0); // if this is hit by floating point error, try clamping
    assert(ha <= 1.0); // if this is hit by floating point error, try clamping
    
    double hc = 2.0 * atan2(sqrt(ha), sqrt(1.0 - ha));
    double hd = r * hc;
    
    return hd;
}


geocoordinate GeoCoordinateTranslate
(
    geocoordinate a,
    double radius,
    double distance,
    double bearing
)
{
    bearing = radians(bearing);
    
    double r = radius;
    double d = distance;
    double phi1 = radians(LatitudeToDouble(GeoCoordinateLatitude(a)));
    double lambda1 = radians(LongitudeToDouble(GeoCoordinateLongitude(a)));
    
    double phi2 = asin
    (
        (sin(phi1) * cos(d/r)) +
        (cos(phi1) * sin(d/r) * cos(bearing))
    );
    
    double lambda2 = lambda1 + atan2
    (
        sin(bearing) * sin(d/r) * cos(phi1),
        cos(d/r) - sin(phi1) * sin(phi2)
    );
    
    phi2 = degrees(phi2);
    lambda2 = degrees(lambda2);
    
    // TODO FIXME do this properly
    char msg[512];
    sprintf(msg, "%f° %f°", phi2, lambda2);
    
    return GeoCoordinate(msg);
}




/* format examples:
    DEGREE°D
    DEGREE°MINUTE′D
    DEGREE°MINUTE.FRACTION″D
    DEGREE°MINUTE′SECOND″D
    DEGREE°MINUTE′SECOND.FRACTION″D
    +DEGREE
    +DEGREE.FRACTION
    -DEGREE
    -DEGREE.FRACTION
*/

# define TOKEN_MAX_SIZE      16

# define TOKEN_INVALID              0
# define TOKEN_NUMBER               1
# define TOKEN_DEGREE_UNIT          2
# define TOKEN_MINUTE_UNIT          3
# define TOKEN_SECOND_UNIT          4
# define TOKEN_DECIMAL_POINT        5
# define TOKEN_CARDINAL_DIRECTION   6
# define TOKEN_SIGN                 7

# define MODE_DEGREE                1
# define MODE_DEGREE_FRACTION       2
# define MODE_MINUTE                3
# define MODE_MINUTE_FRACTION       4
# define MODE_SECOND                5
# define MODE_SECOND_FRACTION       6
# define MODE_DIRECTION             7


static int CharIsNumeric(char token)
{
    return ((token >= '0') && (token <= '9'));
}


static size_t NextSexagesimalToken(const char *start, char buf[16], unsigned int *type)
{
    size_t i = 0;
    buf[0] = '\0';
    
    if (*start == '\0') { return 0; }
    
    if (ParseMultibyte(start, &i))
    {
        if (0 == strncmp(start, "°", i))
        {
            *type = TOKEN_DEGREE_UNIT;
        }
        else if (0 == strncmp(start, "′", i))
        {
            *type = TOKEN_MINUTE_UNIT;
        }
        else if (0 == strncmp(start, "″", i))
        {
            *type = TOKEN_SECOND_UNIT;
        }
        else
        {
            return 0;
        }
        
        return i;
    }
    
    if (CharIsNumeric(*start))
    {
        while (CharIsNumeric(start[i])){ i++; }
        
        if (i >= (TOKEN_MAX_SIZE - 1)) { return 0; }
        memcpy(buf, start, i);
        buf[i] = '\0';
        *type = TOKEN_NUMBER;
        return i;
    }
    
    switch(*start)
    {
        case ' ':
            return 0;
        
        case '\'':
            if (start[1] == '\'')
            {
                return TOKEN_SECOND_UNIT;
                return 2;
            }
            else
            {
                return TOKEN_MINUTE_UNIT;
                return 1;
            }
        
        case '.':
            *type = TOKEN_DECIMAL_POINT;
            return 1;
        
        case '+':
        case '-':
            buf[0] = *start;
            buf[1] = '\0';
            *type = TOKEN_SIGN;
            return 1;
        
        case 'N':
        case 'E':
        case 'S':
        case 'W':
            buf[0] = *start;
            buf[1] = '\0';
            *type = TOKEN_CARDINAL_DIRECTION;
            return 1;
    }
    
    return 0;
}


static int leadingzeroes(char buf[16])
{
    int count = 0;
    
    for (size_t i = 0; buf[i] != '\0'; i++)
    {
        if (buf[i] == '0') { count++; } else { break; }
    }
    
    return count;
}

const char *ParseSexagesimal
(
    const char *start,
    geocoordinate *g,
    sexagesimal *s,
    const char *cardinalities
)
{
    if (!start) { goto invalid; }
    
    unsigned int type = 0;
    unsigned int mode = MODE_DEGREE;
    char buf[TOKEN_MAX_SIZE];
    size_t offset = 0;
    int num;
    
    int allow_minutes = 1;
    int allow_seconds = 1;
    
    s->cardinal_direction = cardinalities[0];
    
    while (1)
    {
        size_t token_size = NextSexagesimalToken(&start[offset], buf, &type);
        offset += token_size;
        
        if (!token_size) { break; }
        // printf("Token %u '%s' %u %u\n", token_size, buf, type, mode);
        
        switch (type)
        {
            case TOKEN_SIGN:
                if (offset != 1) { goto invalid; }
                
                switch (buf[0])
                {
                    case '+':
                        s->cardinal_direction = cardinalities[0];
                        break;
                    case '-':
                        s->cardinal_direction = cardinalities[1];
                        break;
                    default:
                        goto invalid;
                }
                break;
            
            case TOKEN_DEGREE_UNIT:
                switch (mode)
                {
                    case MODE_DEGREE:
                    case MODE_DEGREE_FRACTION:
                        mode = MODE_MINUTE;
                        break;
                    default:
                        goto invalid;
                }
                break;
            
            case TOKEN_MINUTE_UNIT:
                switch (mode)
                {
                    case MODE_MINUTE:
                    case MODE_MINUTE_FRACTION:
                        mode = MODE_SECOND;
                        break;
                    default:
                        goto invalid;
                }
                break;
            
            case TOKEN_SECOND_UNIT:
                switch (mode)
                {
                    case MODE_SECOND:
                    case MODE_SECOND_FRACTION:
                        mode = MODE_DIRECTION;
                        break;
                    default:
                        goto invalid;
                }
                break;
            
            case TOKEN_DECIMAL_POINT:
                switch (mode)
                {
                    case MODE_DEGREE:
                        mode = MODE_DEGREE_FRACTION;
                        allow_minutes = 0;
                        break;
                    case MODE_MINUTE:
                        if (!allow_minutes) { goto invalid; }
                        mode = MODE_MINUTE_FRACTION;
                        allow_seconds = 0;
                        break;
                    case MODE_SECOND:
                        if (!allow_seconds) { goto invalid; }
                        mode = MODE_SECOND_FRACTION;
                        break;
                    default:
                        goto invalid;
                }
                break;
            
            case TOKEN_CARDINAL_DIRECTION:
                for (size_t i = 0; cardinalities[i] != '\0'; i++)
                {
                    if (buf[0] == cardinalities[i])
                    {
                        s->cardinal_direction = buf[0];
                        goto continue0;
                    }
                }
                goto invalid;
                continue0: break;
          
            case TOKEN_NUMBER:
                num = atoi(buf);
                
                switch(mode)
                {
                    case MODE_DEGREE:
                        s->degree = num;
                        break;
                    
                    case MODE_DEGREE_FRACTION:
                        
                        if (num > 0)
                        {
                            num = 60 * num /
                                (int) pow(10, 1 + leadingzeroes(buf) + (int) log10(num));
                        }
                        
                        s->minute = num;
                        break;
                        
                    case MODE_MINUTE:
                        s->minute = num;
                        break;
                    
                    case MODE_MINUTE_FRACTION:
                        
                        if (num > 0)
                        {
                            num = 60 * num /
                                (int) pow(10, 1 + leadingzeroes(buf) + (int) log10(num));
                        }
                        
                        s->second = num;
                        break;
                    
                    case MODE_SECOND:
                        s->second = num;
                        break;
                    
                    case MODE_SECOND_FRACTION:
                        s->fraction = num;
                        break;
                    
                    default:
                        goto invalid;
                }
                break;
        }
    }
    
    return &start[offset];
    
    invalid:
        g->valid = 0;
        return NULL;
}




/* format examples:
    51°30′26″N 0°7′39″W
    51°30'26''N 0°7'39''W
    51°30'26.500''N 0°7'39''W
    23°27.500′N 23°27.500′E
    +12.34 -98.76
    12.34 -98.76
*/
geocoordinate GeoCoordinate(const char *utf8string)
{
    geocoordinate g;
    g.valid = 1;
    
    const char *offset = utf8string;
    
    g.latitude  = Sexagesimal(0, 0, 0, 0, '?');
    g.longitude = Sexagesimal(0, 0, 0, 0, '?');
    
    offset = ParseSexagesimal(offset, &g, &g.latitude,  "NS");
    offset = ParseSexagesimal(offset + 1, &g, &g.longitude, "EW");
    
    if (!g.valid) { printf("(invalid geocoordinate)\n"); }
    
    //SexagesimalPrint(g.latitude);
    //SexagesimalPrint(g.longitude);
    
    return g;
}

