/**
 * Ground.cpp - Implementation of Chunk-based Ground GameObject
 */

#include "Ground.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include "Minimap.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace Engine {

Ground::Ground(const std::string& name, float groundSize, const Vec3& groundColor)
    : GameObject(name), size(groundSize), minimapReference(nullptr) {
    // Set the color using the base class method
    setColor(groundColor);
    
    // Set ground position (below other objects)
    setPosition(Vec3(0.0f, -2.0f, 0.0f));
    
    // Initialize chunk system
    chunkSize = 16;  // 16x16 cubes per chunk
    cubeSize = 1.0f; // Each cube is 1 unit
    renderDistance = 100.0f; // Render chunks within 100 units (smaller perimeter) - doubled for monster visibility
    keepInMemoryDistance = 200.0f; // Keep chunks in memory within 200 units (bigger perimeter) - doubled
    
    // Mark as system object (not an entity)
    setEntity(false);
}

void Ground::generateInitialChunks() {
    // Create a 5x5 grid of chunks to cover twice the area for monster visibility
    // Each chunk is 16x16 units, so 5x5 chunks = 80x80 units total
    
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            std::string chunkName = "Chunk_" + std::to_string(x) + "_" + std::to_string(z);
            auto chunk = std::make_unique<Chunk>(chunkName, Vec2(static_cast<float>(x), static_cast<float>(z)), chunkSize, cubeSize);
            chunk->setColor(getColor());
            
            // Set the renderer for the chunk (same as the ground's renderer)
            if (objectRenderer) {
                chunk->setRenderer(objectRenderer);
            }
            
            // Position the chunk relative to the ground's position
            Vec3 groundPos = getPosition();
            Vec3 chunkWorldPos = chunk->getPosition();
            chunk->setPosition(Vec3(chunkWorldPos.x + groundPos.x, groundPos.y, chunkWorldPos.z + groundPos.z));
            
            chunk->initialize();
            
            chunks.push_back(std::move(chunk));
        }
    }
    
}

void Ground::updateChunkVisibility(const Vec3& playerPosition) {
    // Dual-perimeter system:
    // - Smaller perimeter (renderDistance): Chunks are rendered and visible
    // - Bigger perimeter (keepInMemoryDistance): Chunks kept in memory but not rendered
    for (auto& chunk : chunks) {
        bool shouldRender = chunk->isInRenderDistance(playerPosition, renderDistance);
        chunk->setActive(shouldRender);
    }
}

void Ground::setupMesh() {
    // Ground no longer needs its own mesh - it uses chunks
    // Generate initial chunks here when the Ground is properly initialized
    generateInitialChunks();
}

void Ground::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid()) {
        return;
    }
    
    // Get player position and update chunks dynamically
    Vec3 playerPosition = camera.getPosition();
    updateChunksForPlayer(playerPosition);
    updateChunkVisibility(playerPosition);  // Set visibility based on smaller perimeter
    
    // Update entity visibility system when chunks change
    // Note: We need access to all entities here - this will be called from Scene
    // For now, we'll update this in a separate method that Scene can call
    
    // Render all active chunks
    int renderedChunks = 0;
    for (auto& chunk : chunks) {
        if (chunk->getActive()) {
            chunk->render(renderer, camera);
            renderedChunks++;
        }
    }
    
    // Ground rendering complete
}

void Ground::updateChunksForPlayer(const Vec3& playerPosition) {
    // Use sliding window system for dynamic terrain that follows the player
    updateSlidingChunkWindow(playerPosition);
}

