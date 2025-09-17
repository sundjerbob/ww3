/**
 * Arrow.cpp - Implementation of 2D Direction Arrow for Minimap
 */

#include "Arrow.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Math/Math.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace Engine {

Arrow::Arrow(const std::string& name, float size, const Vec3& arrowColor)
    : GameObject(name), color(arrowColor), arrowSize(size), arrowLength(2.0f), arrowWidth(0.8f) {
    
    // Arrow will be positioned at world center (0,0,0) and stay fixed there
    // Only rotation will change to point in camera direction
    
    // Mark as system object (not an entity)
    setEntity(false);
}

void Arrow::setDirection(const Vec3& direction) {
    // Calculate yaw angle from direction vector
    float yaw = atan2(static_cast<float>(direction.x), static_cast<float>(direction.z)) * 180.0f / 3.14159f;
    setDirectionFromYaw(yaw);
}

void Arrow::setDirectionFromYaw(float yawDegrees) {
    // For minimap top-down view, we rotate around Y-axis (up/down)
    // This makes the arrow rotate properly in the 2D minimap view when viewed from above
    // The Y-axis rotation will appear as Z-axis rotation in the top-down view
    setRotation(Vec3(0.0f, yawDegrees, 0.0f));
}

void Arrow::setupMesh() {
    create2DArrowMesh();
}

void Arrow::create2DArrowMesh() {
    // Create a 2D arrow shape designed for top-down orthographic view
    // The arrow is flat (Y = 0) and points in the positive Z direction by default
    // IMPORTANT: Arrow is centered around origin (0,0,0) so it rotates around its own center
    // When viewed from above, this will appear as a proper 2D arrow
    
    float length = arrowLength * arrowSize;
    float width = arrowWidth * arrowSize;
    
    
    std::vector<float> vertices = {
        // Simple arrow shape pointing forward - Y = 0 for flat 2D appearance
        // Centered around origin for proper rotation
        0.0f, 0.0f, 0.6f,                    // 0: tip of arrow
        -0.3f, 0.0f, -0.4f,                  // 1: left base
         0.3f, 0.0f, -0.4f                   // 2: right base
    };
    
    std::vector<unsigned int> indices = {
        // Single triangle
        0, 1, 2
    };
    
    // Create mesh
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMesh(vertices, indices)) {
    } else {
    }
}

void Arrow::render(const Renderer& renderer, const Camera& camera) {
    if (!getActive() || !isValid() || !mesh) {
        return;
    }
    
    // For minimap rendering, we want to ensure the arrow is rendered with its color
    Mat4 modelMatrix = getModelMatrix();
    renderer.renderMesh(*mesh, modelMatrix, camera, color);
}

} // namespace Engine
