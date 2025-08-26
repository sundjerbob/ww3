/**
 * InfiniteTerrainGround.cpp - Infinite Terrain Ground Implementation
 * 
 * Integrates the InfiniteTerrainGenerator with the existing Ground system
 * to create an infinite procedural terrain world.
 */

#include "InfiniteTerrainGround.h"
#include "../Engine/Utils/TerrainGenerator.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include "Minimap.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace Engine {

InfiniteTerrainGround::InfiniteTerrainGround(const std::string& name, float groundSize, const Vec3& groundColor)
    : Ground(name, groundSize, groundColor), useInfiniteTerrain(false), 
      terrainUpdateInterval(0.1f), lastTerrainUpdate(0.0f) {
    
    // Initialize infinite terrain (parameters will be set by Game.cpp)
    // Default parameters will be overridden by setTerrainParams() call
    infiniteTerrain.setRenderDistance(4);
    infiniteTerrain.setMaxLoadedChunks(32);
    
    // Set up callbacks
    setupInfiniteTerrainCallbacks();
}

bool InfiniteTerrainGround::initialize() {
    if (!Ground::initialize()) {
        return false;
    }
    
    // Enable infinite terrain
    useInfiniteTerrain = true;
    
    // InfiniteTerrainGround initialized successfully
    return true;
}

void InfiniteTerrainGround::update(float deltaTime) {
    // Call parent update
    Ground::update(deltaTime);
    
    // Update infinite terrain if enabled
    if (useInfiniteTerrain) {
        lastTerrainUpdate += deltaTime;
        if (lastTerrainUpdate >= terrainUpdateInterval) {
            lastTerrainUpdate = 0.0f;
            // We'll get player position from the scene later
            // updateInfiniteTerrain(camera->getPosition(), deltaTime);
        }
    }
}

void InfiniteTerrainGround::setupMesh() {
    // Call parent setupMesh to initialize basic ground
    Ground::setupMesh();
    
    // Override with infinite terrain setup
    if (useInfiniteTerrain) {
        // Using infinite terrain generation
    }
}

void InfiniteTerrainGround::updateChunksForPlayer(const Vec3& playerPosition) {
    if (useInfiniteTerrain) {
        // Use infinite terrain system
            // Updating infinite terrain for player
        infiniteTerrain.update(playerPosition, 0.016f); // Assume 60 FPS
    } else {
        // Fall back to original ground system
        Ground::updateChunksForPlayer(playerPosition);
    }
}

void InfiniteTerrainGround::setupInfiniteTerrainCallbacks() {
    // Set up callback for when chunks are generated
    infiniteTerrain.setOnChunkGenerated([this](const ChunkCoord& coord, const std::vector<TerrainBlockType>& blocks) {
        generateTerrainChunk(coord, blocks);
    });
    
    // Set up callback for when chunks are unloaded
    infiniteTerrain.setOnChunkUnloaded([this](const ChunkCoord& coord) {
        unloadTerrainChunk(coord);
    });
}

void InfiniteTerrainGround::updateInfiniteTerrain(const Vec3& playerPosition, float deltaTime) {
    // Update the infinite terrain generator
    infiniteTerrain.update(playerPosition, deltaTime);
}

void InfiniteTerrainGround::generateTerrainChunk(const ChunkCoord& coord, const std::vector<TerrainBlockType>& blocks) {
    // Create a new terrain chunk with height data
    std::string chunkName = "TerrainChunk_" + std::to_string(coord.x) + "_" + std::to_string(coord.z);
    auto terrainChunk = std::make_unique<TerrainChunk>(chunkName, Vec2(static_cast<float>(coord.x), static_cast<float>(coord.z)), 16, 1.0f);
    
    // Set terrain data
    terrainChunk->setTerrainData(blocks);
    terrainChunk->setTerrainColor(getColor());
    
    // Set the renderer for the chunk
    if (objectRenderer) {
        terrainChunk->setRenderer(objectRenderer);
    }
    
    // Initialize the chunk
    terrainChunk->initialize();
    
    // Store the chunk
    terrainChunks[coord] = std::move(terrainChunk);
    
    // Notify minimap about new chunk
    Minimap* minimapRef = getMinimapReference();
    if (minimapRef) {
        // For now, we'll skip the minimap notification until we implement onChunkAdded
        // minimapRef->onChunkAdded(terrainChunks[coord].get());
    }
    
    // Terrain chunk generated successfully
    
    // Debug: Check if adjacent chunks exist
    ChunkCoord left(coord.x - 1, coord.z);
    ChunkCoord right(coord.x + 1, coord.z);
    ChunkCoord top(coord.x, coord.z + 1);
    ChunkCoord bottom(coord.x, coord.z - 1);
    

}

void InfiniteTerrainGround::unloadTerrainChunk(const ChunkCoord& coord) {
    auto it = terrainChunks.find(coord);
    if (it != terrainChunks.end()) {
        terrainChunks.erase(it);
        // Terrain chunk unloaded
    }
}

bool InfiniteTerrainGround::hasTerrainChunk(const ChunkCoord& coord) const {
    return terrainChunks.find(coord) != terrainChunks.end();
}

