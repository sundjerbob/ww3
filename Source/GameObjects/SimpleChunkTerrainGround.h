/**
 * SimpleChunkTerrainGround.h - Simple Chunk-Based Terrain Ground Implementation
 * 
 * A lightweight chunk-based terrain ground that uses SimpleChunkTerrainGenerator
 * for infinite terrain generation with good performance.
 */

#pragma once
#include "Ground.h"
#include "../Engine/Utils/SimpleChunkTerrainGenerator.h"
#include "../Engine/Rendering/Mesh.h"
#include <memory>
#include <vector>
#include <unordered_map>

namespace Engine {

class SimpleChunkTerrainGround : public Ground {
private:
    SimpleChunkTerrainGenerator terrainGenerator;
    std::unordered_map<std::string, std::unique_ptr<Mesh>> chunkMeshes;
    bool isInitialized;
    
    // Terrain parameters
    int renderDistance;
    Vec3 playerPosition;
    
public:
    // Constructor
    SimpleChunkTerrainGround(const std::string& name, float groundSize, const Vec3& groundColor);
    
    // Override base class methods
    bool initialize() override;
    void update(float deltaTime) override;
    void render(const Renderer& renderer, const Camera& camera) override;
    
    // Terrain generation and management
    void updateChunksForPlayer(const Vec3& playerPos);
    void generateChunk(int chunkX, int chunkZ);
    void setTerrainParams(const SimpleChunkTerrainParams& params);
    const SimpleChunkTerrainParams& getTerrainParams() const;
    
    // Height query
    float getHeightAt(float worldX, float worldZ) const;
    
    // Chunk management
    void setRenderDistance(int distance) { renderDistance = distance; }
    int getRenderDistance() const { return renderDistance; }
    void clearAllChunks();
    
    // Get chunk information
    const std::unordered_map<std::string, std::unique_ptr<Mesh>>& getChunkMeshes() const { return chunkMeshes; }
    int getLoadedChunkCount() const { return static_cast<int>(chunkMeshes.size()); }
    
private:
    std::string getChunkKey(int chunkX, int chunkZ) const;
    void createChunkMesh(const TerrainChunkData& chunkData, int chunkX, int chunkZ);
    bool isChunkInRange(int chunkX, int chunkZ, const Vec3& playerPos) const;
};

} // namespace Engine
