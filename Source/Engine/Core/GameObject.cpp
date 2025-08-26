/**
 * GameObject.cpp - Implementation of Base Game Object Class
 */

#include "GameObject.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/Renderer.h"
#include <iostream>
#include <algorithm>

namespace Engine {

GameObject::GameObject(const std::string& objectName)
    : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f),
      parent(nullptr), color(1.0f, 1.0f, 1.0f), name(objectName), isActive(true), isInitialized(false), isEntity(false), 
      lastUpdateTime(0.0f), objectRenderer(nullptr) {}

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

    // Get the appropriate renderer for this object type
    Renderer* selectedRenderer = nullptr;
    if (objectRenderer != nullptr) {
        // Use explicitly assigned renderer (legacy support)
        selectedRenderer = objectRenderer;
    } else {
        // Use the factory to get the preferred renderer for this object type
        selectedRenderer = RendererFactory::getInstance().getRenderer(getPreferredRendererType());
    }
    
    if (!selectedRenderer) {
        std::cerr << "Warning: No renderer available for GameObject '" << name << "'" << std::endl;
        return;
    }

    const Mat4 modelMatrix = getModelMatrix();
    selectedRenderer->renderMesh(*mesh, modelMatrix, camera, getColor());
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
        model = model * rotX;
    }
    if (rotation.y != 0.0f) {
        Mat4 rotY = Engine::rotateY(rotation.y * 3.14159f / 180.0f);
        model = model * rotY;
    }
    if (rotation.z != 0.0f) {
        Mat4 rotZ = Engine::rotateZ(rotation.z * 3.14159f / 180.0f);
        model = model * rotZ;
    }
    
    // Apply scale
    Vec3 scaleVector = scale;
    Mat4 scaleMatrix = Engine::scale(scaleVector);
    model = model * scaleMatrix;
    
    return model;
}

void GameObject::updateTransform() {
    // Base implementation - derived classes can override for custom behavior
}

void GameObject::setupMesh() {
    // Base implementation - derived classes should override
    std::cout << "Warning: GameObject '" << name << "' using default mesh setup" << std::endl;
}

// Parent-child system implementation
void GameObject::addChild(std::unique_ptr<GameObject> child) {
    if (child) {
        child->setParent(this);
        child->setRenderer(objectRenderer); // Pass renderer to child
        children.push_back(std::move(child));
    }
}

void GameObject::removeChild(GameObject* child) {
    if (!child) return;
    
    auto it = std::find_if(children.begin(), children.end(),
                          [child](const std::unique_ptr<GameObject>& ptr) {
                              return ptr.get() == child;
                          });
    
    if (it != children.end()) {
        (*it)->setParent(nullptr);
        children.erase(it);
    }
}

void GameObject::setParent(GameObject* newParent) {
    parent = newParent;
}

// World transform calculations
Vec3 GameObject::getWorldPosition() const {
    if (parent) {
        // Transform local position by parent's world transform
        Mat4 parentWorldMatrix = parent->getWorldModelMatrix();
        Vec4 worldPos = parentWorldMatrix * Vec4(position.x, position.y, position.z, 1.0f);
        return Vec3(worldPos.x, worldPos.y, worldPos.z);
    }
    return position;
}

Vec3 GameObject::getWorldRotation() const {
    if (parent) {
        Vec3 parentRotation = parent->getWorldRotation();
        return Vec3(parentRotation.x + rotation.x, 
                   parentRotation.y + rotation.y, 
                   parentRotation.z + rotation.z);
    }
    return rotation;
}

Vec3 GameObject::getWorldScale() const {
    if (parent) {
        Vec3 parentScale = parent->getWorldScale();
        return Vec3(parentScale.x * scale.x, 
                   parentScale.y * scale.y, 
                   parentScale.z * scale.z);
    }
    return scale;
}

Mat4 GameObject::getWorldModelMatrix() const {
    if (parent) {
        return parent->getWorldModelMatrix() * getModelMatrix();
    }
    return getModelMatrix();
}

} // namespace Engine
