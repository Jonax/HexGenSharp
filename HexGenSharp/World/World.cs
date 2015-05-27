using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Device.Location;
using MathNet.Spatial.Euclidean;
using HexGenSharp.Extensions;

namespace HexGenSharp
{
    public partial class World
    {
        public class PlanetConfig
        {
            public double Radius { get; set; }              // metres
            public double Gravity { get; set; }             // m/s^2
            public double DistanceFromSun { get; set; }     // in AU
            public double SolarLuminosity { get; set; }     // normalised relative to our sun at 1.0. Watts.
        }

        public class AreaConfig
        {
            public GeoCoordinate Center { get; set; }       // xy center of map on sphere
            public Vector3D Dimension { get; set; }        // surface width from top/bottom, left/right, floor/ceil
        }

        public class SeasonConfig
        {
            public double AxialTilt { get; set; }           // degrees; earth is 23.4°
            public double NorthernSolstice { get; set; }    // seasonal lag; point in orbit where this occurs (Earth is at 0.222)
        }

        public double SeaProportion { get; set; }
        public double SeaLevel { get; set; }

        private PlanetConfig _planet;
        public PlanetConfig Planet
        {
            get { return _planet; }
            set
            {
                Debug.Assert(value.Radius > 0.0, "radius must be > 0");
                Debug.Assert(value.Gravity > 0.0, "gravity must be > 0");
                Debug.Assert(value.DistanceFromSun > 0.0, "distance from sun must be > 0");
                Debug.Assert(value.SolarLuminosity > 0.0, "solar_luminosity must be > 0");

                _planet = value;
            }
        }

        private AreaConfig _area;
        public AreaConfig Area
        {
            get { return _area; }
            set
            {
                Debug.Assert(value.Center != null, "geocoordinate invalid");
                Debug.Assert(value.Dimension.X > 0.0, "dimension.x must be > 0");
                Debug.Assert(value.Dimension.Y > 0.0, "dimension.y must be > 0");
                Debug.Assert(value.Dimension.Z > 0.0, "dimension.z must be > 0");
                Debug.Assert(Planet != null, "WorldDefinePlanet first");

                double circumference = 2.0 * Math.PI * Planet.Radius;
                Debug.Assert(circumference >= value.Dimension.X, "dimension.x is too large for the planet radius");
                Debug.Assert(circumference >= value.Dimension.Y, "dimension.y is too large for the planet radius");
                Debug.Assert(value.Dimension.Z <= 50 * 1000.0, "dimension.z is too large for an atmosphere simulation");

                _area = value;
            }
        }

        private SeasonConfig _season;
        public SeasonConfig Season
        {
            get { return _season; }
            set
            {
                Debug.Assert(value.AxialTilt >= -180.0, "tilt must be >= -180");
                Debug.Assert(value.AxialTilt <= +180.0, "tilt must be <= 180");
                Debug.Assert(value.NorthernSolstice >= -1.0, "northern solstice must be > -1.0");
                Debug.Assert(value.NorthernSolstice <= +1.0, "northern solstice must be < 1.0");

                _season = value;
            }
        }

        private Doubles2D _elevation;
        private Doubles2D _sunlight;

		public SimplexNoise noise;

		private IMask mask;

        public World(Size size, SimplexNoise noise, IMask mask = null)
        {
			Debug.Assert(size.Width > 0, "Bad size");
			Debug.Assert(size.Height > 0, "Bad size");

            this.noise = noise;

			_elevation = new Doubles2D(size);
			_sunlight = new Doubles2D(size);

			this.mask = mask ?? new MaskBase();
		}