TerrainBlockType InfiniteTerrainGround::getBlockAtWorldPosition(const Vec3& worldPos) const {
    return infiniteTerrain.getBlockAtWorldPosition(worldPos);
}

bool InfiniteTerrainGround::isBlockSolidAtWorldPosition(const Vec3& worldPos) const {
    TerrainBlockType blockType = getBlockAtWorldPosition(worldPos);
    return infiniteTerrain.isBlockSolid(blockType);
}

Vec3 InfiniteTerrainGround::getTerrainColor(const Vec3& worldPos) const {
    TerrainBlockType blockType = getBlockAtWorldPosition(worldPos);
    return getBlockColor(blockType);
}

void InfiniteTerrainGround::setTerrainParams(const TerrainParams& params) {
    // Update the terrain generator parameters
    infiniteTerrain.setTerrainParams(params);
    
    // Force complete regeneration of all terrain
    forceCompleteTerrainRegeneration();
}

void InfiniteTerrainGround::forceCompleteTerrainRegeneration() {
    // Force complete terrain regeneration
    terrainChunks.clear();
    infiniteTerrain.forceRegenerateAllChunks();
}

const TerrainParams& InfiniteTerrainGround::getTerrainParams() const {
    return infiniteTerrain.getTerrainParams();
}

void InfiniteTerrainGround::setRenderDistance(int distance) {
    infiniteTerrain.setRenderDistance(distance);
}

void InfiniteTerrainGround::setMaxLoadedChunks(int max) {
    infiniteTerrain.setMaxLoadedChunks(max);
}

void InfiniteTerrainGround::printTerrainStatistics() const {
    // Terrain statistics available
    infiniteTerrain.printStatistics();
}

Vec3 InfiniteTerrainGround::getBlockColor(TerrainBlockType blockType) const {
    switch (blockType) {
        case TerrainBlockType::Grass:
            return Vec3(0.2f, 0.8f, 0.2f); // Green
        case TerrainBlockType::Dirt:
            return Vec3(0.6f, 0.4f, 0.2f); // Brown
        case TerrainBlockType::Stone:
            return Vec3(0.5f, 0.5f, 0.5f); // Gray
        case TerrainBlockType::Bedrock:
            return Vec3(0.2f, 0.2f, 0.2f); // Dark gray
        case TerrainBlockType::Water:
            return Vec3(0.0f, 0.5f, 1.0f); // Blue
        case TerrainBlockType::Sand:
            return Vec3(0.9f, 0.9f, 0.6f); // Sand color
        case TerrainBlockType::Air:
        default:
            return Vec3(0.0f, 0.0f, 0.0f); // Black (shouldn't be rendered)
    }
}

bool InfiniteTerrainGround::shouldRenderBlockFace(const std::vector<TerrainBlockType>& blocks, int x, int y, int z, int faceDir) const {
    // Check if the adjacent block in the face direction is air or water
    int adjacentX = x;
    int adjacentY = y;
    int adjacentZ = z;
    
    switch (faceDir) {
        case 0: adjacentX++; break; // +X
        case 1: adjacentX--; break; // -X
        case 2: adjacentY++; break; // +Y
        case 3: adjacentY--; break; // -Y
        case 4: adjacentZ++; break; // +Z
        case 5: adjacentZ--; break; // -Z
    }
    
    // Bounds checking
    if (adjacentX < 0 || adjacentX >= 16 || adjacentY < 0 || adjacentY >= 256 || adjacentZ < 0 || adjacentZ >= 16) {
        return true; // Render face if adjacent block is outside bounds
    }
    
    int adjacentIndex = getBlockIndex(adjacentX, adjacentY, adjacentZ);
    if (adjacentIndex >= 0 && adjacentIndex < static_cast<int>(blocks.size())) {
        TerrainBlockType adjacentBlock = blocks[adjacentIndex];
        return !infiniteTerrain.isBlockSolid(adjacentBlock);
    }
    
    return true;
}

int InfiniteTerrainGround::getBlockIndex(int x, int y, int z) const {
    return y * 16 * 16 + z * 16 + x;
}

void InfiniteTerrainGround::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid() || !useInfiniteTerrain) {
        return;
    }
    
    // Rendering terrain chunks
    
    // Render all terrain chunks
    for (auto& pair : terrainChunks) {
        auto& chunk = pair.second;
        if (chunk && chunk->getActive()) {
            // Rendering terrain chunk
            chunk->render(renderer, camera);
        } else {
            // Skipping inactive chunk
        }
    }
}



const std::vector<std::unique_ptr<Chunk>>& InfiniteTerrainGround::getChunks() const {
    // For now, return the base class chunks (empty)
    // The minimap will need to be updated to handle terrain chunks separately
    return Ground::getChunks();
}

const std::vector<GameObject*>& InfiniteTerrainGround::getVisibleEntities() const {
    // Clear the visible entities list
    terrainVisibleEntities.clear();
    
    // Add all terrain chunks as visible entities
    for (const auto& pair : terrainChunks) {
        if (pair.second && pair.second->getActive()) {
            terrainVisibleEntities.push_back(pair.second.get());
        }
    }
    
    return terrainVisibleEntities;
}

} // namespace Engine