bool Ground::isPlayerInsideTerrain(const Vec3& playerPosition) const {
    // Check if player is within the bounds of any existing chunk
    for (const auto& chunk : chunks) {
        Vec3 chunkPos = chunk->getPosition();
        float chunkHalfSize = (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
        
        // Check if player is within this chunk's bounds
        if (playerPosition.x >= chunkPos.x - chunkHalfSize && 
            playerPosition.x <= chunkPos.x + chunkHalfSize &&
            playerPosition.z >= chunkPos.z - chunkHalfSize && 
            playerPosition.z <= chunkPos.z + chunkHalfSize) {
            return true;
        }
    }
    return false;
}

Vec2 Ground::getChunkCoordinates(const Vec3& worldPosition) const {
    // Convert world position to chunk coordinates
    // Each chunk is chunkSize * cubeSize units wide
    float chunkWorldSize = static_cast<float>(chunkSize) * cubeSize;
    
    int chunkX = static_cast<int>(std::floor(worldPosition.x / chunkWorldSize));
    int chunkZ = static_cast<int>(std::floor(worldPosition.z / chunkWorldSize));
    
    return Vec2(static_cast<float>(chunkX), static_cast<float>(chunkZ));
}

bool Ground::hasChunkAt(const Vec2& chunkCoords) const {
    // Check if we already have a chunk at these coordinates
    for (const auto& chunk : chunks) {
        Vec2 existingCoords = chunk->getChunkPosition();
        if (existingCoords.x == chunkCoords.x && existingCoords.y == chunkCoords.y) {
            return true;
        }
    }
    return false;
}

void Ground::generateChunkAt(const Vec2& chunkCoords) {
    std::string chunkName = "Chunk_" + std::to_string(chunkCoords.x) + "_" + std::to_string(chunkCoords.y);
    auto chunk = std::make_unique<Chunk>(chunkName, chunkCoords, chunkSize, cubeSize);
    chunk->setColor(color);
    
    // Set the renderer for the chunk (same as the ground's renderer)
    if (objectRenderer) {
        chunk->setRenderer(objectRenderer);
    }
    
    // Position the chunk relative to the ground's position
    Vec3 groundPos = getPosition();
    Vec3 chunkWorldPos = chunk->getPosition();
    chunk->setPosition(Vec3(chunkWorldPos.x + groundPos.x, groundPos.y, chunkWorldPos.z + groundPos.z));
    
    chunk->initialize();
    
    chunks.push_back(std::move(chunk));
    
}

void Ground::cleanupDistantChunks(const Vec3& playerPosition) {
    // Remove chunks that are outside the bigger perimeter
    auto it = chunks.begin();
    int removedCount = 0;
    
    while (it != chunks.end()) {
        if (shouldRemoveChunk(it->get(), playerPosition)) {
            it = chunks.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    if (removedCount > 0) {
    }
}

bool Ground::shouldRemoveChunk(const Chunk* chunk, const Vec3& playerPosition) const {
    // Remove chunk if it's outside the bigger perimeter (keepInMemoryDistance)
    return !shouldKeepInMemory(chunk, playerPosition);
}

bool Ground::shouldKeepInMemory(const Chunk* chunk, const Vec3& playerPosition) const {
    // Keep chunk in memory if it's within the bigger perimeter
    Vec3 chunkCenter = chunk->getPosition();
    chunkCenter.x += (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
    chunkCenter.z += (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
    
    float distance = sqrt(
        static_cast<float>((playerPosition.x - chunkCenter.x) * (playerPosition.x - chunkCenter.x) +
        (playerPosition.z - chunkCenter.z) * (playerPosition.z - chunkCenter.z))
    );
    
    return distance <= keepInMemoryDistance;
}

void Ground::updateSlidingChunkWindow(const Vec3& playerPosition) {
    // Get player's current chunk coordinates
    Vec2 playerChunkCoords = getPlayerChunkCoordinates(playerPosition);
    
    // Calculate the 5x5 grid of chunk coordinates around the player
    std::vector<Vec2> requiredChunkCoords;
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            Vec2 chunkCoords(playerChunkCoords.x + static_cast<float>(x), playerChunkCoords.y + static_cast<float>(z));
            requiredChunkCoords.push_back(chunkCoords);
        }
    }
    
    // Check if we need to reposition chunks
    bool needsRepositioning = false;
    for (const auto& requiredCoords : requiredChunkCoords) {
        if (!hasChunkAt(requiredCoords)) {
            needsRepositioning = true;
            break;
        }
    }
    
    if (needsRepositioning) {
        // Reposition existing chunks to new coordinates
        int chunkIndex = 0;
        for (const auto& requiredCoords : requiredChunkCoords) {
            if (chunkIndex < static_cast<int>(chunks.size())) {
                // Reposition existing chunk
                repositionChunkToCoordinates(chunks[chunkIndex].get(), requiredCoords);
    } else {
                // Generate new chunk if we don't have enough
                generateChunkAt(requiredCoords);
            }
            chunkIndex++;
        }
        
        // Remove excess chunks if we have more than 25 (5x5 grid)
        while (chunks.size() > 25U) {
            chunks.pop_back();
        }
        
        
            // Notify minimap that chunks have changed
        if (minimapReference) {
            minimapReference->forceUpdate();
        }
    }
}

// ============================================================================
// GLOBAL VISIBILITY SYSTEM - APPROACH 2
// ============================================================================

bool Ground::isEntityVisible(const GameObject* entity) const {
    if (!entity || !entity->getEntity()) {
        return false; // Non-entities are not part of the visibility system
    }
    
    // Check if this entity is in our visible entities list
    return std::find(visibleEntities.begin(), visibleEntities.end(), entity) != visibleEntities.end();
}

void Ground::updateEntityVisibility(const std::vector<std::unique_ptr<GameObject>>& allEntities) {
    // Clear previous visible entities list
    visibleEntities.clear();
    
    // Check each entity to see if it's on a visible chunk
    for (const auto& entity : allEntities) {
        if (entity && entity->getEntity()) { // Only process actual entities
            Vec3 entityPos = entity->getPosition();
            
            // Check if entity is on any currently visible chunk
            if (isEntityOnVisibleChunk(entityPos)) {
                visibleEntities.push_back(entity.get());
            }
        }
    }
    
}

void Ground::repositionChunkToCoordinates(Chunk* chunk, const Vec2& newCoords) {
    if (!chunk) return;
    
    // Update chunk's internal coordinates
    chunk->setChunkPosition(newCoords);
    
    // Calculate new world position
    float worldX = static_cast<float>(newCoords.x) * static_cast<float>(chunkSize) * cubeSize;
    float worldZ = static_cast<float>(newCoords.y) * static_cast<float>(chunkSize) * cubeSize;
    
    // Position relative to ground's position
    Vec3 groundPos = getPosition();
    chunk->setPosition(Vec3(worldX + groundPos.x, groundPos.y, worldZ + groundPos.z));
    
    // Update chunk name to reflect new position
    std::string newName = "Chunk_" + std::to_string(newCoords.x) + "_" + std::to_string(newCoords.y);
    chunk->setName(newName);
    
    // Regenerate height map for new position (for future terrain generation)
    // For now, this will just reset to flat terrain
    chunk->regenerateHeightMap();
}

Vec2 Ground::getPlayerChunkCoordinates(const Vec3& playerPosition) const {
    // Convert player world position to chunk coordinates
    float chunkWorldSize = static_cast<float>(chunkSize) * cubeSize;
    
    int chunkX = static_cast<int>(std::floor(playerPosition.x / chunkWorldSize));
    int chunkZ = static_cast<int>(std::floor(playerPosition.z / chunkWorldSize));
    
    return Vec2(static_cast<float>(chunkX), static_cast<float>(chunkZ));
}

bool Ground::isEntityOnVisibleChunk(const Vec3& entityPosition) const {
    // Check if an entity is positioned on any currently visible chunk
    return isPositionOnVisibleChunk(entityPosition);
}

bool Ground::isPositionOnVisibleChunk(const Vec3& position) const {
    // Check if a position is within any currently active (visible) chunk
    for (const auto& chunk : chunks) {
        if (chunk->getActive()) {  // Only check visible chunks
            Vec3 chunkPos = chunk->getPosition();
            float chunkHalfSize = (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
            
            // Check if position is within this chunk's bounds
            if (position.x >= chunkPos.x - chunkHalfSize && 
                position.x <= chunkPos.x + chunkHalfSize &&
                position.z >= chunkPos.z - chunkHalfSize && 
                position.z <= chunkPos.z + chunkHalfSize) {
                return true;
            }
        }
    }
    return false;
}

} // namespace Engine
