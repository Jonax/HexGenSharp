using MathNet.Spatial.Euclidean;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HexGenSharp
{
    public class Windcell
    {
        // "Intrinsic"
        public double mass;
        double temperature;
        double moisture;
        public double altitude;
        public Vector3D dimension; // of any cell
        public Vector3D velocity;

        // Derived
        double volume;
        public double weight;
        public Vector3D dimension_reciprocal;

        // Forces due to pressure
        public double force_up;
        public double force_down; // differs due to gravity

        public void Init(
            double altitude,     // m
            double mass,         // kg
            double temperature,  // K
            double moisture,     // kg
            Vector3D dimension)  // m*m*m
        {
            Debug.Assert(altitude >= 0, "altitude must be positive");
            Debug.Assert(mass >= 0, "mass must be positive");
            Debug.Assert(temperature >= 0, "temperature must be positive (Kelvin)");
            Debug.Assert(moisture >= 0, "moisture must be positive");

            Debug.Assert(dimension.X >= 0, "dimension.x must be positive");
            Debug.Assert(dimension.Y >= 0, "dimension.y must be positive");
            Debug.Assert(dimension.Z >= 0, "dimension.z must be positive");

            this.altitude = altitude;
            this.mass = mass;
            this.temperature = temperature;
            this.moisture = moisture;
            this.dimension = dimension;

            this.volume = this.dimension.X * this.dimension.Y * this.dimension.Z;

            this.velocity = new Vector3D(0.0, 0.0, 0.0);

            // JW - Why is there no straight V3D/V3D divisor?
            this.dimension_reciprocal = new Vector3D(
                1.0 / this.dimension.X,
                1.0 / this.dimension.Y,
                1.0 / this.dimension.Z);
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

        public void ChangeMass(double amount)
        {
            mass += amount;
            Debug.Assert(mass >= 0);
        }
    };
}
