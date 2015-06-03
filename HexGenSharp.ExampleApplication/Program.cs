using MathNet.Spatial.Euclidean;
using System;
using System.Data.HashFunction;
using System.Device.Location;
using System.Diagnostics;
using System.IO;
using System.Linq;
using HexGenSharp;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Drawing;

namespace HexGenSharp.ExampleApplication
{
    class Program
    {
        private const double NORTHERN_SOLSTICE_EARTH = 0.222;
        private const double SEASONAL_TILT_EARTH = 23.5;

        // Sample run
        static void Main(string[] args)
        {
            /*
            JObject config = JObject.Parse(File.ReadAllText("config.json"));

            Dictionary<string, GeoCoordinate> availableLocations = config["available_locations"].ToDictionary(kv => (kv as JProperty).Name,
                                                                                                            kv =>
                                                                                                            {
                                                                                                                double[] coords = ((kv as JProperty).Value as JArray).Select(v => Convert.ToDouble(v)).ToArray();
                                                                                                                
                                                                                                                return new GeoCoordinate(coords[0], coords[1]);
                                                                                                            });
            */

            byte[] data = new byte[512];

            uint timestamp = Convert.ToUInt32(DateTime.UtcNow.ToFileTimeUtc() % uint.MaxValue);
            Random rng = new Random((int)timestamp);

            //MurmurHash3 murmur = new MurmurHash3(seed: timestamp, hashSize: 128);
            SimplexNoise noiseGen = new SimplexNoise(data);

            World testWorld = new World(new System.Drawing.Size(512, 512), noiseGen, mask: new CircleGradiantMask())
            {
                SeaProportion = 0.55,
                SeaLevel = 0.15,
                Planet = new World.PlanetConfig
                {
                    Radius = 6371000.0,     // in metres e.g. 6371000.0
                    Gravity = 9.81,         // gravity at surface, in e.g. 9.81 m/s^-2
                    DistanceFromSun = 1.0,  // in astronomical units e.g. 1.0 AU for Earth
                    SolarLuminosity = 1.0,  // normalised relative to our sun. 1.0 => 3.846×10^26 Watts
                },
                Area = new World.AreaConfig
                {
                    Center = new GeoCoordinate(51.50021, -0.115958),
                    Dimension = new Vector3D(1000 * 1000.0, 1000 * 1000.0, 1350.0) // UK island size
                },
                Season = new World.SeasonConfig
                {
                    AxialTilt = SEASONAL_TILT_EARTH,    // degrees - severity of seasons (-180 to 180; Earth is 23.5)
                    NorthernSolstice = NORTHERN_SOLSTICE_EARTH  // point in orbit (0.0 to 1.0) where this occurs (Earth is at 0.222)
                },
            };

            int numAttempts = 5;
            for (int i = 0; i < numAttempts; ++i)
            {
                Console.WriteLine(i);

                rng.NextBytes(noiseGen.Perm);

                //data = murmur.ComputeHash(data);

                WindSim windSim = new WindSim();
                windSim.Init(testWorld, 4, 4, 24);
                windSim.Run(100);

                if (!Directory.Exists("results"))
                {
                    Directory.CreateDirectory("results");
                }

                if (testWorld.GenerateHeightmap(
                    energy: 1.5,
                    turbulance: 0.25))
                {
                    testWorld.RenderElevation(String.Format("results/test_elevation_raw_{0}.png", i), raw: true);
                    testWorld.RenderElevation(String.Format("results/test_elevation_quick_{0}.png", i));

                    for (int month = 0; month < 12; ++month)
                    {
                        testWorld.WorldCalculateDirectSolarRadiation(Convert.ToDouble(month) / 12);

                        testWorld.RenderSunlight(String.Format("results/test_sunlight_raw_{0}-{1}.png", i, month), raw: true);
                        testWorld.RenderSunlight(String.Format("results/test_sunlight_quick_{0}-{1}.png", i, month));
                    }
                }
            }
        }
    }
}
