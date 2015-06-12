using MathNet.Numerics.LinearAlgebra.Double;
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
            JObject config = JObject.Parse(File.ReadAllText("config.json"));

            Dictionary<string, GeoCoordinate> availableLocations = config["available_locations"].ToDictionary(kv => (kv as JProperty).Name,
                                                                                                            kv =>
                                                                                                            {
                                                                                                                double[] coords = ((kv as JProperty).Value as JArray).Select(v => Convert.ToDouble(v)).ToArray();
                                                                                                                
                                                                                                                return new GeoCoordinate(coords[0], coords[1]);
                                                                                                            });

            byte[] data = new byte[512];

            uint timestamp = Convert.ToUInt32(DateTime.UtcNow.ToFileTimeUtc() % uint.MaxValue);
            Random rng = new Random((int)timestamp);

            //MurmurHash3 murmur = new MurmurHash3(seed: timestamp, hashSize: 128);
            SimplexNoise noiseGen = new SimplexNoise(data);

            World testWorld = new World(4800, 2700, noiseGen, mask: new MaskBase());
            testWorld.SeaProportion = config.Value<double>("sea_proportion");
            testWorld.SeaLevel = config.Value<double>("sea_level");
            testWorld.Planet = new World.PlanetConfig
            {
                Radius = Convert.ToDouble(config["planet"]["radius"]),     // in metres e.g. 6371000.0
                Gravity = Convert.ToDouble(config["planet"]["gravity"]),   // gravity at surface, in e.g. 9.81 m/s^-2
                DistanceFromSun = Convert.ToDouble(config["planet"]["distance_from_sun"]),  // in astronomical units e.g. 1.0 AU for Earth
                SolarLuminosity = Convert.ToDouble(config["planet"]["solar_luminosity"])  // normalised relative to our sun. 1.0 => 3.846×10^26 Watts
            };
            testWorld.Area = new World.AreaConfig
            {
                Center = availableLocations["UK"],
                Dimension = new DenseVector(config["area"]["dimension"].Children()
                                                                           .Select(jt => Convert.ToDouble(jt))
                                                                           .ToArray())
            };
            testWorld.Season = new World.SeasonConfig
            {
                AxialTilt = Convert.ToDouble(config["season"]["seasonal_tile_earth"]),    // degrees - severity of seasons (-180 to 180; Earth is 23.5)
                NorthernSolstice = Convert.ToDouble(config["season"]["northern_solstice_earth"])  // point in orbit (0.0 to 1.0) where this occurs (Earth is at 0.222)
            };

            int numAttempts = 5;
            for (int i = 0; i < numAttempts; ++i)
            {
                Console.WriteLine(i);

                rng.NextBytes(noiseGen.Perm);

                //data = murmur.ComputeHash(data);

                WindSim windSim = new WindSim(testWorld, length: 4, width: 4, height: 24);
                windSim.Run(100);

                if (!Directory.Exists("results"))
                {
                    Directory.CreateDirectory("results");
                }

                if (testWorld.GenerateHeightmap(
                    energy: 1.5,
                    turbulance: 0.25))
                {
                    //testWorld.RenderElevation(String.Format("results/test_{0}_elevation_raw.png", i), raw: true);
                    testWorld.RenderElevation(String.Format("results/test_{0}_elevation.png", i), renderWidth: 1920);

                    for (int month = 0; month < 12; ++month)
                    {
                        testWorld.WorldCalculateDirectSolarRadiation(Convert.ToDouble(month) / 12);

                        //testWorld.RenderSunlight(String.Format("results/test_{0}_sunlight_raw-{1}.png", i, month), raw: true);
                        testWorld.RenderSunlight(String.Format("results/test_{0}_sunlight-{1}.png", i, month), renderWidth: 1920);
                    }
                }
            }
        }
    }
}
