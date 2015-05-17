using System;

namespace HexgenSharp2014
{
	public interface IMask
	{
		double Apply(double x, double y, double w, double h);
	}

    public class MaskBase : IMask
    {
		public virtual double Apply(double x, double y, double w, double h)
		{
			return 1.0;
		}
    }

	public class CircleGradiantMask : MaskBase, IMask
	{
		public override double Apply(double x, double y, double w, double h)
		{
			/* Distance from center */
			double dx = (w * 0.5) - x;
			double dy = (h * 0.5) - y;

			/* Max radius for spherical falloff */
			double maxr = w * 0.5;

			/* Distance of point from center (basic pythag) */
			double r = Math.Sqrt((dx * dx) + (dy * dy));

			if (r > maxr)
			{
				return 0.0;
			}

			double scale = 1.0 - (r / maxr);

			return scale; // 0.0 to 1.0, where 1.0 is opaque
		}
	}

	public class CircleInverseGradiantMask : MaskBase, IMask
	{
		public override double Apply(double x, double y, double w, double h)
		{
			/* Distance from center */
			double dx = (w * 0.5) - x;
			double dy = (h * 0.5) - y;

			/* Max radius for spherical falloff */
			double maxr = w * 0.5;

			/* Distance of point from center (basic pythag) */
			double r = Math.Sqrt((dx * dx) + (dy * dy));

			double scale = (r / maxr);

			// 0.0 to 1.0, where 1.0 is opaque
			return Math.Min(1.0, scale);
		}
	}
}