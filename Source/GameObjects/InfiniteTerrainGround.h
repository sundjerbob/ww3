/**
 * InfiniteTerrainGround.h - Infinite Terrain Ground Integration
 * 
 * Integrates the InfiniteTerrainGenerator with the existing Ground system
 * to create an infinite procedural terrain world.
 * 
 * FEATURES:
 * - Infinite terrain generation as player moves
 * - Dynamic chunk loading/unloading
 * - Seamless terrain transitions
 * - Memory-efficient chunk management
 * - Integration with existing Ground system
 */

#pragma once
#include "Ground.h"
#include "TerrainChunk.h"
#include "../Engine/Utils/InfiniteTerrainGenerator.h"
#include <unordered_map>
#include <memory>

namespace Engine {

/**
 * InfiniteTerrainGround - Ground with infinite terrain generation
 * 
 * Extends the existing Ground class to use infinite terrain generation.
 * Dynamically loads and unloads terrain chunks as the player moves.
 */
class InfiniteTerrainGround : public Ground {
private:
    // Infinite terrain generator
    InfiniteTerrainGenerator infiniteTerrain;
    
    // Chunk management
    std::unordered_map<ChunkCoord, std::unique_ptr<TerrainChunk>, ChunkCoord::Hash> terrainChunks;
    
    // Configuration
    bool useInfiniteTerrain;
    float terrainUpdateInterval;
    float lastTerrainUpdate;
    
    // Mutable storage for visible entities (for minimap integration)
    mutable std::vector<GameObject*> terrainVisibleEntities;

public:
    // Constructor
    InfiniteTerrainGround(const std::string& name, float groundSize, const Vec3& groundColor);
    virtual ~InfiniteTerrainGround() = default;
    
    // Override Ground methods
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void setupMesh() override;
    virtual void updateChunksForPlayer(const Vec3& playerPosition) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
    // Infinite terrain methods
    void enableInfiniteTerrain(bool enable) { useInfiniteTerrain = enable; }
    bool isInfiniteTerrainEnabled() const { return useInfiniteTerrain; }
    
    // Terrain configuration
    void setTerrainParams(const TerrainParams& params);
    const TerrainParams& getTerrainParams() const;
    void setRenderDistance(int distance);
    void setMaxLoadedChunks(int max);
    void forceCompleteTerrainRegeneration();
    
    // Chunk management
    void generateTerrainChunk(const ChunkCoord& coord, const std::vector<TerrainBlockType>& blocks);
    void unloadTerrainChunk(const ChunkCoord& coord);
    bool hasTerrainChunk(const ChunkCoord& coord) const;
    
    // Utility
    TerrainBlockType getBlockAtWorldPosition(const Vec3& worldPos) const;
    bool isBlockSolidAtWorldPosition(const Vec3& worldPos) const;
    Vec3 getTerrainColor(const Vec3& worldPos) const;
    
    // Statistics
    int getLoadedTerrainChunkCount() const { return static_cast<int>(terrainChunks.size()); }
    void printTerrainStatistics() const;

private:
    // Helper methods
    void setupInfiniteTerrainCallbacks();
    void updateInfiniteTerrain(const Vec3& playerPosition, float deltaTime);
    Vec3 getBlockColor(TerrainBlockType blockType) const;
    bool shouldRenderBlockFace(const std::vector<TerrainBlockType>& blocks, int x, int y, int z, int faceDir) const;
    int getBlockIndex(int x, int y, int z) const;
    
    // Override getChunks method for minimap integration
    const std::vector<std::unique_ptr<Chunk>>& getChunks() const override;
    
    // Override getVisibleEntities for minimap integration
    const std::vector<GameObject*>& getVisibleEntities() const override;
    
    // Get terrain chunks specifically
    const std::unordered_map<ChunkCoord, std::unique_ptr<TerrainChunk>, ChunkCoord::Hash>& getTerrainChunks() const { return terrainChunks; }
};

} // namespace Engine
