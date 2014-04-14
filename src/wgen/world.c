#include "base.h"
#include "wgen/wgen.h"
#include "noise/simplexnoise1234.h"
#include <stdio.h>
#include <math.h> // log

int WorldInit(World *w, Generator *g, size2D size)
{
    if (!w) { X2(bad_arg, "NULL world pointer");}
    if (!g) { X2(bad_arg, "NULL generator pointer");}
    if (size.x == 0) { X2(bad_arg, "bad size"); }
    if (size.y == 0) { X2(bad_arg, "bad size"); }
    
    *w->name = '\0';
    *w->desc = '\0';
    w->generator = g;
    w->seasons   = NULL;
    
    if (!Doubles2DInit(&w->elevation, size)) { X(Doubles2DInit_elevation); }
    if (!Doubles2DInit(&w->sunlight,  size)) { X(Doubles2DInit_sunlight);  }
    
    return 1;
    
    err_Doubles2DInit_sunlight:
        Doubles2DTeardown(&w->elevation);
    err_Doubles2DInit_elevation:
    err_bad_arg:
        return 0;
}


static void WorldApplyNoise
(
    World *w,
    double energy,          // 0.6 to 3.0.  Higher == more islands
    double turbulance       // -0.5 to 1.0. Controls shape and contrast
)
{
    Doubles2D *elevation = &w->elevation;
    
    // longest length
    size_t length = max_size_t(elevation->size.x, elevation->size.y);
    
    /* This function applies multiple octaves of noise in order to generate an
     * interesting heightmap.
     * 
     * We want the smallest octave of noise to have either pixel or sub-pixel
     * interpolation for detail.
     * 
     * The last octave is at 2^(iterations-1)
     * Desired detail: for length/2^(iterations-1) to be <= 1 pixel
     * therefore iterations = log_2(length) + 1, rounded up
     */
    unsigned int iterations = (unsigned int)
        (1.5 + (log((double) length) / log(2.0)));
    
    printf("Octaves of noise for %ux%u : %u\n",
        (unsigned int) length, (unsigned int) length, iterations);
    
    double width  = (double) elevation->size.x;
    double height = (double) elevation->size.y;
    
    /* How chaotic should the first octave of noise be? For a low value we are
     * "zoomed out" and interpolating between two noise values. This will give
     * us a smooth gradient between points. This is good for generating
     * strongly defined continents.
     * 
     * For a high value we are zoomed in and will cross more distinct values.
     * This is good for generating land that is broken up.
     * 
     * Try values from 0.5 to 3.0.
     */
    double scale = (energy / (double) length);
    double scale_r = 1.0 / scale; // reciprocal
    
    /* The first octave is what dominates the shape of the map. Further octaves
     * give finer detail. Therefore if you want a stand-alone continent in the
     * centre of the map, you can set the first octave to be a circular
     * gradient. Or you can use your own image to get a custom shape.
     */
    for (size_t i = 0; i < elevation->elements; i++)
    {
        double y = (double) ((unsigned int) (i / elevation->size.x));
        double x = (double) ((unsigned int) (i % elevation->size.x));
        
        elevation->values[i] = scale_r *
            (w->generator->mask_sampler(w->generator->mask_sampler_rsc,
                                         x, y, width, height));
    }
    
    /* Octaves of noise */
    for (unsigned int i = 1; i < iterations; i++)
    {
        scale *= 2.0;   // double the scale for each octave
        scale_r *= 0.5; // this halves the reciprocal
    
        for (size_t j = 0; j < elevation->elements; j++)
        {
            double y = (double) ((unsigned int) (j / elevation->size.x));
            double x = (double) ((unsigned int) (j % elevation->size.x));
            
            double turb0 = fabs(snoise2(x * scale * 3.0, y * scale * 3.0));
            double turb1 = fabs(snoise2(x * scale * 7.0, y * scale * 7.0));
            double turb = turb0 * turb1 * turbulance;
            
            /* micro trubulance for defined mountains */
            /*if (i == 3)
            {
                turb *= 10.0;
            }
            */
            
            elevation->values[j] += scale_r *
                (snoise2(x * scale, y * scale) + turb);
        }
    }
}


int WorldGenerateHeightmap
(
    World *w,
    double energy,          // 0.6 to 3.0.  Higher == more islands
    double turbulance       // -0.5 to 1.0. Controls shape and contrast
)
{
    turbulance /= energy;
    
    /* Seed the state used by the noise function */
    snoise_randomise(w->generator->rng);
    
    /* Apply several octaves of noise at a specific scale */
    WorldApplyNoise(w, energy, turbulance);
    
    /* Normalise elevation between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    /* Create a sea floor by clamping anything below 0.6 */
    Doubles2DClampFloorTo(&w->elevation, SEA_PROPORTION, SEA_PROPORTION);
    
    /* Normalise elevation again between 0.0 to 1.0 */
    Doubles2DNormalise(&w->elevation);
    
    return 1;
}


