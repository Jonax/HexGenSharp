using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ImageMagick;
using System.Diagnostics;

namespace HexgenSharp2014
{
    public partial class World
    {
        public void RenderElevationRaw(World world, string path)
        {
            byte[] pixels = world._elevation.Nodes
                                 .SelectMany(h => new byte[] { Convert.ToByte(255 * h), Convert.ToByte(255 * h), Convert.ToByte(255 * h) })
                                 .ToArray();

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(pixels, new MagickReadSettings()
            {
                Width = (int)world._elevation.Width,
                Height = (int)world._elevation.Height,
                Format = MagickFormat.Rgb
            }))
            {
                image.Format = MagickFormat.Png32;

                image.Write(path);
            }
        }

        public void RenderElevationQuick(World world, string path)
        {
            byte[] pixels = world._elevation.Nodes
                                 .SelectMany(h =>
                                 {
                                     if (h < World.SEA_LEVEL)
                                     {
                                        return new byte[]
                                        {
                                             Convert.ToByte(32.0 + (32.0 * h)),
                                             Convert.ToByte(64.0 + (255.0 * h)),
                                             Convert.ToByte(128.0 + (128.0 * h))
                                        };
                                     }
                                     else
                                     {
                                         return new byte[]
                                         {
                                             Convert.ToByte(64.0 + (32.0 * h)),
                                             Convert.ToByte(128.0 + (64.0 * h)),
                                             64
                                         };
                                     }
                                 })
                                 .ToArray();

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(pixels, new MagickReadSettings()
                                                               {
                                                                   Width = (int)world._elevation.Width,
                                                                   Height = (int)world._elevation.Height,
                                                                   Format = MagickFormat.Rgb
                                                               }))
            {
                image.Write(path);
            }
        }

        public void RenderSunlightRaw(World world, string path)
        {
            byte[] pixels = world._sunlight.Nodes
                                 .SelectMany(s => new byte[] { Convert.ToByte(255 * s), Convert.ToByte(255 * s), Convert.ToByte(255 * s) })
                                 .ToArray();

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(pixels, new MagickReadSettings()
            {
                Width = (int)world._sunlight.Width,
                Height = (int)world._sunlight.Height,
                Format = MagickFormat.Rgb
            }))
            {
                image.Write(path);
            }
        }

        public void RenderSunlightQuick(World world, string path)
        {
            byte[] pixels = world._sunlight.Nodes
                                 .Zip(world._elevation.Nodes, (s, e) =>
                                 {
                                     if (e < World.SEA_LEVEL)
                                     {
                                         return new byte[]
                                         {
                                             32, 64, 128
                                         };
                                     }
                                     else
                                     {
                                         return new byte[]
                                         {
                                             Convert.ToByte(255.0 * s),
                                             Convert.ToByte(128.0 * s),
                                             Convert.ToByte(64.0 * s)
                                         };
                                     }
                                 })
                                 .SelectMany(h => h)
                                 .ToArray();

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(pixels, new MagickReadSettings()
                                                        {
                                                            Width = (int)world._elevation.Width,
                                                            Height = (int)world._elevation.Height,
                                                            Format = MagickFormat.Rgb
                                                        }))
            {
                image.Write(path);
            }
        }
    }
}