
struct Map
{
    RNG *rng;
    unsigned int seed;
    //Clock *clock;
    
    //size2D world_size;
    
    void (*callback)(void *state, unsigned int pc);
    void *callback_ptr;
    
    // array of values for every point
    double *elevation;       // normalised 0.0 - 1.0
    double *direct_sunlight; // normalised 0.0 - 1.0
    double *temperature;     // normalised 0.0 - 1.0
    
    //size2D elevation_size;
    //size2D temperature_size;
    //size2D direct_sunlight_size;
};
