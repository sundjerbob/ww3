/**
 * Crosshair.cpp - GameObject representing a 2D crosshair overlay
 */

#include "Crosshair.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"

namespace Engine {

Crosshair::Crosshair(const std::string& name)
    : GameObject(name) {}

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

void Crosshair::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;
    // Use the assigned renderer (CrosshairRenderer) instead of the passed renderer
    const Renderer& selectedRenderer = (objectRenderer != nullptr) ? *objectRenderer : renderer;
    // Render as white; overlay renderer ignores camera and draws in NDC
    selectedRenderer.renderMesh(*mesh, Mat4(), camera, Vec3(1.0f, 1.0f, 1.0f));
}

} // namespace Engine


