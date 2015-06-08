using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MathNet.Numerics.LinearAlgebra.Double;

namespace HexGenSharp
{
    public class Windcell
    {
        // "Intrinsic"
        public double mass;
        double temperature;
        double moisture;
        public double altitude;
        public DenseVector dimensionX;
        public DenseVector dimension; // of any cell
        public DenseVector velocity;

        // Derived
        double volume;
        public double weight;
        public DenseVector dimension_reciprocal;

        // Forces due to pressure
        public double force_up;
        public double force_down; // differs due to gravity

        public void Init(
            double altitude,     // m
            double mass,         // kg
            double temperature,  // K
            double moisture,     // kg
            Vector dimension)  // m*m*m
        {
            Debug.Assert(altitude >= 0, "altitude must be positive");
            Debug.Assert(mass >= 0, "mass must be positive");
            Debug.Assert(temperature >= 0, "temperature must be positive (Kelvin)");
            Debug.Assert(moisture >= 0, "moisture must be positive");

            Debug.Assert(dimension.At(0) >= 0, "dimension.x must be positive");
            Debug.Assert(dimension.At(1) >= 0, "dimension.y must be positive");
            Debug.Assert(dimension.At(2) >= 0, "dimension.z must be positive");

            this.altitude = altitude;
            this.mass = mass;
            this.temperature = temperature;
            this.moisture = moisture;
            this.dimension = dimension as DenseVector;

            this.volume = this.dimension[0] * this.dimension[1] * this.dimension[2];

            this.velocity = DenseVector.Create(3, 0.0);

            // JW - Why is there no straight V3D/V3D divisor?
            this.dimension_reciprocal = (DenseVector.Create(3, 1.0) / this.dimension) as DenseVector;
        }

        public double Density
        {
            get { return mass / volume; }
        }

        public double Pressure
        {
            get
            {
                // http://www.st-andrews.ac.uk/~dib2/climate/pressure.html
                const double R_specific = 287.0; // should change based on moisture content

                return Density * temperature * R_specific;
            }
        }

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
    };
}
