/**
 * GameObject.cpp - Implementation of Base Game Object Class
 */

#include "GameObject.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/Renderer.h"
#include <iostream>

namespace Engine {

GameObject::GameObject(const std::string& objectName)
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f),
      name(objectName), isActive(true), isInitialized(false), lastUpdateTime(0.0f),
      objectRenderer(nullptr) {}

GameObject::~GameObject() {
    cleanup();
}

bool GameObject::initialize() {
    if (isInitialized) {
        std::cout << "GameObject '" << name << "' already initialized" << std::endl;
        return true;
    }
    
    std::cout << "Initializing GameObject '" << name << "'..." << std::endl;
    
    // Setup mesh; shaders are owned/selected by the renderer implementation
    setupMesh();
    
    isInitialized = true;
    std::cout << "GameObject '" << name << "' initialized successfully" << std::endl;
    return true;
}

void GameObject::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    lastUpdateTime += deltaTime;
    updateTransform();
}

void GameObject::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;

    const Renderer& selectedRenderer = (objectRenderer != nullptr) ? *objectRenderer : renderer;
    const Mat4 modelMatrix = getModelMatrix();
    const Vec3 defaultColor(1.0f, 1.0f, 1.0f);
    selectedRenderer.renderMesh(*mesh, modelMatrix, camera, defaultColor);
}

void GameObject::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up GameObject '" << name << "'..." << std::endl;
    
    mesh.reset();
    
    isInitialized = false;
    std::cout << "GameObject '" << name << "' cleanup complete" << std::endl;
}

Mat4 GameObject::getModelMatrix() const {
    // Create transformation matrix: scale * rotation * translation
    Mat4 model = Mat4(); // Identity matrix
    
    // Apply translation
    model = Engine::translate(model, position);
    
    // Apply rotation (convert degrees to radians)
    if (rotation.x != 0.0f) {
        Mat4 rotX = Engine::rotateX(rotation.x * 3.14159f / 180.0f);
        model = Engine::multiply(model, rotX);
    }
    if (rotation.y != 0.0f) {
        Mat4 rotY = Engine::rotateY(rotation.y * 3.14159f / 180.0f);
        model = Engine::multiply(model, rotY);
    }
    if (rotation.z != 0.0f) {
        Mat4 rotZ = Engine::rotateZ(rotation.z * 3.14159f / 180.0f);
        model = Engine::multiply(model, rotZ);
    }
    
    // Apply scale
    Vec3 scaleVector = scale;
    Mat4 scaleMatrix = Engine::scale(scaleVector);
    model = Engine::multiply(model, scaleMatrix);
    
    return model;
}

void GameObject::updateTransform() {
    // Base implementation - derived classes can override for custom behavior
}

void GameObject::setupMesh() {
    // Base implementation - derived classes should override
    std::cout << "Warning: GameObject '" << name << "' using default mesh setup" << std::endl;
}

} // namespace Engine
