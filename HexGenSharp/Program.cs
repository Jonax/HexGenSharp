using MathNet.Spatial.Euclidean;
using System;
using System.Data.HashFunction;
using System.Device.Location;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace HexgenSharp2014
{
    class Program
    {
        private const double NORTHERN_SOLSTICE_EARTH = 0.222;
        private const double SEASONAL_TILT_EARTH = 23.5;

        /* Some "representative" geocoordinates based on major cities */
        private static readonly GeoCoordinate GEOCOORDINATE_UK = new GeoCoordinate(51.50021, -0.115958);
        private static readonly GeoCoordinate GEOCOORDINATE_RUSSIA = new GeoCoordinate(55.75, 37.616667);
        private static readonly GeoCoordinate GEOCOORDINATE_USA = new GeoCoordinate(38.895111, -77.036667);
        private static readonly GeoCoordinate GEOCOORDINATE_AUSTRALIA = new GeoCoordinate(-35.308, 149.1245);
        private static readonly GeoCoordinate GEOCOORDINATE_BRAZIL = new GeoCoordinate(-15.783333, -47.866667);
        private static readonly GeoCoordinate GEOCOORDINATE_SOUTH_AFRICA = new GeoCoordinate(-26.204444, 28.045556);
        private static readonly GeoCoordinate GEOCOORDINATE_CHINA = new GeoCoordinate(39.916667, 116.383333);
        private static readonly GeoCoordinate GEOCOORDINATE_GREECE = new GeoCoordinate(37.966667, 23.716667);
        private static readonly GeoCoordinate GEOCOORDINATE_ITALY = new GeoCoordinate(41.9, 12.483333);
        private static readonly GeoCoordinate GEOCOORDINATE_ICELAND = new GeoCoordinate(64.133333, -21.933333);
        private static readonly GeoCoordinate GEOCOORDINATE_EGYPT = new GeoCoordinate(30.033333, 31.216667);
        private static readonly GeoCoordinate GEOCOORDINATE_KAZAKHSTAN = new GeoCoordinate(51.166667, 71.416667);
        private static readonly GeoCoordinate GEOCOORDINATE_INDIA = new GeoCoordinate(28.613333, 77.208333);

        // Sample run
        static void Main(string[] args)
        {
            uint timestamp = Convert.ToUInt32((DateTime.UtcNow - new DateTime(1970, 1, 1)).TotalSeconds);
            MurmurHash3 murmur = new MurmurHash3(seed: timestamp, hashSize: 128);

            int splitPages = 512 / (murmur.HashSize / 8);
            Debug.Assert(splitPages * murmur.HashSize / 8 == 512);

            const uint WIDTH = 32;
            const uint HEIGHT = 16;

            MemoryStream stream = new MemoryStream(512);
            foreach (byte[] page in Enumerable.Range(0, splitPages)
                                              .Select(p => murmur.ComputeHash(p)))
            {
                stream.Write(page, 0, page.Length);
            }

            SimplexNoise noiseGen = new SimplexNoise(stream.GetBuffer());

            World testWorld = new World(new System.Drawing.Size(512, 512), noiseGen, new CircleGradiantMask());
            testWorld.DefinePlanet(
                radius: 6371000.0, // radius
                gravity: 9.81, // g
                distance_sun: 1.0, // distance from sun in AU
                solar_luminosity: 1.0 // relative solar luminosity
            );

            testWorld.DefineSeasons(
                SEASONAL_TILT_EARTH,
                NORTHERN_SOLSTICE_EARTH
            );

            testWorld.DefineArea(
                GEOCOORDINATE_UK,
                new Vector3D(1000 * 1000.0, 1000 * 1000.0, 1350.0) // UK island size
            );

            WindSim windSim = new WindSim();
            windSim.Init(testWorld, 4, 4, 24);
            windSim.RunWindSim(100);

			// JW - Certainly has to be a better way of handling this.  Move it to inside World?
			while (true)
			{
				testWorld.GenerateHeightmap(1.5, 0.25);

				if (testWorld.LandmassAtTopEdge)
				{
					Console.WriteLine("REJECT heightmap: landmass at top edge");
					continue;
				}

				if (testWorld.LandmassAtBottomEdge)
				{
					Console.WriteLine("REJECT heightmap: landmass at bottom edge");
					continue;
				}

				if (testWorld.LandmassAtLeftEdge)
				{
					Console.WriteLine("REJECT heightmap: landmass at left edge");
					continue;
				}

				if (testWorld.LandmassAtRightEdge)
				{
					Console.WriteLine("REJECT heightmap: landmass at right edge");
					continue;
				}

				if (testWorld.LandMassProportion < 0.10)	// TODO user limit
				{
					Console.WriteLine("REJECT heightmap: landmass proportion too low");
					continue;
				}

				if (testWorld.LandMassProportion > 1.00)	// TODO user limit
				{
					Console.WriteLine("REJECT heightmap: landmass proportion too high");
					continue;
				}

				break;
			}

			testWorld.RenderElevationRaw(testWorld, "test_elevation_raw.png");
			testWorld.RenderElevationQuick(testWorld, "test_elevation_quick.png");
			testWorld.RenderSunlightRaw(testWorld, "test_sunlight_raw.png");
			testWorld.RenderSunlightQuick(testWorld, "test_sunlight_quick.png");
		}
    }
}
