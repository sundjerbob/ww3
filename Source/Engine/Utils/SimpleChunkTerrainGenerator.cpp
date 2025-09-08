/**
 * SimpleChunkTerrainGenerator.cpp - Simple Chunk-Based Terrain Generator Implementation
 */

#include "SimpleChunkTerrainGenerator.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace Engine {

SimpleChunkTerrainGenerator::SimpleChunkTerrainGenerator(const SimpleChunkTerrainParams& terrainParams)
    : params(terrainParams), rng(params.seed), perlinNoise(params.seed) {
}

float SimpleChunkTerrainGenerator::getHeightAt(float worldX, float worldZ) const {
    // Generate height using Perlin noise with octaves for natural terrain
    double noise = perlinNoise.getTerrainHeight(worldX, worldZ, 
                                               params.amplitude, params.frequency,
                                               params.octaves, params.persistence, params.lacunarity);
    return params.baseHeight + static_cast<float>(noise);
}

Vec3 SimpleChunkTerrainGenerator::calculateNormal(float worldX, float worldZ, float step) const {
    // Calculate normal using finite differences
    // Sample heights at neighboring points
    float heightCenter = getHeightAt(worldX, worldZ);
    float heightRight = getHeightAt(worldX + step, worldZ);
    float heightLeft = getHeightAt(worldX - step, worldZ);
    float heightForward = getHeightAt(worldX, worldZ + step);
    float heightBack = getHeightAt(worldX, worldZ - step);
    
    // Calculate tangent vectors using finite differences
    Vec3 tangentX = Vec3(2.0f * step, heightRight - heightLeft, 0.0f);
    Vec3 tangentZ = Vec3(0.0f, heightForward - heightBack, 2.0f * step);
    
    // Normal is the cross product of tangent vectors
    Vec3 normal = cross(tangentZ, tangentX);
    
    // Normalize the result
    float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 0.0f) {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    } else {
        // Fallback to up vector if calculation fails
        normal = Vec3(0.0f, 1.0f, 0.0f);
    }
    
    return normal;
}

std::string SimpleChunkTerrainGenerator::getChunkKey(int chunkX, int chunkZ) const {
    std::ostringstream oss;
    oss << chunkX << "_" << chunkZ;
    return oss.str();
}



TerrainChunkData* SimpleChunkTerrainGenerator::getChunk(int chunkX, int chunkZ) {
    std::string key = getChunkKey(chunkX, chunkZ);
    
    auto it = chunks.find(key);
    if (it != chunks.end()) {
        return &it->second;
    }
    
    // Create new chunk
    TerrainChunkData newChunk;
    newChunk.chunkX = chunkX;
    newChunk.chunkZ = chunkZ;
    newChunk.isGenerated = false;
    
    chunks[key] = newChunk;
    return &chunks[key];
}

void SimpleChunkTerrainGenerator::generateChunkMesh(int chunkX, int chunkZ) {
    TerrainChunkData* chunk = getChunk(chunkX, chunkZ);
    if (!chunk || chunk->isGenerated) {
        return;
    }
    
    
    // Calculate chunk world bounds
    float startX = static_cast<float>(chunkX) * params.chunkSize;
    float startZ = static_cast<float>(chunkZ) * params.chunkSize;
    float endX = startX + params.chunkSize;
    float endZ = startZ + params.chunkSize;
    
    // Generate vertices and indices
    std::vector<float>& vertices = chunk->vertices;
    std::vector<unsigned int>& indices = chunk->indices;
    
    vertices.clear();
    indices.clear();
    
    int resolution = params.chunkResolution;
    float step = params.chunkSize / static_cast<float>(resolution - 1);
    
    // Generate vertices
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            float worldX = startX + x * step;
            float worldZ = startZ + z * step;
            float worldY = getHeightAt(worldX, worldZ);
            
            // Position (x, y, z)
            vertices.push_back(worldX);
            vertices.push_back(worldY);
            vertices.push_back(worldZ);
            
            // Calculate normal based on terrain slope
            Vec3 normal = calculateNormal(worldX, worldZ, step);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }
    
    // Generate indices for triangles
    for (int z = 0; z < resolution - 1; z++) {
        for (int x = 0; x < resolution - 1; x++) {
            int topLeft = z * resolution + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * resolution + x;
            int bottomRight = bottomLeft + 1;
            
            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    chunk->isGenerated = true;
    
    std::cout << "Generated chunk with " << vertices.size() / 6
              << " vertices and " << indices.size() << " indices" << std::endl;
}

void SimpleChunkTerrainGenerator::clearAllChunks() {
    chunks.clear();
}

TerrainChunkData* SimpleChunkTerrainGenerator::getChunkAtWorldPos(float worldX, float worldZ) {
    int chunkX = static_cast<int>(worldX / params.chunkSize);
    int chunkZ = static_cast<int>(worldZ / params.chunkSize);
    
    // Handle negative coordinates correctly
    if (worldX < 0) chunkX--;
    if (worldZ < 0) chunkZ--;
    
    return getChunk(chunkX, chunkZ);
}

float SimpleChunkTerrainGenerator::getHeightAtWorldPos(float worldX, float worldZ) const {
    return getHeightAt(worldX, worldZ);
}

void SimpleChunkTerrainGenerator::setParams(const SimpleChunkTerrainParams& newParams) {
    params = newParams;
    rng.seed(params.seed);
    perlinNoise.setSeed(params.seed);
    // Clear chunks to force regeneration with new parameters
    clearAllChunks();
}

} // namespace Engine
