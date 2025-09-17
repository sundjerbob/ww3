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
      lastUpdateTime(0.0f), objectRenderer(nullptr), owningScene(nullptr) {}

GameObject::~GameObject() {
    cleanup();
}

bool GameObject::initialize() {
    if (isInitialized) {
        return true;
    }
    
    
    // Setup mesh; shaders are owned/selected by the renderer implementation
    setupMesh();
    
    isInitialized = true;
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
        return;
    }

    const Mat4 modelMatrix = getModelMatrix();
    selectedRenderer->renderMesh(*mesh, modelMatrix, camera, getColor());
}

void GameObject::cleanup() {
    if (!isInitialized) return;
    
    
    mesh.reset();
    
    isInitialized = false;
}

Mat4 GameObject::getModelMatrix() const {
    // FIXED: Correct transformation order: Translation * Rotation * Scale
    Mat4 model = Mat4(); // Identity matrix
    
    // DEBUG: Add extensive logging for transformation steps
    static int debugCount = 0;
    debugCount++;
    bool shouldDebug = (debugCount % 300 == 0); // Every 5 seconds
    
    if (shouldDebug) {
        std::cout << "=== GETMODELMATRIX DEBUG (FIXED ORDER) ===" << std::endl;
        std::cout << "Object name: " << name << std::endl;
        std::cout << "Raw position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        std::cout << "Raw rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ") degrees" << std::endl;
        std::cout << "Raw scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")" << std::endl;
    }
    
    // STEP 1: Apply scale first
    Vec3 scaleVector = scale;
    Mat4 scaleMatrix = Engine::scale(scaleVector);
    model = model * scaleMatrix;
    
    if (shouldDebug) {
        std::cout << "After scale - matrix position: (" << model.m[12] << ", " << model.m[13] << ", " << model.m[14] << ")" << std::endl;
    }
    
    // STEP 2: Apply rotation (convert degrees to radians)
    if (rotation.x != 0.0f) {
        Mat4 rotX = Engine::rotateX(rotation.x * 3.14159f / 180.0f);
        model = model * rotX;
    }
    if (rotation.y != 0.0f) {
        Mat4 rotY = Engine::rotateY(rotation.y * 3.14159f / 180.0f);
        model = model * rotY;
        if (shouldDebug) {
            std::cout << "After Y rotation (" << rotation.y << " degrees) - matrix position: (" << model.m[12] << ", " << model.m[13] << ", " << model.m[14] << ")" << std::endl;
        }
    }
    if (rotation.z != 0.0f) {
        Mat4 rotZ = Engine::rotateZ(rotation.z * 3.14159f / 180.0f);
        model = model * rotZ;
    }
    
    // STEP 3: Apply translation last
    model = Engine::translate(model, position);
    
    if (shouldDebug) {
        std::cout << "After translation - final position: (" << model.m[12] << ", " << model.m[13] << ", " << model.m[14] << ")" << std::endl;
        std::cout << "Position difference from raw: (" << (model.m[12] - position.x) << ", " << (model.m[13] - position.y) << ", " << (model.m[14] - position.z) << ")" << std::endl;
        std::cout << "============================" << std::endl;
    }
    
    return model;
}

void GameObject::updateTransform() {
    // Base implementation - derived classes can override for custom behavior
}

void GameObject::setupMesh() {
    // Base implementation - derived classes should override
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
