using MathNet.Spatial.Euclidean;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HexGenSharp
{
	public class WindSim
	{
        private const double WINDSIM_HEIGHT = 100000.0; // 100km

        private World world;

        private uint gridLength;    // X
        private uint gridWidth;     // Y
        private uint gridHeight;    // Z

		private uint elements;
		private double height; // metres

		private Windcell[] cell;

		private Doubles2D elevation; // resampled to a smaller resolution
		private Doubles2D sunlight;	 // resampled to a smaller resolution
		private Doubles2D albedo;	 // resampled to a smaller resolution

        public void Init(World w, uint length, uint width, uint height)
        {
            Debug.Assert(w != null, "NULL world pointer");
            Debug.Assert(length > 0, "size.X must be > 0");
            Debug.Assert(width > 0, "size.Y must be > 0");
            Debug.Assert(height > 0, "size.Z must be > 0");

            Size xy = new Size((int)length, (int)width);

            this.world = w;
            this.gridLength = length;
            this.gridWidth= width;
            this.gridHeight = height;
            this.elements = length * width * height;
            this.height = WINDSIM_HEIGHT;

            /* a 3D grid of cells covering the map surface */
            cell = new Windcell[elements];
            Debug.Assert(cell != null, "alloc_sim_cells");

            /* 2D buffers for sampling information from the world map at a lower
             * resolution */
            elevation = new Doubles2D(xy);
            sunlight = new Doubles2D(xy);
            albedo = new Doubles2D(xy);

            Debug.Assert(elevation != null, "Doubles2DInit_elevation");
            Debug.Assert(sunlight != null, "Doubles2DInit_sunlight");
            Debug.Assert(albedo != null, "Doubles2DInit_albedo");
        }

        void InitCells()
        {
            /* Initialises a stack of cells over the simulation, the depth of each cell
             * (I will generally use "depth" to describe height in the Z axis, reserving
             * "height" for describing the Y axis) being smaller at lower altitudes
             * in order to give greater accuracy at the more important area. Heights
             * increase in proportion: 1, 2, 3, ... n.
             * 
             * Each cell is initialised with an approximation so that the simulation
             * converges to realistic point as soon as possible.
             * */
            double altitude = 0;
            double n = (double)gridHeight + 1;

            // for each layer
            for (uint z = 0; z < gridHeight; z++)
            {
                // smaller depths nearer the surface for accuracy
                // using depths of nx for 
                // and x = 2h/((n)(n-1))
                double x = 2.0 * this.height / (n * (n - 1));
                double depth = x * (1.0 + (double)z);
                altitude += depth * 0.5; // midpoint

                // Find width of cell by extending a triangle from planet radius to
                // height of layer.
                double width = TriangleExtendedOpposite
                (
                    world.Radius,
                    world.Dimension.X / Convert.ToDouble(gridLength),
                    altitude
                );

                double height = TriangleExtendedOpposite
                (
                    world.Radius,
                    world.Dimension.Y / Convert.ToDouble(gridWidth),
                    altitude
                );

                // for width and height
                for (uint i = 0; i < gridLength * gridWidth; ++i)
                {
                    // JW: Simplify this later.  
                    cell[(gridLength * gridWidth * z) + i] = new Windcell();

                    // initialise with an approximation
                    cell[(gridLength * gridWidth * z) + i].Init
                    (
                        altitude,
                        0.15 * width * height * depth, // air mass kg
                        273.15, // temperature in Kelvin (0 C)
                        0.0, // moisture kg
                        new Vector3D(width, height, depth) // m*m*m
                    );
                }

                altitude += depth * 0.5; // move beyond midpoint
            }
        }

        public Windcell AtXYZ(uint x, uint y, uint z)
        {
            return cell
            [
                (z * gridWidth * gridLength) +
                (y * gridLength) +
                (x)
            ];
        }

        public Windcell AtZI(uint z, uint i)
        {
            return cell
            [
                (z * gridWidth * gridLength) + i
            ];
        }

        private static double TriangleExtendedOpposite(
            double radius,
            double opposite,
            double extend)
        {
            /* Given a triangle, with an unknown angle A, an opposite side of known
            * length, a, and known adjacent sides, b and c, both of size radius,
            * find the new length of side a in a triangle formed by extending the
            * radius by a known distance.
            * 
            * In other words, we have a triangle formed by approximating an arc from
            * the center of a planet of known radius, for a certain known size
            * on the surface (given by the opposite argument). We want to know how
            * much bigger the triangle at the top of the arc becomes at higher
            * altitudes, given by the extend argument.
            * 
            * This is used to find the width of a simulation cell at higher altitudes
            * from the surface of the planet. This is simply the cosine rule. */

            // Cosine rule: a^2 = b^2 + c^2 - 2bc cosA
            double a = opposite;
            double b = radius;
            double c = radius;
            double cosA = (a * a - b * b - c * c) / (-2.0 * b * c);

            b = radius + extend;
            c = radius + extend;
            a = Math.Sqrt(b * b + c * c - (2.0 * b * c * cosA));

            return a;
        }

        void StepForce(uint z, uint i)
        {
            Windcell cell = AtZI(z, i);

            double re = world.Radius;
            double altitude = cell.altitude;

            // Gravity at altitude by the inverse square law
            // JW: Used a Math.Pow here rather than the usual x*x for convenience.  
            //     Look at replacing when optimising.  
            double gravity = world.Gravity * Math.Pow(re / (re + altitude), 2);

            // Cell weight by F=ma
            cell.weight = cell.mass * gravity;

            // Pressure by surface area
            double pressure = cell.Pressure;
            double area_force_z = pressure * (cell.dimension.X * cell.dimension.Y);
            cell.force_up = area_force_z;
            cell.force_down = area_force_z + cell.weight;
        }


        void StepVelocity(uint z, uint i)
        {
            if (z == 0)
            {
                // assumption: force of air rushing downwards is always met by the
                // surface of the earth, so we start each step at the floor with the
                // cell velocity at zero before acceleration is considered normally.
                Windcell floorcell = AtZI(z, i);
                // JW: Annoying that Vector3D attributes are read-only.  Move to an 
                // alternative with read-write?
                floorcell.velocity = new Vector3D(
                    floorcell.velocity.X,
                    floorcell.velocity.Y,
                    Math.Max(floorcell.velocity.Z, 0.0)
                );

                // steps are evaluated from in order from z to z-1.
                // shere z=0, there is no z-1.
                return;
            }

            Windcell above = AtZI(z, i);
            Windcell below = AtZI(z - 1, i);

            Windcell from, to;

            double force = (above.force_down - below.force_up);

            // JW: General gist; make sure the changes below actually 
            // apply to "from" & "to", and not just apply them to copies.  
            if (force > 0.0)
            {
                from = above; to = below;
            }
            else
            {
                from = below; to = above;
            }

            // Newton's Laws of Motion
            // change in force = change in velocity = acceleration
            // force = mass * acceleration: F=ma, a = F/m
            
            // JW: Annoying that Vector3D attributes are read-only.  Move to an 
            // alternative with read-write?
            from.velocity = from.velocity + new Vector3D(0.0, 0.0, force / from.mass);
            to.velocity = to.velocity + new Vector3D(0.0, 0.0, force / to.mass);
        }

        void StepMass(uint z, uint i)
        {
            // Transfer of mass and momentum due to velocity
            Windcell from = AtZI(z, i);
            double transfer = from.mass * from.velocity.Z * from.dimension_reciprocal.Z;

            if (transfer > 0.0 && z == 0)
                return;

            if (transfer < 0.0 && z == gridHeight - 1)
                return;

            //assert(transfer >= -from.mass);
            //assert(transfer <= from.mass);

            Windcell to;
            if (transfer > 0.0)
            {
                to = AtZI(z - 1, i);
            }
            else
            {
                to = AtZI(z + 1, i);
            }

            // when transferring mass, it is travelling at a velocity
            // so we want to transfer momentum to a cell
            // p = mv
            double momentum1 = Math.Abs(transfer) * from.velocity.Z;
            double momentum2 = to.mass * to.velocity.Z;
            to.velocity = to.velocity + new Vector3D(0.0, 0.0, (momentum1 + momentum2) / to.mass);

            // and an equal force in the opposite direction accordingly
            momentum2 = from.mass * from.velocity.Z;
            from.velocity = from.velocity - new Vector3D(0.0, 0.0, (momentum1 + momentum2) / from.mass);

            from.ChangeMass(-Math.Abs(transfer));
            to.ChangeMass(Math.Abs(transfer));
        }


        public void RunWindSim(int iterations)
        {
            InitCells();

            //iterations = 2501;
            //iterations = 3;
            //iterations = 50;
            iterations = 10001;

            Console.WriteLine("Wind simulation: {0} iterations over {1}x{2}x{3} cells", iterations, gridLength, gridWidth, gridHeight);

            for (int iteration = 0; iteration < iterations; ++iteration)
            {
                if (iteration % 100 == 0)
                {
                    Console.WriteLine("Windsim: {0}/{1}", iteration, iterations - 1);
                }

                for (uint i = 0; i < gridLength * gridWidth; ++i)
                {
                    for (uint z = 0; z < gridHeight; ++z)
                    {
                        // Calculate Forces due to pressure and gravity
                        StepForce(z, i);
                    }

                    for (uint z = 0; z < gridHeight; z++)
                    {
                        // Calculate change in velocity due to forces
                        StepVelocity(z, i);
                    }

                    for (uint z = 0; z < gridHeight; z++)
                    {
                        // Transfer of mass and momentum due to velocity
                        StepMass(z, i);
                    }
                }
            }
        }
    }
}
