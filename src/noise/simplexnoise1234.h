/* SimplexNoise1234, Simplex noise with true analytic
 * derivative in 1D to 4D.
 *
 * Author: Stefan Gustavson, 2003-2005
 * Contact: stegu@itn.liu.se
 *
 * This code was GPL licensed until February 2011.
 * As the original author of this code, I hereby
 * release it into the public domain.
 * Please feel free to use it for whatever you want.
 * Credit is appreciated where appropriate, and I also
 * appreciate being told where this code finds any use,
 * but you may do as you like.
 */
 
 /*
    MODIFICATIONS by BSAG 2013:
    Add "static" keywords where needed
    Add pragma to ignore -Wconversion
    Change float to double
    Add randomise function
 */

/** \file
		\brief Header for "simplexnoise1234.c" for producing Perlin simplex noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin Simplex noise function.
 * It is a stand-alone compilation unit with no external dependencies,
 * highly reusable without source code modifications.
 *
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. Having both versions could be useful.
 */

/** 1D, 2D, 3D and 4D double Perlin simplex noise
 */
#   include "rng/rng.h"

    double snoise1( double x );
    double snoise2( double x, double y );
    double snoise3( double x, double y, double z );
    double snoise4( double x, double y, double z, double w );
    
    // Set permutation table
    void snoise_randomise(RNG *rng);