		public void ApplyNoise(
			double energy,			// 0.6 to 3.0.  Higher == more islands
			double turbulance)		// -0.5 to 1.0. Controls shape and contrast
		{
			// longest length
			uint length = Convert.ToUInt32(Math.Max(_elevation.Width, _elevation.Height));

			/* This function applies multiple octaves of noise in order to generate an
			* interesting heightmap.
			* 
			* We want the smallest octave of noise to have either pixel or sub-pixel
			* interpolation for detail.
			* 
			* The last octave is at 2^(iterations-1)
			* Desired detail: for length/2^(iterations-1) to be <= 1 pixel
			* therefore iterations = log_2(length) + 1, rounded up
			*/
			uint iterations = (uint)(1.5 + (Math.Log((double)length) / Math.Log(2.0)));

			// JW - See about adding a delegate to pass debug/normal log output to.  
			Console.WriteLine("Octaves of noise for {0}x{0} : {1}\n", Convert.ToUInt32(length), iterations);

			double width = Convert.ToDouble(_elevation.Width);
			double height = Convert.ToDouble(_elevation.Height);

			/* How chaotic should the first octave of noise be? For a low value we are
			* "zoomed out" and interpolating between two noise values. This will give
			* us a smooth gradient between points. This is good for generating
			* strongly defined continents.
			* 
			* For a high value we are zoomed in and will cross more distinct values.
			* This is good for generating land that is broken up.
			* 
			* Try values from 0.5 to 3.0.
			*/
			double scale = energy / Convert.ToDouble(length);
			double scale_r = 1.0 / scale; // reciprocal

			/* The first octave is what dominates the shape of the map. Further octaves
			* give finer detail. Therefore if you want a stand-alone continent in the
			* centre of the map, you can set the first octave to be a circular
			* gradient. Or you can use your own image to get a custom shape.
			*/
			for (uint i = 0; i < _elevation.Area; ++i)
			{
				double y = Math.Floor(Convert.ToDouble(i) / _elevation.Width);
				double x = Math.Floor(Convert.ToDouble(i) % _elevation.Width);

				_elevation.Nodes[i] = scale_r * mask.Apply(x, y, width, height);
			}
    
			/* Octaves of noise */
			for (uint i = 1; i < iterations; ++i)
			{
				scale *= 2;   // double the scale for each octave
				scale_r /= 2; // this halves the reciprocal
    
				for (uint j = 0; j < _elevation.Area; ++j)
				{
					double y = Math.Floor(Convert.ToDouble(j) / _elevation.Width);
					double x = Math.Floor(Convert.ToDouble(j) % _elevation.Width);
            
					double turb0 = Math.Abs(noise.Generate(x * scale * 3.0, y * scale * 3.0));
					double turb1 = Math.Abs(noise.Generate(x * scale * 7.0, y * scale * 7.0));
					double turb = turb0 * turb1 * turbulance;

					_elevation.Nodes[j] += scale_r * (noise.Generate(x * scale, y * scale) + turb);
				}
			}
		}

		private void RollHeightmapCandidate(
			double energy,			// 0.6 to 3.0.  Higher == more islands
			double turbulance)		// -0.5 to 1.0. Controls shape and contrast
		{
			turbulance /= energy;

			/* Seed the state used by the noise function */
			// JW: Placeholder for now.  Properly seed it later.  
            //noise.Perm = new byte[512];
            string test = String.Join("", noise.Perm.Select(p => String.Format("{0:X}", p)));

			/* Apply several octaves of noise at a specific scale */
			ApplyNoise(energy, turbulance);

			/* Normalise elevation between 0.0 to 1.0 */
			_elevation.Normalise();

            /* Create a sea floor by clamping anything below 0.6 */
            // JW - Only do if set to a reasonable value (i.e. 0 is not valid)
            if (SeaProportion > 0)
            {
                _elevation.ClampFloorTo(SeaProportion);
            }

			/* Normalise elevation again between 0.0 to 1.0 */
			_elevation.Normalise();
		}

        public bool GenerateHeightmap(
            double energy,			// 0.6 to 3.0.  Higher == more islands
            double turbulance)      // -0.5 to 1.0. Controls shape and contrast
        {
            // JW - Certainly has to be a better way of handling this.  Move it to inside World?
            // JW - See about adding a "maximum tries" value here (or infinite if left null/zero).
            RollHeightmapCandidate(1.5, 0.25);

            /*
            if (LandmassAtTopEdge)
            {
                Console.WriteLine("REJECT heightmap: landmass at top edge");
                return false;
            }

            if (LandmassAtBottomEdge)
            {
                Console.WriteLine("REJECT heightmap: landmass at bottom edge");
                return false;
            }

            if (LandmassAtLeftEdge)
            {
                Console.WriteLine("REJECT heightmap: landmass at left edge");
                return false;
            }

            if (LandmassAtRightEdge)
            {
                Console.WriteLine("REJECT heightmap: landmass at right edge");
                return false;
            }

            if (LandMassProportion < 0.10)	// TODO user limit
            {
                Console.WriteLine("REJECT heightmap: landmass proportion too low");
                return false;
            }

            if (LandMassProportion > 1.00)	// TODO user limit
            {
                Console.WriteLine("REJECT heightmap: landmass proportion too high");
                return false;
            }
            */

            return true;
        }

        private bool LandmassAtTopEdge
		{
			get
			{
				// JW: See if all the edge functions can be moved to IEnumerable properties
				// inside Doubles2D (possibly as generators).  
				// JW: Checks the very top row, seeing if any are above sea level.  
				return _elevation.Nodes.Take(Convert.ToInt32(_elevation.Width))
									   .Any(v => v >= SeaLevel);
			}
		}

        private bool LandmassAtBottomEdge
		{
			get
            {
				// JW: Essentially the same as above. The reverse should affect 
				// the iterator only (i.e. shouldn't cause the array itself to be
				// reversed.  
				return _elevation.Nodes.Reverse()
								   	   .Take((int)_elevation.Width)
                                       .Any(v => v >= SeaLevel);
			}
		}

