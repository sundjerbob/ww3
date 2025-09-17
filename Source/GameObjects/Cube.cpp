/**
 * Cube.cpp - Implementation of 3D Cube GameObject
 */

#include "Cube.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include <iostream>

namespace Engine {

Cube::Cube(const std::string& name, const Vec3& cubeColor)
    : GameObject(name), isRotating(false), rotationSpeed(90.0f) {
    // Set the color using the base class method
    setColor(cubeColor);
    
    // Set default scale for cube
    setScale(Vec3(1.0f, 1.0f, 1.0f));
    
    // Mark as entity for chunk-based rendering
    setEntity(true);
}

void Cube::update(float deltaTime) {
    // Call base class update
    GameObject::update(deltaTime);
    
    // Handle rotation animation
    if (isRotating) {
        Vec3 currentRotation = getRotation();
        currentRotation.y += rotationSpeed * deltaTime;
        
        // Keep rotation in 0-360 range
        if (currentRotation.y >= 360.0f) {
            currentRotation.y -= 360.0f;
        }
        
        setRotation(currentRotation);
    }
}

void Cube::setupMesh() {
    // Create cube vertices (8 vertices for a cube)
    std::vector<float> vertices = {
        // Front face
        -0.5f, -0.5f,  0.5f,  // 0: bottom-left-front
         0.5f, -0.5f,  0.5f,  // 1: bottom-right-front
         0.5f,  0.5f,  0.5f,  // 2: top-right-front
        -0.5f,  0.5f,  0.5f,  // 3: top-left-front
        
        // Back face
        -0.5f, -0.5f, -0.5f,  // 4: bottom-left-back
         0.5f, -0.5f, -0.5f,  // 5: bottom-right-back
         0.5f,  0.5f, -0.5f,  // 6: top-right-back
        -0.5f,  0.5f, -0.5f   // 7: top-left-back
    };
    
    // Create cube indices (12 triangles = 36 indices)
    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Back face
        5, 4, 7,  7, 6, 5,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Right face
        1, 5, 6,  6, 2, 1,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        4, 5, 1,  1, 0, 4
    };
    
    // Create mesh
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMesh(vertices, indices)) {
    } else {
    }
}

void Cube::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;
    Mat4 modelMatrix = getModelMatrix();
    renderer.renderMesh(*mesh, modelMatrix, camera, getColor());
}

} // namespace Engine
