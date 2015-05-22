using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;

namespace HexGenSharp
{
	public class Doubles2D
	{
		private double[] _nodes;
		private Size _size;

		public double[] Nodes
		{
			get { return _nodes; }
		}

		// JW - Are these used outside of the Doubles2D class?
		public double Minimum { get; private set; }
		public double Maximum { get; private set; }

		public uint Width
		{
			get { return (uint)_size.Width; }
		}

		public uint Height
		{
			get { return (uint)_size.Height; }
		}

		public uint Area
		{
			get { return (uint)(_size.Width * _size.Height); }
		}

        public Doubles2D(Size s)
        {
            _size = s;

			// JW: Will this definitely work if the property is grabbed from within the constructor?
			_nodes = new double[Area];
        }

        /// <summary>
        /// Normalise all values between 0.0 & 1.0.  
        /// </summary>
        public void Normalise()
        {
            double min = _nodes.Min();

            double difference = _nodes.Max() - min;

            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] -= min;
                _nodes[i] /= difference;
            }
        }

        /// <summary>
        /// Normalise all values between 0.0 & 1.0, but maintaining the full range.  
        /// </summary>
        public void NormaliseMaximum()
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                if (_nodes[i] < 0.0)
                {
                    _nodes[i] = 0.0;
                }
            }

            double min = _nodes.Min();
            double max = _nodes.Max();

            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] /= max;
            }

            Minimum = min;
            Maximum = max;
        }

        /// <summary>
        /// Clamps all values below the minimum limit to itself.  
        /// </summary>
        /// <param name="minimum">The minimum limit for values.</param>
        public void ClampFloorTo(double minimum)
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                if (_nodes[i] < minimum)
                {
                    _nodes[i] = minimum;
                }
            }
        }

        /// <summary>
        /// Clamps all values above the maximum limit to itself.  
        /// </summary>
        /// <param name="minimum">The maximum limit for values.</param>
        public void ClampCeilingTo(double maximum)
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                if (_nodes[i] > maximum)
                {
                    _nodes[i] = maximum;
                }
            }
        }

        /// <summary>
        /// Multiplies each value by itself.
        /// </summary>
        public void Square()
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] *= _nodes[i];
            }
        }
    }
}
