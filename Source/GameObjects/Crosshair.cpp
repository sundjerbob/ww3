/**
 * Crosshair.cpp - GameObject representing a 2D crosshair overlay
 */

#include "Crosshair.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include <iostream>

namespace Engine {

Crosshair::Crosshair(const std::string& name)
    : GameObject(name),
      recoilOffset(0.0f, 0.0f, 0.0f),
      recoilVelocity(0.0f, 0.0f, 0.0f),
      recoilRecoveryRate(4.0f) {
    // Mark as system object (not an entity)
    setEntity(false);
}

bool Crosshair::initialize() {
    if (isInitialized) return true;
    setupMesh();
    isInitialized = true;
    return true;
}

void Crosshair::setupMesh() {
    // Build two thin quads for a crosshair in NDC around origin
    const float crosshairSize = 0.05f;  // Increased size to make it more visible
    const float crosshairThickness = 0.005f;  // Increased thickness

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Horizontal
    vertices.insert(vertices.end(), {-crosshairSize, crosshairThickness, 0.0f});
    vertices.insert(vertices.end(), { crosshairSize, crosshairThickness, 0.0f});
    vertices.insert(vertices.end(), { crosshairSize,-crosshairThickness, 0.0f});
    vertices.insert(vertices.end(), {-crosshairSize,-crosshairThickness, 0.0f});

    // Vertical
    vertices.insert(vertices.end(), {-crosshairThickness,-crosshairSize, 0.0f});
    vertices.insert(vertices.end(), { crosshairThickness,-crosshairSize, 0.0f});
    vertices.insert(vertices.end(), { crosshairThickness, crosshairSize, 0.0f});
    vertices.insert(vertices.end(), {-crosshairThickness, crosshairSize, 0.0f});

    indices.insert(indices.end(), {0,1,2, 0,2,3});
    indices.insert(indices.end(), {4,5,6, 4,6,7});

    mesh = std::make_unique<Mesh>();
    mesh->createMesh(vertices, indices);
}

void Crosshair::update(float deltaTime) {
    GameObject::update(deltaTime);
    
    // Update recoil system
    updateRecoil(deltaTime);
}

void Crosshair::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;
    // Render as white; overlay renderer ignores camera and draws in NDC
    // The base class will automatically use the CrosshairRenderer via the factory
    GameObject::render(renderer, camera);
}

// Recoil methods
void Crosshair::applyRecoil(const Vec3& recoil) {
    // CAMERA-BASED RECOIL (crosshair movement, unlimited, matches weapon rotation)
    // Scale recoil for screen space coordinates (NDC) - matches weapon rotation
    float cameraRecoil = recoil.y * 0.8f;  // Same scale as weapon rotation recoil
    recoilOffset.y += cameraRecoil;  // Positive for crosshair (moves up when weapon tilts up)
    
    // Set recoil velocity for smooth movement
    recoilVelocity.y = recoil.y * 1.0f; // Positive velocity to match weapon upward rotation
    
}

void Crosshair::updateRecoil(float deltaTime) {
    // CAMERA RECOIL RECOVERY (unlimited, always returns to center) - FAST RECOVERY
    if (recoilOffset.y > 0.0f) {
        recoilOffset.y -= recoilRecoveryRate * 8.0f * deltaTime; // 8x faster recovery for crosshair
        recoilOffset.y = std::max(0.0f, recoilOffset.y);
    } else if (recoilOffset.y < 0.0f) {
        recoilOffset.y += recoilRecoveryRate * 8.0f * deltaTime; // 8x faster recovery for crosshair
        recoilOffset.y = std::min(0.0f, recoilOffset.y);
    }
    
    // Update recoil velocity for smooth movement - much stronger damping
    recoilVelocity.y *= (1.0f - deltaTime * 12.0f); // Much stronger damping for faster return
    
    // Update crosshair position with recoil offset
    Vec3 finalPosition = Vec3(0.0f, 0.0f, 0.0f) + recoilOffset;
    setPosition(finalPosition);
}

} // namespace Engine


