/**
 * PerlinNoise.h - Perlin Noise Implementation for Terrain Generation
 * 
 * A fast and efficient Perlin noise implementation for generating
 * natural-looking terrain height maps with multiple octaves.
 */

#pragma once
#include <vector>
#include <random>

namespace Engine {

class PerlinNoise {
private:
    std::vector<int> p; // Permutation table
    std::mt19937 rng;
    
    // Fade function for smooth interpolation
    double fade(double t) const;
    
    // Linear interpolation
    double lerp(double t, double a, double b) const;
    
    // Gradient function
    double grad(int hash, double x, double y, double z) const;
    
    // Noise function for 3D coordinates
    double noise3D(double x, double y, double z) const;

public:
    // Constructor with optional seed
    PerlinNoise(unsigned int seed = 0);
    
    // Generate 2D noise (for terrain height)
    double noise2D(double x, double z) const;
    
    // Generate octave noise (multiple frequencies for more natural terrain)
    double octaveNoise2D(double x, double z, int octaves, double persistence, double lacunarity) const;
    
    // Generate terrain height with parameters
    double getTerrainHeight(double x, double z, 
                           double amplitude, double frequency, 
                           int octaves = 4, double persistence = 0.5, double lacunarity = 2.0) const;
    
    // Set new seed and regenerate permutation table
    void setSeed(unsigned int seed);
};

} // namespace Engine
