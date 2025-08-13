/**
 * Ground.h - Ground Plane GameObject
 * 
 * OVERVIEW:
 * A terrain system that uses chunks for efficient rendering.
 * Inherits from GameObject and provides chunk-based terrain functionality.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "Chunk.h"
#include <GL/glew.h>
#include <vector>
#include <memory>

namespace Engine {

/**
 * Ground - Chunk-based Terrain GameObject
 * 
 * Features:
 * - Chunk-based terrain system
 * - Distance-based rendering
 * - Configurable chunk size and render distance
 * - Positioned below other objects
 */
class Ground : public GameObject {
private:
    // Ground properties
    Vec3 color;
    float size;
    
    // Chunk system
    std::vector<std::unique_ptr<Chunk>> chunks;
    int chunkSize;  // Number of cubes per chunk side
    float cubeSize; // Size of each cube
    float renderDistance; // Distance within which chunks are rendered
    
public:
    // Constructor/Destructor
    Ground(const std::string& name = "Ground", float groundSize = 50.0f, const Vec3& groundColor = Vec3(0.4f, 0.3f, 0.2f));
    virtual ~Ground() = default;
    
    // Ground-specific methods
    void setColor(const Vec3& groundColor) { color = groundColor; }
    Vec3 getColor() const { return color; }
    
    void setSize(float groundSize) { size = groundSize; }
    float getSize() const { return size; }
    
    // Chunk system methods
    void setRenderDistance(float distance) { renderDistance = distance; }
    float getRenderDistance() const { return renderDistance; }
    
    // Minimap support - get chunks for rendering
    const std::vector<std::unique_ptr<Chunk>>& getChunks() const { return chunks; }
    
protected:
    // Override GameObject setup methods
    virtual void setupMesh() override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
private:
    // Helper methods
    void generateInitialChunks();
    void updateChunkVisibility(const Vec3& playerPosition);
};

} // namespace Engine