int WorldLandmassAtTopEdge(World *w)
{
    double *v = w->elevation.values;
    
    for (size_t x = 0; x < w->elevation.size.x; x++)
    {
        if (*v++ >= SEA_LEVEL) return 1;
    }
    
    return 0;
}


int WorldLandmassAtBottomEdge(World *w)
{
    double *v = w->elevation.values;
    v += (w->elevation.size.x * (w->elevation.size.y - 1));
    
    for (size_t x = 0; x < w->elevation.size.x; x++)
    {
        if (*v++ >= SEA_LEVEL) return 1;
    }
    
    return 0;
}


int WorldLandmassAtLeftEdge(World *w)
{
    double *v = w->elevation.values;
    
    for (size_t y = 0; y < w->elevation.size.y; y++)
    {
        if (*v >= SEA_LEVEL) return 1;
        
        v += w->elevation.size.x;
    }
    
    return 0;
}


int WorldLandmassAtRightEdge(World *w)
{
    double *v = w->elevation.values;
    v += w->elevation.size.x - 1;
    
    for (size_t y = 0; y < w->elevation.size.y; y++)
    {
        if (*v >= SEA_LEVEL) return 1;
        
        v += w->elevation.size.x;
    }
    
    return 0;
}


double WorldLandMassProportion(World *w)
{
    unsigned int count = 0;
    double *v = w->elevation.values;
    
    for (size_t i = 0; i < w->elevation.elements; i++, v++)
    {
        if (*v > SEA_LEVEL) { count++; }
    }
    
    return (((double) count) / ((double) w->elevation.elements));
}


/* Model the direct solar radiation over the map at any given time (direct i.e.
   sunshine). This EXCLUDES diffuse radiation (e.g. atmospheric scattering)
   as this depends on current local cloud formation.
   
    With reference to:
    
    1. W. B. Stine and M. Geyer, "The Sun's Energy" in
       *Power From The Sun*, (Online), 2001.
       http://www.powerfromthesun.net/Book/chapter02/chapter02.html
*/
int WorldCalculateDirectSolarRadiation
(
    Doubles2D *buffer,
    double orbit,               // yearly orbit normalised 0.0 to 1.0
    double northern_solstace,   // point in orbit where this occurs (Earth is at 0.222)
    double axial_tilt,          // degrees - severity of seasons (-180 to 180; Earth is 23.5)
    double planet_radius,       // where 1.0 is the mean radius of the Earth
    double distance_from_sun,   // in astronomical units e.g. 1.0 AU for Earth
    double solar_luminosity,    // 1.0 for our Sun ~= 3.846 × 10^26 Watts
    geocoordinate map_center,   // see wgen/geocoordinates.h
    double mapsize              // kilometres between Northern- and Southern-most points
)
{
#   define PI 3.141592654
    UNUSED(mapsize);
    double *v = buffer->values;
    
    // Denormalise orbit to a specific day
    double day = orbit;
    
    // Denormalise to SI units (metres, watts, etc).
    double si_solar_luminosity  = solar_luminosity * 3.846 * pow(10.0, 26.0);
    double si_distance_from_sun = distance_from_sun * 149600000000.0;
    double si_planet_radius     = planet_radius * 6371000.0;
    //double si_planet_surface_area = (4.0 * PI * si_planet_radius * si_planet_radius);
    double si_surface_distance_from_sun = si_distance_from_sun - si_planet_radius;
    
    if (PI * si_planet_radius < mapsize * 1000.0)
    { W("mapsize is too large for a planet of this radius!"); }
    
    // Radiation on an imaginary surface at the planet's edge, perpendicular
    // to the direction of rays from the sun, using Inverse Square Law.
    double incident_radiance = si_solar_luminosity /
        (4.0 * PI * si_surface_distance_from_sun * si_surface_distance_from_sun);
    
    for (size_t y = 0; y < buffer->size.y; y++)
    {
        // latitude of a point
        double yrange = (double) y / (double) buffer->size.y; // 0.0 to 1.0
        double yoffset = (yrange - 0.5) * mapsize;
        
        geocoordinate ypoint = GeoCoordinateTranslate(map_center,
            planet_radius * 6371, yoffset, 180.0);
        
        // Radiation at a point
        double phi = radians(LatitudeToDouble(GeoCoordinateLatitude(ypoint)));
        double delta = asin(
            sin(radians(axial_tilt)) *
            sin(2 * PI * (day - northern_solstace))
        );
        
        double angle = radians(90.0) - phi + delta;
        double point_radiance = incident_radiance * sin(angle);
        
        for (size_t x = 0; x < buffer->size.x; x++)
        {
            *v++ = point_radiance;
        }
    }
    
    Doubles2DNormaliseMaximum(buffer);
    
    printf
    (
        "Orbit %.1f/1.0 radiance (W/m^2): "
        "low=%.2f "
        "high=%.2f "
        "(incident=%.2f)"
        "\n",
        day,
        buffer->minimum,
        buffer->maximum,
        incident_radiance
    );
    
    return 1;
}


