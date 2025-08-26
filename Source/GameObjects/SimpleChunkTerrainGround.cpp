/**
 * SimpleChunkTerrainGround.cpp - Simple Chunk-Based Terrain Ground Implementation
 */

#include "SimpleChunkTerrainGround.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/BasicRenderer.h"
#include <iostream>
#include <sstream>

namespace Engine {

SimpleChunkTerrainGround::SimpleChunkTerrainGround(const std::string& name, float groundSize, const Vec3& groundColor)
    : Ground(name, groundSize, groundColor), isInitialized(false), renderDistance(8), playerPosition(0, 0, 0) {
    
    std::cout << "SimpleChunkTerrainGround: Created - Name: " << name << ", Render Distance: " << renderDistance << std::endl;
    
    // Set up terrain parameters for natural-looking terrain with Perlin noise
    SimpleChunkTerrainParams params;
    params.baseHeight = -10.0f;        // Very low terrain (below entities)
    params.amplitude = 1.5f;           // Moderate height variation
    params.frequency = 0.05f;          // Low frequency for smooth terrain
    params.octaves = 4;                // 4 octaves for natural fractal noise
    params.persistence = 0.5;          // How much each octave contributes
    params.lacunarity = 2.0;           // How frequency changes between octaves
    params.seed = 12345;
    params.chunkSize = 16;             // 16x16 world units per chunk
    params.chunkResolution = 32;       // 32x32 vertices per chunk
    
    terrainGenerator.setParams(params);
    std::cout << "SimpleChunkTerrainGround: Terrain parameters set" << std::endl;
}

bool SimpleChunkTerrainGround::initialize() {
    std::cout << "SimpleChunkTerrainGround: Initializing..." << std::endl;
    
    if (!Ground::initialize()) {
        std::cout << "SimpleChunkTerrainGround: Failed to initialize base Ground" << std::endl;
        return false;
    }
    
    // Generate initial chunks around origin
    updateChunksForPlayer(Vec3(0, 0, 0));
    
    isInitialized = true;
    std::cout << "SimpleChunkTerrainGround: Initialization complete" << std::endl;
    return true;
}

void SimpleChunkTerrainGround::update(float deltaTime) {
    Ground::update(deltaTime);
}

void SimpleChunkTerrainGround::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid() || !isInitialized) {
        return;
    }
    
    std::cout << "SimpleChunkTerrainGround: Rendering " << chunkMeshes.size() << " chunks..." << std::endl;
    
    // Render the terrain mesh with height-based coloring
    const BasicRenderer* basicRenderer = dynamic_cast<const BasicRenderer*>(&renderer);
    
    // Render each chunk
    for (const auto& chunkPair : chunkMeshes) {
        const auto& chunkMesh = chunkPair.second;
        if (chunkMesh) {
            if (basicRenderer) {
                // Use height-based coloring for terrain
                basicRenderer->renderMesh(*chunkMesh, getModelMatrix(), camera, getColor(), true);
            } else {
                // Fall back to regular rendering
                renderer.renderMesh(*chunkMesh, getModelMatrix(), camera, getColor());
            }
        }
    }
}

void SimpleChunkTerrainGround::updateChunksForPlayer(const Vec3& playerPos) {
    playerPosition = playerPos;
    
    // Calculate which chunks should be loaded based on player position
    int playerChunkX = static_cast<int>(playerPos.x / terrainGenerator.getChunkSize());
    int playerChunkZ = static_cast<int>(playerPos.z / terrainGenerator.getChunkSize());
    
    // Handle negative coordinates correctly
    if (playerPos.x < 0) playerChunkX--;
    if (playerPos.z < 0) playerChunkZ--;
    
    std::cout << "SimpleChunkTerrainGround: Player at chunk (" << playerChunkX << ", " << playerChunkZ << ")" << std::endl;
    
    // Generate chunks in render distance
    for (int z = playerChunkZ - renderDistance; z <= playerChunkZ + renderDistance; z++) {
        for (int x = playerChunkX - renderDistance; x <= playerChunkX + renderDistance; x++) {
            if (isChunkInRange(x, z, playerPos)) {
                generateChunk(x, z);
            }
        }
    }
}

void SimpleChunkTerrainGround::generateChunk(int chunkX, int chunkZ) {
    std::string key = getChunkKey(chunkX, chunkZ);
    
    // Check if chunk already exists
    if (chunkMeshes.find(key) != chunkMeshes.end()) {
        return;
    }
    
    std::cout << "SimpleChunkTerrainGround: Generating chunk (" << chunkX << ", " << chunkZ << ")" << std::endl;
    
    // Generate chunk data
    terrainGenerator.generateChunkMesh(chunkX, chunkZ);
    TerrainChunkData* chunkData = terrainGenerator.getChunk(chunkX, chunkZ);
    
    if (chunkData && chunkData->isGenerated) {
        createChunkMesh(*chunkData, chunkX, chunkZ);
    }
}

void SimpleChunkTerrainGround::createChunkMesh(const TerrainChunkData& chunkData, int chunkX, int chunkZ) {
    std::string key = getChunkKey(chunkX, chunkZ);
    
    // Create mesh from chunk data
    auto mesh = std::make_unique<Mesh>();
    if (mesh->createMeshWithNormals(chunkData.vertices, chunkData.indices)) {
        chunkMeshes[key] = std::move(mesh);
        std::cout << "SimpleChunkTerrainGround: Created mesh for chunk (" << chunkX << ", " << chunkZ << ")" << std::endl;
    } else {
        std::cout << "SimpleChunkTerrainGround: FAILED to create mesh for chunk (" << chunkX << ", " << chunkZ << ")" << std::endl;
    }
}

std::string SimpleChunkTerrainGround::getChunkKey(int chunkX, int chunkZ) const {
    std::ostringstream oss;
    oss << chunkX << "_" << chunkZ;
    return oss.str();
}

bool SimpleChunkTerrainGround::isChunkInRange(int chunkX, int chunkZ, const Vec3& playerPos) const {
    // Calculate chunk center
    float chunkCenterX = (chunkX + 0.5f) * terrainGenerator.getChunkSize();
    float chunkCenterZ = (chunkZ + 0.5f) * terrainGenerator.getChunkSize();
    
    // Calculate distance from player to chunk center
    float distance = sqrt((chunkCenterX - playerPos.x) * (chunkCenterX - playerPos.x) + 
                         (chunkCenterZ - playerPos.z) * (chunkCenterZ - playerPos.z));
    
    // Check if chunk is within render distance
    float maxDistance = renderDistance * terrainGenerator.getChunkSize();
    return distance <= maxDistance;
}

void SimpleChunkTerrainGround::setTerrainParams(const SimpleChunkTerrainParams& params) {
    std::cout << "SimpleChunkTerrainGround: Setting new terrain parameters" << std::endl;
    terrainGenerator.setParams(params);
    clearAllChunks(); // Clear existing chunks to regenerate with new parameters
}

const SimpleChunkTerrainParams& SimpleChunkTerrainGround::getTerrainParams() const {
    return terrainGenerator.getParams();
}

float SimpleChunkTerrainGround::getHeightAt(float worldX, float worldZ) const {
    return terrainGenerator.getHeightAtWorldPos(worldX, worldZ);
}

void SimpleChunkTerrainGround::clearAllChunks() {
    std::cout << "SimpleChunkTerrainGround: Clearing all chunks" << std::endl;
    chunkMeshes.clear();
    terrainGenerator.clearAllChunks();
}

} // namespace Engine
