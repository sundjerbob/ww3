/**
 * Ground.cpp - Implementation of Chunk-based Ground GameObject
 */

#include "Ground.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include <iostream>

namespace Engine {

Ground::Ground(const std::string& name, float groundSize, const Vec3& groundColor)
    : GameObject(name), color(groundColor), size(groundSize) {
    // Set ground position (below other objects)
    setPosition(Vec3(0.0f, -2.0f, 0.0f));
    
    // Initialize chunk system
    chunkSize = 16;  // 16x16 cubes per chunk
    cubeSize = 1.0f; // Each cube is 1 unit
    renderDistance = 50.0f; // Render chunks within 50 units
}

void Ground::generateInitialChunks() {
    // Create a 3x3 grid of chunks to cover the same area as the original ground
    // Each chunk is 16x16 units, so 3x3 chunks = 48x48 units total
    
    for (int x = -1; x <= 1; x++) {
        for (int z = -1; z <= 1; z++) {
            std::string chunkName = "Chunk_" + std::to_string(x) + "_" + std::to_string(z);
            auto chunk = std::make_unique<Chunk>(chunkName, Vec2(x, z), chunkSize, cubeSize);
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
    }
    
    std::cout << "Generated " << chunks.size() << " chunks for ground '" << getName() << "'" << std::endl;
}

void Ground::updateChunkVisibility(const Vec3& playerPosition) {
    // For now, just render all chunks
    // This will be expanded to only render chunks within render distance
    for (auto& chunk : chunks) {
        chunk->setActive(true);
    }
}

void Ground::setupMesh() {
    // Ground no longer needs its own mesh - it uses chunks
    // Generate initial chunks here when the Ground is properly initialized
    generateInitialChunks();
    std::cout << "Ground '" << getName() << "' uses chunk-based rendering" << std::endl;
}

void Ground::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid()) {
        std::cout << "Ground not active or not valid" << std::endl;
        return;
    }
    
    // Update chunk visibility based on player position
    Vec3 playerPosition = camera.getPosition();
    updateChunkVisibility(playerPosition);
    
    // Render all active chunks
    int renderedChunks = 0;
    for (auto& chunk : chunks) {
        if (chunk->getActive()) {
            chunk->render(renderer, camera);
            renderedChunks++;
        }
    }
    
    // Debug output (only print occasionally to avoid spam)
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) { // Print every 60 frames (about once per second)
        std::cout << "Ground rendering " << renderedChunks << " chunks out of " << chunks.size() << " total chunks" << std::endl;
    }
}

} // namespace Engine