        private bool LandmassAtLeftEdge
		{
			get
			{
				for (uint i = 0; i < _elevation.Area; i += _elevation.Width)
				{
                    if (_elevation.Nodes[i] >= SeaLevel)
					{
						return true;
					}
				}

				return false;
			}
		}

        private bool LandmassAtRightEdge
		{
			get
			{
				for (uint i = _elevation.Width - 1; i < _elevation.Area; i += _elevation.Width)
				{
                    if (_elevation.Nodes[i] >= SeaLevel)
					{
						return true;
					}
				}

				return false;
			}
		}

        private double LandMassProportion
		{
			get
			{
                return Convert.ToDouble(_elevation.Nodes.Count(v => v >= SeaLevel)) / _elevation.Area;
			}
		}

		public void WorldCalculateDirectSolarRadiation(double orbit)
		{
			Debug.Assert(orbit >= 0.0, "orbit must be >= 0.0");
			Debug.Assert(orbit <= 1.0, "orbit must be >= 1.0");
			Debug.Assert(Planet != null, "WorldDefinePlanet first");
			Debug.Assert(Season != null, "WorldDefineSeasons first");
			Debug.Assert(Area != null, "WorldDefineArea first");

			// JW: This is essentially a wrapper function, right?  Check to see if there's anything missed
			// out.  If not, bite the bullet and merge these two functions together.  
			PlanetCalculateDirectSolarRadiation
			(
				ref _sunlight,
				orbit,
				Season.NorthernSolstice,
                Season.AxialTilt,
                Planet.Radius,
                Planet.DistanceFromSun,
                Planet.SolarLuminosity,
				Area.Center,
                Area.Dimension.Y
			);
		}

		/* Model the direct solar radiation over the map at any given time (direct i.e.
		   sunshine). This EXCLUDES diffuse radiation (e.g. atmospheric scattering)
		   as this depends on current local cloud formation.
   
			With reference to:
    
			1. W. B. Stine and M. Geyer, "The Sun's Energy" in
			   *Power From The Sun*, (Online), 2001.
			   http://www.powerfromthesun.net/Book/chapter02/chapter02.html
		*/
		private void PlanetCalculateDirectSolarRadiation(
			ref Doubles2D buffer,
			double orbit,				// yearly orbit normalised 0.0 to 1.0
			double northern_solstace,	// point in orbit where this occurs (Earth is at 0.222)
			double axial_tilt,			// degrees - severity of seasons (-180 to 180; Earth is 23.5)
			double planet_radius,		// in metres
			double distance_from_sun,	// in astronomical units e.g. 1.0 AU for Earth
			double solar_luminosity,	// 1.0 for our Sun ~= 3.846 × 10^26 Watts
			GeoCoordinate map_center,	// see wgen/geocoordinates.h
			double mapsize)				// kilometres between Northern- and Southern-most points
		{
			// Denormalise orbit to a specific day
			double day = orbit;

			// Denormalise to SI units (metres, watts, etc).
			double si_solar_luminosity = solar_luminosity * 3.846 * Math.Pow(10.0, 26.0);
			double si_distance_from_sun = distance_from_sun * 149600000000.0;
			//double si_planet_surface_area = (4.0 * PI * si_planet_radius * si_planet_radius);
			double si_surface_distance_from_sun = si_distance_from_sun - planet_radius;

			// Radiation on an imaginary surface at the planet's edge, perpendicular
			// to the direction of rays from the sun, using Inverse Square Law.
			double incident_radiance = si_solar_luminosity /
				(4.0 * Math.PI * si_surface_distance_from_sun * si_surface_distance_from_sun);

			for (uint y = 0; y < buffer.Height; ++y)
			{
				// latitude of a point
				double yrange = Convert.ToDouble(y) / buffer.Height;	// 0.0 to 1.0
				double yoffset = (yrange - 0.5) * mapsize;

				GeoCoordinate ypoint = GeoCoordinateEx.Translate(map_center, planet_radius, yoffset, 180.0);

				// Radiation at a point
				double phi = ypoint.Latitude.ToRadians();
				double delta = Math.Asin(
					Math.Sin(axial_tilt.ToRadians()) *
					Math.Sin(2 * Math.PI * (day - northern_solstace))
				);

				double angle = (90.0).ToRadians() - phi + delta;
				double point_radiance = incident_radiance * Math.Sin(angle);

				// JW: Possibly add a yield enumerator here to simplify the 
				// "calculate then apply to one row at a time" bit?
				for (uint x = 0; x < buffer.Width; ++x)
				{
					buffer.Nodes[(y * buffer.Width) + x] = point_radiance;
				}
			}

			buffer.NormaliseMaximum();

			Console.WriteLine("Orbit {0:0.00}/1.0 radiance(W / m ^ 2): low={1:0.00} high={2:0.00} (incident={3:0.00})", 
				day, buffer.Minimum, buffer.Maximum, incident_radiance);
		}
	}
}
