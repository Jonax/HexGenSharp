using System.Numerics;
using System.Diagnostics;
using MathNet.Numerics.LinearAlgebra.Double;

namespace HexGenSharp
{
    public class Windcell
    {
        // "Intrinsic"
        public double Temperature { get; set; }
        public double Altitude { get; set; }
        public DenseVector Dimension { get; set; } // of any cell
        public DenseVector Velocity { get; set; }

        private double mass;
        public double Mass
        {
            get { return mass; }
            set
            {
                if (mass != value)
                {
                    mass = value;
                    Debug.Assert(mass >= 0);
                }
            }
        }

        // Derived
        public double Weight { get; set; }

        // Forces due to pressure
        public double UpwardForce { get; set; }
        public double DownwardForce { get; set; } // differs due to gravity

        public void Init(
            double altitude,     // m
            double mass,         // kg
            double temperature,  // K
            Vector<double> dimension)  // m*m*m
        {
            Debug.Assert(altitude >= 0, "altitude must be positive");
            Debug.Assert(mass >= 0, "mass must be positive");
            Debug.Assert(temperature >= 0, "temperature must be positive (Kelvin)");

            Debug.Assert(dimension[0] >= 0, "dimension.x must be positive");
            Debug.Assert(dimension[1] >= 0, "dimension.y must be positive");
            Debug.Assert(dimension[2] >= 0, "dimension.z must be positive");

            Dimension = DenseVector.Create(3, 0.0);
            Velocity = DenseVector.Create(3, 0.0);

            Temperature = temperature;
            Altitude = altitude;
            Mass = mass;
        }

        public double Volume
        {
            get { return Dimension.At(0) * Dimension.At(1) * Dimension.At(2); }
        }

        public double Density
        {
            get { return mass / Volume; }
        }

        public double Pressure
        {
            get
            {
                // http://www.st-andrews.ac.uk/~dib2/climate/pressure.html
                const double R_SPECIFIC = 287.0; // should change based on moisture content

                return Density * Temperature * R_SPECIFIC;
            }
        }
    };
}
