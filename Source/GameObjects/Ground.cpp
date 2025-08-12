/**
 * Ground.cpp - Implementation of Ground Plane GameObject
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
    
    // Set ground scale based on size
    setScale(Vec3(size, 1.0f, size));
}

void Ground::setupMesh() {
    // Create ground plane vertices (4 vertices for a quad)
    float halfSize = 0.5f; // Base size, will be scaled by transform
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
        std::cerr << "Failed to create ground mesh for '" << getName() << "'" << std::endl;
    } else {
        std::cout << "Created ground mesh for '" << getName() << "' (size: " << size << ")" << std::endl;
    }
}

void Ground::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;
    Mat4 modelMatrix = getModelMatrix();
    renderer.renderMesh(*mesh, modelMatrix, camera, color);
}

} // namespace Engine
