/**
 * PerlinNoise.cpp - Perlin Noise Implementation
 */

#include "PerlinNoise.h"
#include <algorithm>
#include <cmath>

namespace Engine {

PerlinNoise::PerlinNoise(unsigned int seed) : rng(seed) {
    // Initialize permutation table
    p.resize(256);
    for (int i = 0; i < 256; ++i) {
        p[i] = i;
    }
    
    // Shuffle the permutation table
    std::shuffle(p.begin(), p.end(), rng);
    
    // Duplicate the permutation table to avoid overflow
    p.insert(p.end(), p.begin(), p.end());
}

void PerlinNoise::setSeed(unsigned int seed) {
    rng.seed(seed);
    
    // Regenerate permutation table
    for (int i = 0; i < 256; ++i) {
        p[i] = i;
    }
    std::shuffle(p.begin(), p.begin() + 256, rng);
    
    // Duplicate the permutation table
    p.insert(p.begin() + 256, p.begin(), p.begin() + 256);
}

double PerlinNoise::fade(double t) const {
    // Fade function as defined by Ken Perlin
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) const {
    // Convert lower 4 bits of hash code into 12 simple gradient directions
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise3D(double x, double y, double z) const {
    // Find unit cube that contains the point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;
    
    // Find relative x, y, z of point in cube
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    
    // Compute fade curves for each of x, y, z
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);
    
    // Hash coordinates of the 8 cube corners
    int A  = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B  = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;
    
    // Add blended results from 8 corners of cube
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                   grad(p[BA], x - 1, y, z)),
                           lerp(u, grad(p[AB], x, y - 1, z),
                                   grad(p[BB], x - 1, y - 1, z))),
                   lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                   grad(p[BA + 1], x - 1, y, z - 1)),
                           lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                   grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

double PerlinNoise::noise2D(double x, double z) const {
    // Use 3D noise with y = 0 for 2D terrain
    return noise3D(x, 0.0, z);
}

double PerlinNoise::octaveNoise2D(double x, double z, int octaves, double persistence, double lacunarity) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    // Add several octaves of noise together
    for (int i = 0; i < octaves; i++) {
        total += noise2D(x * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    // Normalize the result
    return total / maxValue;
}

double PerlinNoise::getTerrainHeight(double x, double z, 
                                    double amplitude, double frequency, 
                                    int octaves, double persistence, double lacunarity) const {
    // Generate octave noise and scale by amplitude
    double noise = octaveNoise2D(x * frequency, z * frequency, octaves, persistence, lacunarity);
    return noise * amplitude;
}

} // namespace Engine
