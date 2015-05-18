namespace HexGenSharp.Extensions
{
	static class MathEx
	{
		public static double ToRadians(this double deg)
		{
			return deg * System.Math.PI / 180;
		}

		// JW: Move this to an extension in the future.  
		public static double ToDegrees(this double rad)
		{
			return rad * 180 / System.Math.PI;
		}
	}
}
