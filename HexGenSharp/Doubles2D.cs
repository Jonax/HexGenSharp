using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;

namespace HexgenSharp2014
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

        public void ZeroFill()
        {
            Array.Clear(_nodes, 0, _nodes.Length);
        }

        // normalise all values between 0.0 & 1.0
        public void Normalise()
        {
            double min = _nodes.Min();
            double max = _nodes.Max();

            double diff = max - min;

            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] -= min;
                _nodes[i] /= diff;
            }
        }

        // normalise all values between 0.0 & 1.0 but maintaining the full range
        public void NormaliseMaximum()
        {
            double min = Math.Max(_nodes.Min(), 0);
            double max = _nodes.Max();

            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] /= max;
            }

            Minimum = min;
            Maximum = max;
        }

        public void ClampFloorTo(double min, double to)
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                if (_nodes[i] < min)
                {
                    _nodes[i] = to;
                }
            }
        }

        public void ClampCeilingTo(double max, double to)
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                if (_nodes[i] > max)
                {
                    _nodes[i] = to;
                }
            }
        }

        public void Square()
        {
            for (int i = 0; i < _nodes.Length; ++i)
            {
                _nodes[i] *= _nodes[i];
            }
        }

        double Average(Rectangle rect)
        {
            double avg = 0;

            for (int i = rect.Y * rect.Height; i < rect.Y * rect.Height + rect.Height; ++i)
            {
                for (int j = rect.X * rect.Width; j < rect.X * rect.Width + rect.Width; ++j)
                {
                    avg += _nodes[(i * _size.Width) + j];
                }
            }

            return avg / (rect.Width * rect.Height);
        }

		// JW - Not used anywhere?
        static void Downsample(Doubles2D dest, Doubles2D src)
        {
            Debug.Assert(dest._size.Width <= src._size.Width, "src_width_too_small");
            Debug.Assert(dest._size.Height <= src._size.Height, "src_height_too_small");

            int xs = src._size.Width / dest._size.Width;
            int ys = src._size.Height / dest._size.Height;

            int i = 0;
            for (int y = 0; y < dest._size.Width; y++)
            {
                for (int x = 0; x < dest._size.Height; x++)
                {
                    dest._nodes[i++] = src.Average(new Rectangle(x, y, xs, ys));
                }
            }

            dest.Maximum = src.Maximum;
            dest.Minimum = src.Minimum;
        }
    }
}
