/**
 * Chunk.cpp - Implementation of Terrain Chunk GameObject
 */

#include "Chunk.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"   
#include "../Engine/Math/Math.h"
#include <iostream>
#include <cmath>

namespace Engine {

Chunk::Chunk(const std::string& name, const Vec2& position, int size, float cubeSize)
    : GameObject(name), chunkPosition(position), chunkSize(size), cubeSize(cubeSize) {
    
    // Set chunk position in world space
    float worldX = static_cast<float>(position.x) * static_cast<float>(size) * cubeSize;
    float worldZ = static_cast<float>(position.y) * static_cast<float>(size) * cubeSize;
    setPosition(Vec3(worldX, 0.0f, worldZ));
    
    // Initialize height map
    heightMap.resize(size, std::vector<float>(size, 0.0f));
    generateHeightMap();
    
    // Set default color using base class method
    setColor(Vec3(0.4f, 0.3f, 0.2f));
}

void Chunk::generateHeightMap() {
    // Simple flat terrain for now - all heights are 0
    // This will be expanded later with proper terrain generation
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            heightMap[x][z] = 0.0f;
        }
    }
}

void Chunk::regenerateHeightMap() {
    // Regenerate height map for new chunk position
    // This will be expanded later with proper terrain generation based on chunk coordinates
    for (int x = 0; x < chunkSize; x++) {
        for (int z = 0; z < chunkSize; z++) {
            heightMap[x][z] = 0.0f;
        }
    }
}

float Chunk::getHeightAt(int x, int z) const {
    if (x >= 0 && x < chunkSize && z >= 0 && z < chunkSize) {
        return heightMap[x][z];
    }
    return 0.0f;
}

bool Chunk::isInRenderDistance(const Vec3& playerPosition, float renderDistance) const {
    // Calculate distance from player to chunk center
    Vec3 chunkCenter = getPosition();
    chunkCenter.x += (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
    chunkCenter.z += (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
    
    float distance = sqrt(
        static_cast<float>((playerPosition.x - chunkCenter.x) * (playerPosition.x - chunkCenter.x) +
        (playerPosition.z - chunkCenter.z) * (playerPosition.z - chunkCenter.z))
    );
    
    return distance <= renderDistance;
}

void Chunk::setupMesh() {
    // For now, create a simple flat mesh like the original ground
    // This will be expanded to create actual cube geometry later
    
    float halfSize = (static_cast<float>(chunkSize) * cubeSize) / 2.0f;
    std::vector<float> vertices = {
        // Ground plane (facing up)
        -halfSize, 0.0f, -halfSize,  // 0: bottom-left
         halfSize, 0.0f, -halfSize,  // 1: bottom-right
         halfSize, 0.0f,  halfSize,  // 2: top-right
        -halfSize, 0.0f,  halfSize   // 3: top-left
    };
    
    // Create ground indices (2 triangles = 6 indices)
    std::vector<unsigned int> indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    // Create mesh
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMesh(vertices, indices)) {
        std::cerr << "Failed to create chunk mesh for '" << getName() << "'" << std::endl;
    } else {
        std::cout << "Created chunk mesh for '" << getName() << "' at position (" 
                  << chunkPosition.x << ", " << chunkPosition.y << ")" << std::endl;
    }
}

void Chunk::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid() || !mesh) {
        std::cout << "Chunk " << getName() << " not rendering: active=" << getActive() 
                  << ", valid=" << isValid() << ", mesh=" << (mesh ? "yes" : "no") << std::endl;
        return;
    }
    Mat4 modelMatrix = getModelMatrix();
    
    // Use individual colors for all chunks (no height-based coloring)
    renderer.renderMesh(*mesh, modelMatrix, camera, getColor());
}

} // namespace Engine
