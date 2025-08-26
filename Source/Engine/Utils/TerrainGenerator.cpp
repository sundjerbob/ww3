/**
 * TerrainGenerator.cpp - Procedural Terrain Generation Implementation
 * 
 * Extracts clean Perlin noise-based terrain generation from CProceduralGame.
 * Excludes the problematic erosion system for stability and performance.
 */

#include "TerrainGenerator.h"
#include <cmath>
#include <algorithm>

namespace Engine {

// Perlin noise hash table (from CProceduralGame noise.c)
const long TerrainGenerator::hashTable[256] = {
    208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
    185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
    9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
    70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
    203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
    164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
    228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
    232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
    193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
    101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
    135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
    114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};

TerrainGenerator::TerrainGenerator(const TerrainParams& terrainParams)
    : params(terrainParams) {
}

// Noise generation functions (extracted from CProceduralGame noise.c)
long TerrainGenerator::noise2(int x, int y, long seed) const {
    long tmp = hashTable[(y + seed) % 256];
    return hashTable[(tmp + x) % 256];
}

long TerrainGenerator::noise3(int x, int y, int z, long seed) const {
    long tmp = hashTable[(y + seed) % 256];
    tmp = hashTable[(tmp + x) % 256];
    return hashTable[(tmp + z) % 256];
}

float TerrainGenerator::noise2d(float x, float y, long seed) const {
    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    
    int s = noise2(x_int, y_int, seed);
    int t = noise2(x_int + 1, y_int, seed);
    int u = noise2(x_int, y_int + 1, seed);
    int v = noise2(x_int + 1, y_int + 1, seed);
    
    // Smooth interpolation
    float low = x_frac * x_frac * (3.0f - 2.0f * x_frac) * (t - s) + s;
    float high = x_frac * x_frac * (3.0f - 2.0f * x_frac) * (v - u) + u;
    return y_frac * y_frac * (3.0f - 2.0f * y_frac) * (high - low) + low;
}

float TerrainGenerator::perlin2d(float x, float y, long seed, float freq, int depth) const {
    float xa = x * freq;
    float ya = y * freq;
    float amp = 1.0f;
    float fin = 0.0f;
    float div = 0.0f;

    for (int i = 0; i < depth; i++) {
        div += 256.0f * amp;
        fin += noise2d(xa, ya, seed) * amp;
        amp /= 2.0f;
        xa *= 2.0f;
        ya *= 2.0f;
    }

    return fin / div;
}

HeightMap TerrainGenerator::generateHeightMap(int width, int height, const Vec2& offset) const {
    HeightMap heightMap(width, height);
    
    // Generate raw height values
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            float worldX = static_cast<float>(x) + offset.x;
            float worldZ = static_cast<float>(z) + offset.y;
            
            float heightValue = perlin2d(worldX, worldZ, params.seed, params.frequency, params.octaves);
            heightMap.at(x, z) = heightValue;
        }
    }
    
    // Apply smoothing based on standard deviation
    if (params.standardDeviation < 1.0f) {
        // Apply Gaussian smoothing for flatter terrain
        HeightMap smoothedMap(width, height);
        
        for (int z = 0; z < height; z++) {
            for (int x = 0; x < width; x++) {
                float sum = 0.0f;
                float weightSum = 0.0f;
                
                // Apply smoothing kernel
                int kernelSize = static_cast<int>(3.0f / params.standardDeviation);
                for (int dz = -kernelSize; dz <= kernelSize; dz++) {
                    for (int dx = -kernelSize; dx <= kernelSize; dx++) {
                        int nx = x + dx;
                        int nz = z + dz;
                        
                        if (nx >= 0 && nx < width && nz >= 0 && nz < height) {
                            float distance = sqrt(static_cast<float>(dx * dx + dz * dz));
                            float weight = exp(-distance * distance / (2.0f * params.standardDeviation * params.standardDeviation));
                            
                            sum += heightMap.at(nx, nz) * weight;
                            weightSum += weight;
                        }
                    }
                }
                
                smoothedMap.at(x, z) = sum / weightSum;
            }
        }
        
        heightMap = smoothedMap;
    }
    
    // Apply final height amplification
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            heightMap.at(x, z) *= params.heightAmplifier;
        }
    }
    
    return heightMap;
}

void TerrainGenerator::generateChunkTerrain(std::vector<TerrainBlockType>& blocks, 
                                           int chunkSize, int chunkHeight,
                                           const Vec2& chunkOffset) const {
    // Resize blocks array to accommodate chunk
    blocks.resize(chunkSize * chunkHeight * chunkSize, TerrainBlockType::Air);
    
    // Generate height map for this chunk
    HeightMap heightMap = generateHeightMap(chunkSize, chunkSize, chunkOffset);
    
    // Generate terrain layers
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            // Calculate terrain height at this position
            int terrainHeight = static_cast<int>(heightMap.at(x, z)) + params.seaLevel;
            terrainHeight = std::max(1, std::min(terrainHeight, chunkHeight - 1));
            
            // Generate terrain layers from bottom to top
            for (int y = 0; y < chunkHeight; y++) {
                int blockIndex = y * chunkSize * chunkSize + z * chunkSize + x;
                
                if (y == 0) {
                    // Bedrock at bottom
                    blocks[blockIndex] = TerrainBlockType::Bedrock;
                } else if (y <= terrainHeight) {
                    // Terrain layers
                    blocks[blockIndex] = getBlockType(y, terrainHeight);
                } else if (y <= params.seaLevel) {
                    // Water above terrain up to sea level
                    blocks[blockIndex] = TerrainBlockType::Water;
                } else {
                    // Air above everything
                    blocks[blockIndex] = TerrainBlockType::Air;
                }
            }
        }
    }
}

TerrainBlockType TerrainGenerator::getBlockType(int worldY, int terrainHeight) const {
    if (worldY == terrainHeight) {
        // Surface layer - grass on top
        return TerrainBlockType::Grass;
    } else if (worldY > terrainHeight - params.grassLayerHeight - params.dirtLayerHeight) {
        // Dirt layer
        return TerrainBlockType::Dirt;
    } else if (worldY > terrainHeight - params.grassLayerHeight - params.dirtLayerHeight - params.stoneLayerHeight) {
        // Stone layer
        return TerrainBlockType::Stone;
    } else {
        // Deep stone
        return TerrainBlockType::Stone;
    }
}

bool TerrainGenerator::isBlockSolid(TerrainBlockType blockType) const {
    return blockType != TerrainBlockType::Air && blockType != TerrainBlockType::Water;
}

} // namespace Engine