/* WIND:
 * 
 * http://www.srh.noaa.gov/jetstream/synoptic/wind.htm
 * 
 * For day/night cycle (not implemented in hexgen2014: see Hero Extant proper).
 * http://en.wikipedia.org/wiki/Mountain_breeze_and_valley_breeze
 * http://en.wikipedia.org/wiki/Anabatic_wind
 *
 * Related to altitude & pressure: 
 * http://en.wikipedia.org/wiki/Katabatic_wind
 * http://en.wikipedia.org/wiki/Foehn_wind
 * http://en.wikipedia.org/wiki/Rain_shadow
 * http://en.wikipedia.org/wiki/Density_of_air
 * * http://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_atmospheric_pressure_variation
 * 
 * Global:
 * http://en.wikipedia.org/wiki/Intertropical_convergence_zone
 * http://en.wikipedia.org/wiki/Trade_wind
 * http://en.wikipedia.org/wiki/Prevailing_winds
 * http://en.wikipedia.org/wiki/Prevailing_winds#Effect_on_precipitation
 * http://en.wikipedia.org/wiki/Wind_speed
 *
 * Local:
 * http://en.wikipedia.org/wiki/Sea_breeze
 * http://en.wikipedia.org/wiki/Prevailing_winds#Circulation_in_elevated_regions
 * 
 * RAIN:
 * 
 * http://en.wikipedia.org/wiki/Rain#Causes
 * http://en.wikipedia.org/wiki/Dry_season
 * http://en.wikipedia.org/wiki/Tropical_rain_belt
 * 
 * MECHANICS:
 * http://www.st-andrews.ac.uk/~dib2/climate/pressure.html (p = R r T)
 * http://en.wikipedia.org/wiki/Primitive_equations
 * 
 * BIOMES:
 * http://en.wikipedia.org/wiki/K%C3%B6ppen_climate_classification
 * http://en.wikipedia.org/wiki/Holdridge_life_zones
 * 
 */

/*
 * Let's first model the Tropical rain belt across the solar equator.
 * This mechanism is convection.
 * 
 * Convection knowledge:
 * Air is heated by solar radiation, especially along the solar equator.
 * Hot air is less dense than cold air. Hot air rises. Cold air falls.
 * Warmer air is able to retain more moisture than colder air.
 * Moist air is less dense than dry air.
 * When the warmer air is replaced by colder air, the colder air cannot absorb
 *     moisture. The excess moisture coalesces and then falls as rain due to
 *     gravity.
 * 
 * -- So due to tilt of the earth, we have a rain belt that moves to follow the
 *    seasonal solar equator.
 * 
 * Altitude knowledge:
 * Temperature falls with altitude.
 * Air pressure falls linearly with altitude.
 * Air density generally falls with altitude.
 * Rising air cools, cool air increases in density, and condenses causing rain.
 * Density of air is inversely proportional to temperature (cold = dense)
 * This is more pronounced when a mountain forces air to rise quickly.
 * 
 * High density air from a high elevation going down a slope causes
 * Katabatic winds due to acceleration due to gravity.
 * These can be cold and intense. But sinking air warms which means it can
 * hold more water.
 * 
 * -- So we can model the effects of mountains which give interesting wind
 *    speeds, temperatures, and rain shadows.
 * 
 * Simulating air will need require measure of air moisture, air density, air
 * temperature, air speed, and a 3D position.
 * 
 * We will need to model what happens when air collides - friction,
 * elastic collision, etc.
 * 
 * Individual particles do not have density -- only a mass of air as a whole.
 * If we have enough particles, density is implicit (i.e. density = particles
 * divided by volume).
 * 
 * Let's try a simple model: cells across a grid of size 128x128x8
 * 
 * Convection:
 * -- Each cell can be heated/cooled by the land beneath it.
 * -- Each cell can receive moisture from the land beneath it.
 * -- Hot air increases pressure, so pushes out & up, reducing density in the
 *        current cell and increasing density in adjacent and (mainly) above
 *        cells (direction can be implicit due to adjacent cells of equal
 *        pressure all wanting to push up to a low pressure area
 *        instead of adjacently).
 * -- Cold air is denser, so pushes into less dense cells, reducing temperature.
 *        This is due to gravity. Pressure caused by density can be compared
 *        to pressure caused by kinetic energy from gravitational acceleration.
 * -- Cold cells can hold less moisture, so rain sooner and rain colder.
 * -- Raining reduces moisture held in a cell.
 * 
 * The key point is WIND MOVEMENT IS CAUSED BY DIFFERENCES IN PRESSURE
 * 
 * And at the end we want to display:
 * -- Wind speed at the surface cells.
 * -- Air pressure.
 * -- Rainfall quantity & temperature.
 */








