/**
 * Scene.cpp - Implementation of Scene Management System
 */

#include "Scene.h"
#include "../Rendering/Renderer.h"
#include <iostream>
#include <algorithm>

namespace Engine {

Scene::Scene(const std::string& name)
    : sceneName(name), isInitialized(false), isActive(true),
      totalObjects(0), activeObjects(0), renderedObjects(0) {}

Scene::~Scene() {
    cleanup();
}

bool Scene::initialize() {
    if (isInitialized) {
        std::cout << "Scene '" << sceneName << "' already initialized" << std::endl;
        return true;
    }
    
    std::cout << "Initializing Scene '" << sceneName << "'..." << std::endl;
    
    // Initialize all game objects
    for (auto& object : gameObjects) {
        if (!object->initialize()) {
            std::cerr << "Failed to initialize GameObject '" << object->getName() << "'" << std::endl;
            return false;
        }
    }
    
    updateObjectCounts();
    isInitialized = true;
    
    std::cout << "Scene '" << sceneName << "' initialized successfully with " 
              << totalObjects << " objects" << std::endl;
    return true;
}

void Scene::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update all active game objects
    for (auto& object : gameObjects) {
        if (object->getActive()) {
            object->update(deltaTime);
        }
    }
    
    // Clean up any destroyed objects
    cleanupDestroyedObjects();
    
    // Update object counts
    updateObjectCounts();
}

void Scene::render(const Camera& camera, const Renderer& renderer) {
    if (!isActive || !isInitialized) return;
    
    renderedObjects = 0;
    
    // Render all active game objects
    for (auto& object : gameObjects) {
        if (object->getActive() && object->isValid()) {
            object->render(renderer, camera);
            renderedObjects++;
        }
    }
}

void Scene::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up Scene '" << sceneName << "'..." << std::endl;
    
    // Clean up all game objects
    for (auto& object : gameObjects) {
        object->cleanup();
    }
    
    gameObjects.clear();
    objectMap.clear();
    
    totalObjects = 0;
    activeObjects = 0;
    renderedObjects = 0;
    
    isInitialized = false;
    std::cout << "Scene '" << sceneName << "' cleanup complete" << std::endl;
}

void Scene::addGameObject(std::unique_ptr<GameObject> object) {
    if (!object) {
        std::cerr << "Attempted to add null GameObject to scene" << std::endl;
        return;
    }
    
    std::string objectName = object->getName();
    
    // Check if object with this name already exists
    if (objectMap.find(objectName) != objectMap.end()) {
        std::cerr << "GameObject with name '" << objectName << "' already exists in scene" << std::endl;
        return;
    }
    
    // Add to collections
    GameObject* objectPtr = object.get();
    gameObjects.push_back(std::move(object));
    objectMap[objectName] = objectPtr;
    
    // Initialize if scene is already initialized
    if (isInitialized) {
        if (!objectPtr->initialize()) {
            std::cerr << "Failed to initialize GameObject '" << objectName << "'" << std::endl;
            // Remove from collections
            gameObjects.pop_back();
            objectMap.erase(objectName);
            return;
        }
    }
    
    updateObjectCounts();
    std::cout << "Added GameObject '" << objectName << "' to scene '" << sceneName << "'" << std::endl;
}

GameObject* Scene::getGameObject(const std::string& name) {
    auto it = objectMap.find(name);
    if (it != objectMap.end()) {
        return it->second;
    }
    return nullptr;
}

void Scene::removeGameObject(const std::string& name) {
    auto it = objectMap.find(name);
    if (it != objectMap.end()) {
        removeGameObject(it->second);
    }
}

void Scene::removeGameObject(GameObject* object) {
    if (!object) return;
    
    // Find and remove from vector
    auto it = std::find_if(gameObjects.begin(), gameObjects.end(),
                          [object](const std::unique_ptr<GameObject>& ptr) {
                              return ptr.get() == object;
                          });
    
    if (it != gameObjects.end()) {
        std::string objectName = object->getName();
        
        // Clean up the object
        object->cleanup();
        
        // Remove from collections
        gameObjects.erase(it);
        objectMap.erase(objectName);
        
        updateObjectCounts();
        std::cout << "Removed GameObject '" << objectName << "' from scene '" << sceneName << "'" << std::endl;
    }
}

void Scene::clear() {
    std::cout << "Clearing Scene '" << sceneName << "'..." << std::endl;
    
    // Clean up all objects
    for (auto& object : gameObjects) {
        object->cleanup();
    }
    
    gameObjects.clear();
    objectMap.clear();
    
    updateObjectCounts();
    std::cout << "Scene '" << sceneName << "' cleared" << std::endl;
}

void Scene::printSceneInfo() const {
    // Calculate current renderable objects (active and valid)
    int currentRenderableObjects = 0;
    for (const auto& object : gameObjects) {
        if (object->getActive() && object->isValid()) {
            currentRenderableObjects++;
        }
    }
    
    std::cout << "\n=== Scene Information ===" << std::endl;
    std::cout << "Scene Name: " << sceneName << std::endl;
    std::cout << "Initialized: " << (isInitialized ? "Yes" : "No") << std::endl;
    std::cout << "Active: " << (isActive ? "Yes" : "No") << std::endl;
    std::cout << "Total Objects: " << totalObjects << std::endl;
    std::cout << "Active Objects: " << activeObjects << std::endl;
    std::cout << "Renderable Objects: " << currentRenderableObjects << std::endl;
    std::cout << "Last Rendered Objects: " << renderedObjects << std::endl;
    
    if (!gameObjects.empty()) {
        std::cout << "\nGameObjects:" << std::endl;
        for (const auto& object : gameObjects) {
            std::cout << "  - " << object->getName() 
                      << " (Active: " << (object->getActive() ? "Yes" : "No") << ")"
                      << " (Valid: " << (object->isValid() ? "Yes" : "No") << ")" << std::endl;
        }
    }
    std::cout << "========================\n" << std::endl;
}

void Scene::updateObjectCounts() {
    totalObjects = static_cast<int>(gameObjects.size());
    activeObjects = 0;
    
    for (const auto& object : gameObjects) {
        if (object->getActive()) {
            activeObjects++;
        }
    }
}

void Scene::cleanupDestroyedObjects() {
    // Remove objects that are no longer valid
    auto it = gameObjects.begin();
    while (it != gameObjects.end()) {
        if (!(*it)->isValid()) {
            std::string objectName = (*it)->getName();
            objectMap.erase(objectName);
            it = gameObjects.erase(it);
            std::cout << "Removed destroyed GameObject '" << objectName << "'" << std::endl;
        } else {
            ++it;
        }
    }
}

} // namespace Engine
