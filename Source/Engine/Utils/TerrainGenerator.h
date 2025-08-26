/**
 * TerrainGenerator.h - Procedural Terrain Generation System
 * 
 * OVERVIEW:
 * Extracts the good terrain generation from CProceduralGame while excluding
 * the problematic erosion system. Provides clean Perlin noise-based terrain
 * generation for the WW3 chunk system.
 * 
 * FEATURES:
 * - Perlin noise-based height map generation
 * - Configurable terrain parameters
 * - Chunk-based terrain generation
 * - Layered terrain (grass, dirt, stone, bedrock)
 * - Water level system
 * - No erosion (excluded due to implementation issues)
 */

#pragma once
#include "../Math/Math.h"
#include <vector>
#include <memory>

namespace Engine {

// Terrain block types
enum class TerrainBlockType {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    Bedrock = 4,
    Water = 5,
    Sand = 6
};

// Terrain generation parameters
struct TerrainParams {
    float heightAmplifier = 0.0001f;  // Extremely ultra-low terrain height multiplier
    float frequency = 0.08f;          // Noise frequency (smoother terrain)
    int octaves = 1;                  // Single octave for minimal variation
    float standardDeviation = 0.001f; // Terrain smoothness (extremely flat)
    int seaLevel = -100;              // Water level (extremely deep below ground level)
    float grassLayerHeight = 0.01f;   // Extremely thin grass layer
    float dirtLayerHeight = 0.02f;    // Extremely thin dirt layer
    float stoneLayerHeight = 0.03f;   // Extremely thin stone layer
    long seed = 12345;                // Random seed for terrain generation
};

// Height map for a chunk
struct HeightMap {
    std::vector<float> heights;
    int width;
    int height;
    
    HeightMap(int w, int h) : width(w), height(h) {
        heights.resize(width * height, 0.0f);
    }
    
    float& at(int x, int z) { return heights[z * width + x]; }
    const float& at(int x, int z) const { return heights[z * width + x]; }
};

/**
 * TerrainGenerator - Procedural terrain generation system
 * 
 * Provides clean, efficient terrain generation using Perlin noise.
 * Excludes the problematic erosion system from the original implementation.
 */
class TerrainGenerator {
private:
    TerrainParams params;
    
    // Perlin noise hash table (from CProceduralGame)
    static const long hashTable[256];
    
    // Noise generation functions
    long noise2(int x, int y, long seed) const;
    long noise3(int x, int y, int z, long seed) const;
    float noise2d(float x, float y, long seed) const;
    float perlin2d(float x, float y, long seed, float freq, int depth) const;

public:
    // Constructor
    TerrainGenerator(const TerrainParams& terrainParams = TerrainParams());
    
    // Terrain generation
    HeightMap generateHeightMap(int width, int height, const Vec2& offset) const;
    
    // Chunk terrain generation
    void generateChunkTerrain(std::vector<TerrainBlockType>& blocks, 
                             int chunkSize, int chunkHeight,
                             const Vec2& chunkOffset) const;
    
    // Utility functions
    TerrainBlockType getBlockType(int worldY, int terrainHeight) const;
    bool isBlockSolid(TerrainBlockType blockType) const;
    
    // Parameter access
    void setParams(const TerrainParams& newParams) { params = newParams; }
    const TerrainParams& getParams() const { return params; }
};

} // namespace Engine
