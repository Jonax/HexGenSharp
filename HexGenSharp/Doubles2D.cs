using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using MathNet.Numerics.LinearAlgebra.Double;

namespace HexGenSharp
{
	public class Doubles2D
	{
		private Matrix _nodes;
		private Size _size;

		public double[] Nodes
		{
			get { return _nodes.Enumerate().ToArray(); }
		}

		// JW - Are these used outside of the Doubles2D class?
		public double Minimum { get; private set; }
		public double Maximum { get; private set; }

		public uint Width { get; private set; }
        public uint Height { get; private set; }

		public uint Area
		{
			get { return Width * Height; }
		}

        public Doubles2D(uint width, uint height)
        {
            Width = width;
            Height = height;

            // JW: Will this definitely work if the property is grabbed from within the constructor?
            _nodes = Matrix.Build.Dense((int)width, (int)height, 0.0) as Matrix;
        }

        /// <summary>
        /// Normalise all values between 0.0 & 1.0.  
        /// </summary>
        public void Normalise()
        {
            IEnumerable<double> values = _nodes.Enumerate();

            double min = values.Min();
            double difference = values.Max() - min;

            _nodes.MapInplace(v => (difference == 0) ? 0 : (v - min) / difference);
        }

        /// <summary>
        /// Normalise all values between 0.0 & 1.0, but maintaining the full range.  
        /// </summary>
        public void NormaliseMaximum()
        {
            _nodes.CoerceZero(v => v < 0.0);

            IEnumerable<double> values = _nodes.Enumerate();
            Minimum = values.Min();
            Maximum = values.Max();

            _nodes.MapInplace(v => v / Maximum);
        }

        /// <summary>
        /// Clamps all values below the minimum limit to itself.  
        /// </summary>
        /// <param name="minimum">The minimum limit for values.</param>
        public void ClampFloorTo(double minimum)
        {
            _nodes.MapInplace(v => Math.Max(v, minimum));
        }

        /// <summary>
        /// Clamps all values above the maximum limit to itself.  
        /// </summary>
        /// <param name="minimum">The maximum limit for values.</param>
        public void ClampCeilingTo(double maximum)
        {
            _nodes.MapInplace(v => Math.Min(v, maximum));
        }

        /// <summary>
        /// Multiplies each value by itself.
        /// </summary>
        public void Square()
        {
            _nodes.MapInplace(v => v * v);
        }
    }
}
