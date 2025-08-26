/**
 * Chunk.h - Terrain Chunk GameObject
 * 
 * OVERVIEW:
 * A single chunk of terrain that contains multiple cubes.
 * Each chunk is a collection of cubes that form part of the terrain.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include <vector>
#include <memory>

namespace Engine {

/**
 * Chunk - Terrain Chunk GameObject
 * 
 * Features:
 * - Collection of cubes forming terrain
 * - Configurable chunk size
 * - Position-based rendering
 */
class Chunk : public GameObject {
private:
    // Chunk properties
    int chunkSize;  // Number of cubes per side (e.g., 16x16)
    float cubeSize; // Size of each individual cube
    Vec2 chunkPosition; // 2D position in chunk grid (x, z)
    
    // Chunk data
    std::vector<std::vector<float>> heightMap; // Height values for each position
    
public:
    // Constructor/Destructor
    Chunk(const std::string& name, const Vec2& position, int size = 16, float cubeSize = 1.0f);
    virtual ~Chunk() = default;
    
    // Chunk-specific methods (color methods inherited from GameObject)
    
    Vec2 getChunkPosition() const { return chunkPosition; }
    int getChunkSize() const { return chunkSize; }
    
    // Chunk repositioning methods
    void setChunkPosition(const Vec2& newPosition) { chunkPosition = newPosition; }
    void regenerateHeightMap();
    
    // Check if chunk is within render distance
    bool isInRenderDistance(const Vec3& playerPosition, float renderDistance) const;
    
public:
    // Override GameObject setup methods
    virtual void setupMesh() override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
private:
    // Helper methods
    void generateHeightMap();
    float getHeightAt(int x, int z) const;
};

} // namespace Engine
