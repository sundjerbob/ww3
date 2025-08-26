/**
 * TerrainChunk.h - Terrain Chunk with Height Data
 * 
 * A specialized chunk class that represents terrain with actual height data
 * instead of flat chunks.
 */

#pragma once
#include "Chunk.h"
#include "../Engine/Utils/TerrainGenerator.h"
#include <vector>

namespace Engine {

/**
 * TerrainChunk - Chunk with height-based terrain
 * 
 * Represents a chunk of terrain with actual height data,
 * creating a proper 3D terrain mesh.
 */
class TerrainChunk : public Chunk {
private:
    // Terrain data
    std::vector<TerrainBlockType> terrainBlocks;
    std::vector<float> heightMap;
    int maxHeight;
    
    // Mesh data
    std::vector<float> terrainVertices;
    std::vector<unsigned int> terrainIndices;
    std::vector<float> terrainNormals;
    
    // Terrain properties
    Vec3 terrainColor;
    bool meshGenerated;

public:
    // Constructor
    TerrainChunk(const std::string& name, const Vec2& position, int size, float cubeSize);
    
    // Terrain-specific methods
    void setTerrainData(const std::vector<TerrainBlockType>& blocks);
    void setHeightMap(const std::vector<float>& heights);
    void setTerrainColor(const Vec3& color);
    
    // Mesh generation
    void generateTerrainMesh();
    void updateMesh();
    
    // Override Chunk methods
    virtual void setupMesh() override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
    // Getters
    int getMaxHeight() const { return maxHeight; }
    const std::vector<float>& getHeightMap() const { return heightMap; }
    bool isMeshGenerated() const { return meshGenerated; }

private:
    // Helper methods
    void calculateMaxHeight();
    void generateTerrainVertices();
    void generateTerrainIndices();
    void calculateNormals();
    Vec3 getBlockColor(TerrainBlockType blockType) const;
};

} // namespace Engine
