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
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text.RegularExpressions;
using System.Linq;
using HexGenSharp.Extensions;
using System.Device.Location;

namespace HexGenSharp
{
    public class GeoCoordinateEx
    {
		//GeoCoordinate position;

		//private static readonly Regex PARSE_REGEX;

		/*
		JW - Leaving this in, in case we need to parse a GeoCoord from a string.  
        static GeoCoordinate()
        {
            List<string> validPatterns = new List<string>();
            validPatterns.Add(@"(?<degree>[0-9]+)°(?<direction>N|S|W|E)");
            validPatterns.Add(@"(?<degree>[0-9]+)°(?<minute>[0-9\.]+)′(?<direction>N|S|W|E)");
            validPatterns.Add(@"(?<degree>[0-9]+)°(?<minute>[0-9]+)′(?<second>[0-9\.]+)″(?<direction>N|S|W|E)");
            validPatterns.Add(@"(?<sign>(-|+))(?<degree>[0-9\.]+)");

            PARSE_REGEX = new Regex(String.Join("|", validPatterns.Select(
                                        vp => String.Format("({0})", vp)
                                    )));
			
		}
		*/

		/* Turns e.g. 12°34′56″N 12°34′56″E into a geocoordinate.
           Also accepts the format +123.456 -987.654 */

		/* format examples:
            51°30′26″N 0°7′39″W
            51°30'26''N 0°7'39''W
            51°30'26.500''N 0°7'39''W
            23°27.500′N 23°27.500′E
            +12.34 -98.76
            12.34 -98.76
            */

		// JW: Can't use GeoCoordinate.GetDistanceTo() as it doesn't seem to factor arbitrary radius.
		public static double Distance(GeoCoordinate a, GeoCoordinate b, double radius)
		{
			// phi: latitude
			// lambda: longitude
			// d: distance along the surface of a sphere
			// r: radius

			double r = radius;
			double phi1 = a.Latitude.ToRadians();
			double phi2 = b.Latitude.ToRadians();
			double lambda1 = a.Longitude.ToRadians();
			double lambda2 = b.Longitude.ToRadians();

			// http://andrew.hedges.name/experiments/haversine/
			// http://www.movable-type.co.uk/scripts/latlong.html

			double phid = phi2 - phi1;
			double lambdad = lambda2 - lambda1;

			double ha = (Math.Sin(phid / 2) * Math.Sin(phid / 2)) +
				(Math.Cos(phi1) * Math.Cos(phi2) * (Math.Sin(lambdad / 2)) * (Math.Sin(lambdad / 2)));

			Debug.Assert(ha >= 0.0); // if this is hit by floating point error, try clamping
			Debug.Assert(ha <= 1.0); // if this is hit by floating point error, try clamping

			double hc = 2.0 * Math.Atan2(Math.Sqrt(ha), Math.Sqrt(1.0 - ha));
			double hd = r * hc;

			return hd;
		}

		public static GeoCoordinate Translate(GeoCoordinate geo, double radius, double distance, double bearing)
        {
            bearing = bearing.ToRadians();

            double r = radius;
            double d = distance;
            double phi1 = geo.Latitude.ToRadians();
            double lambda1 = geo.Longitude.ToRadians();

            double phi2 = Math.Asin
            (
                (Math.Sin(phi1) * Math.Cos(d / r)) +
                (Math.Cos(phi1) * Math.Sin(d / r) * Math.Cos(bearing))
            );

            double lambda2 = lambda1 + Math.Atan2
            (
                Math.Sin(bearing) * Math.Sin(d / r) * Math.Cos(phi1),
                Math.Cos(d / r) - Math.Sin(phi1) * Math.Sin(phi2)
            );

            phi2 = phi2.ToDegrees();
            lambda2 = lambda2.ToDegrees();

            return new GeoCoordinate(phi2, lambda2);
        }
    }
}