using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ImageMagick;
using System.Diagnostics;

namespace HexGenSharp
{
    public partial class World
    {
        public void RenderElevation(string path, bool raw = false)
        {
            byte[] pixels;
            if (raw)
            {
                pixels = _elevation.Nodes
                                   .SelectMany(h => new byte[] { Convert.ToByte(255 * h), Convert.ToByte(255 * h), Convert.ToByte(255 * h) })
                                   .ToArray();
            }
            else
            {
                pixels = _elevation.Nodes
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
            }

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(new MagickColor(255, 0, 255), (int)_elevation.Width, (int)_elevation.Height))
            {
                image.GetWritablePixels().Set(pixels);

                image.Format = MagickFormat.Png32;

                image.Write(path);
            }
        }

        public void RenderSunlight(string path, bool raw = false)
        {
            byte[] pixels;
            if (raw)
            {
                pixels = _sunlight.Nodes
                                  .SelectMany(s => new byte[] { Convert.ToByte(255 * s), Convert.ToByte(255 * s), Convert.ToByte(255 * s) })
                                  .ToArray();
            }
            else
            {
                pixels = _sunlight.Nodes
                                  .Zip(_elevation.Nodes, (s, e) =>
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
            }

            // Using RGB so 3 (C version uses RGBA so 4)
            using (MagickImage image = new MagickImage(new MagickColor(255, 0, 255), (int)_sunlight.Width, (int)_sunlight.Height))
            {
                image.GetWritablePixels().Set(pixels);

                image.Format = MagickFormat.Png32;

                image.Write(path);
            }
        }
    }
}