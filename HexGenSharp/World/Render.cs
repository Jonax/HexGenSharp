using System;
using System.Collections.Generic;
using System.Linq;

using ImageMagick;

namespace HexGenSharp
{
    public partial class World
    {
        private static readonly MagickColor MASK_COLOUR;
        
        static World()
        {
            MASK_COLOUR = new MagickColor(255, 0, 255);
        }

        public void RenderElevation(string path, bool raw = false, int renderWidth = 0)
        {
            IEnumerable<byte> pixels;
            if (raw)
            {
                pixels = _elevation.Nodes
                                   .SelectMany(h => new byte[] { Convert.ToByte(255 * h), Convert.ToByte(255 * h), Convert.ToByte(255 * h) });
            }
            else
            {
                pixels = _elevation.Nodes
                                   .SelectMany(h =>
                                   {
                                       return (h < SeaLevel)
                                              ? new byte[] {
                                                  Convert.ToByte(32.0 + (32.0 * h)),
                                                  Convert.ToByte(64.0 + (255.0 * h)),
                                                  Convert.ToByte(128.0 + (128.0 * h))
                                              }
                                              : new byte[] {
                                                  Convert.ToByte(64.0 + (32.0 * h)),
                                                  Convert.ToByte(128.0 + (64.0 * h)),
                                                  64
                                              };
                                   });
            }

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(MASK_COLOUR, (int)_elevation.Width, (int)_elevation.Height))
            {
                image.GetPixels().Set(pixels.ToArray());

                image.Format = MagickFormat.Png32;

                if (renderWidth > 0)
                {
                    int newHeight = Convert.ToInt32(Convert.ToDouble(image.Height) / image.Width * renderWidth);

                    image.Resize(renderWidth, newHeight);
                }

                image.Write(path);
            }
        }

        public void RenderSunlight(string path, bool raw = false, int renderWidth = 0)
        {
            IEnumerable<byte> pixels;
            if (raw)
            {
                pixels = _sunlight.Nodes
                                  .SelectMany(s => new byte[] { Convert.ToByte(255 * s), Convert.ToByte(255 * s), Convert.ToByte(255 * s) });
            }
            else
            {
                pixels = _sunlight.Nodes
                                  .Zip(_elevation.Nodes, (s, e) =>
                                  {
                                      return (e < SeaLevel)
                                             ? new byte[] { 32, 64, 128 }
                                             : new byte[] {
                                                 Convert.ToByte(255.0 * s),
                                                 Convert.ToByte(128.0 * s),
                                                 Convert.ToByte(64.0 * s)
                                             };
                                  })
                                  .SelectMany(h => h);
            }

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(MASK_COLOUR, (int)_sunlight.Width, (int)_sunlight.Height))
            {
                image.GetPixels().Set(pixels.ToArray());

                image.Format = MagickFormat.Png32;

                if (renderWidth > 0)
                {
                    int newHeight = Convert.ToInt32(Convert.ToDouble(image.Height) / image.Width * renderWidth);

                    image.Resize(renderWidth, newHeight);
                }

                image.Write(path);
            }
        }
    }
}