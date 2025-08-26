/**
 * TerrainGround.h - Terrain Ground Integration Example
 * 
 * Shows how to integrate the TerrainGenerator with the existing Ground system
 * to replace the current flat ground with procedural terrain.
 * 
 * This is an example implementation - you can adapt this approach to your needs.
 */

#pragma once
#include "Ground.h"
#include "../Engine/Utils/TerrainGenerator.h"
#include <memory>

namespace Engine {

/**
 * TerrainGround - Ground with procedural terrain generation
 * 
 * Extends the existing Ground class to use procedural terrain generation
 * instead of flat terrain. This shows how to integrate the terrain system
 * with minimal changes to the existing engine.
 */
class TerrainGround : public Ground {
private:
    // Terrain generator
    TerrainGenerator terrainGenerator;
    
    // Terrain parameters
    TerrainParams terrainParams;
    
    // Chunk terrain data (for each chunk)
    std::unordered_map<std::string, std::vector<TerrainBlockType>> chunkTerrainData;

public:
    // Constructor
    TerrainGround(const std::string& name, float groundSize, const Vec3& groundColor);
    virtual ~TerrainGround() = default;
    
    // Override Ground methods
    virtual void setupMesh() override;
    virtual void updateChunksForPlayer(const Vec3& playerPosition) override;
    
    // Terrain generation
    void generateChunkTerrain(Chunk* chunk);
    void regenerateAllChunks();
    
    // Terrain parameters
    void setTerrainParams(const TerrainParams& params);
    const TerrainParams& getTerrainParams() const;
    
    // Utility
    TerrainBlockType getBlockAtWorldPosition(const Vec3& worldPos) const;
    bool isBlockSolidAtWorldPosition(const Vec3& worldPos) const;

private:
    // Helper methods
    Vec2 getChunkKey(int chunkX, int chunkZ) const;
    std::string getChunkTerrainKey(int chunkX, int chunkZ) const;
    Vec3 getTerrainColor(const Vec3& worldPos) const;
};

} // namespace Engine
