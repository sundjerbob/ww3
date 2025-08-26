/**
 * SimpleChunkTerrainGenerator.h - Simple Chunk-Based Terrain Generator
 * 
 * A lightweight chunk-based terrain generator that uses world X,Z positions
 * to generate Y values for terrain chunks, providing infinite terrain capability
 * with much better performance than the complex infinite terrain system.
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <random>
#include "PerlinNoise.h"
#include "../Math/Math.h"

namespace Engine {

// Simple terrain parameters for chunk generation
struct SimpleChunkTerrainParams {
    float baseHeight = -10.0f;        // Base terrain height (negative = below entities)
    float amplitude = 2.0f;           // Terrain amplitude (height variation)
    float frequency = 0.1f;           // Noise frequency (lower = smoother)
    int octaves = 4;                  // Number of octaves for fractal noise
    double persistence = 0.5;         // How much each octave contributes
    double lacunarity = 2.0;          // How frequency changes between octaves
    int seed = 12345;                 // Random seed
    int chunkSize = 16;               // Size of each chunk in world units
    int chunkResolution = 32;         // Resolution of chunk mesh (vertices per side)
};

// Chunk data structure
struct TerrainChunkData {
    std::vector<float> vertices;      // Vertex data (position + normal)
    std::vector<unsigned int> indices; // Index data
    int chunkX, chunkZ;               // Chunk coordinates
    bool isGenerated;                 // Whether chunk mesh is generated
};

class SimpleChunkTerrainGenerator {
private:
    SimpleChunkTerrainParams params;
    std::mt19937 rng;
    std::unordered_map<std::string, TerrainChunkData> chunks;
    PerlinNoise perlinNoise;
    
    // Generate chunk key from coordinates
    std::string getChunkKey(int chunkX, int chunkZ) const;
    
    // Generate height at world position
    float getHeightAt(float worldX, float worldZ) const;
    
    // Calculate normal at world position
    Vec3 calculateNormal(float worldX, float worldZ, float step) const;

public:
    SimpleChunkTerrainGenerator(const SimpleChunkTerrainParams& terrainParams = SimpleChunkTerrainParams());
    
    // Generate or get existing chunk
    TerrainChunkData* getChunk(int chunkX, int chunkZ);
    
    // Generate chunk mesh data
    void generateChunkMesh(int chunkX, int chunkZ);
    
    // Get all loaded chunks
    const std::unordered_map<std::string, TerrainChunkData>& getChunks() const { return chunks; }
    
    // Clear all chunks (for regeneration)
    void clearAllChunks();
    
    // Get chunk at world position
    TerrainChunkData* getChunkAtWorldPos(float worldX, float worldZ);
    
    // Get height at world position
    float getHeightAtWorldPos(float worldX, float worldZ) const;
    
    // Set parameters and regenerate if needed
    void setParams(const SimpleChunkTerrainParams& newParams);
    const SimpleChunkTerrainParams& getParams() const { return params; }
    
    // Get chunk size and resolution
    int getChunkSize() const { return params.chunkSize; }
    int getChunkResolution() const { return params.chunkResolution; }
};

} // namespace Engine
