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

// Forward declaration for Minimap
namespace Engine {
    class Minimap;
}

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
    float size;
    
    // Chunk system
    std::vector<std::unique_ptr<Chunk>> chunks;
    int chunkSize;  // Number of cubes per chunk side
    float cubeSize; // Size of each cube
    float renderDistance; // Distance within which chunks are rendered (smaller perimeter)
    float keepInMemoryDistance; // Distance to keep chunks in memory (bigger perimeter)
    
public:
    // Constructor/Destructor
    Ground(const std::string& name = "Ground", float groundSize = 50.0f, const Vec3& groundColor = Vec3(0.4f, 0.3f, 0.2f));
    virtual ~Ground() = default;
    
    // Ground-specific methods (color methods inherited from GameObject)
    
    void setSize(float groundSize) { size = groundSize; }
    float getSize() const { return size; }
    
    // Chunk system methods
    void setRenderDistance(float distance) { renderDistance = distance; }
    float getRenderDistance() const { return renderDistance; }
    
    // Minimap support - get chunks for rendering
    virtual const std::vector<std::unique_ptr<Chunk>>& getChunks() const { return chunks; }
    
    // Dynamic chunk generation
    virtual void updateChunksForPlayer(const Vec3& playerPosition);
    bool isPlayerInsideTerrain(const Vec3& playerPosition) const;
    Vec2 getChunkCoordinates(const Vec3& worldPosition) const;
    bool hasChunkAt(const Vec2& chunkCoords) const;
    void generateChunkAt(const Vec2& chunkCoords);
    
    // Sliding window chunk system
    void updateSlidingChunkWindow(const Vec3& playerPosition);
    void repositionChunkToCoordinates(Chunk* chunk, const Vec2& newCoords);
    Vec2 getPlayerChunkCoordinates(const Vec3& playerPosition) const;
    
    // Entity system
    bool isEntityOnVisibleChunk(const Vec3& entityPosition) const;
    bool isPositionOnVisibleChunk(const Vec3& position) const;
    
    // Global visibility system - Approach 2
    virtual const std::vector<GameObject*>& getVisibleEntities() const { return visibleEntities; }
    bool isEntityVisible(const GameObject* entity) const;
    void updateEntityVisibility(const std::vector<std::unique_ptr<GameObject>>& allEntities);
    
    // Minimap notification
    void setMinimapReference(class Minimap* minimap) { minimapReference = minimap; }
    Minimap* getMinimapReference() const { return minimapReference; }
    
private:
    // Minimap reference for notifications
    class Minimap* minimapReference;
    
    // Global visibility system - Approach 2
    std::vector<GameObject*> visibleEntities; // Entities currently on visible chunks
    
    // Chunk cleanup and memory management
    void cleanupDistantChunks(const Vec3& playerPosition);
    bool shouldRemoveChunk(const Chunk* chunk, const Vec3& playerPosition) const;
    bool shouldKeepInMemory(const Chunk* chunk, const Vec3& playerPosition) const;
    
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
