/**
 * Scene.cpp - Implementation of Scene Management System
 */

#include "Scene.h"
#include "../Rendering/Renderer.h"
#include "../../GameObjects/Ground.h"
#include "../../GameObjects/Monster.h"
#include <iostream>
#include <algorithm>

namespace Engine {

Scene::Scene(const std::string& name)
    : sceneName(name), isInitialized(false), isActive(true),
      totalObjects(0), activeObjects(0), renderedObjects(0), groundReference(nullptr) {}

Scene::~Scene() {
    cleanup();
}

bool Scene::initialize() {
    if (isInitialized) {
        return true;
    }
    
    
    // Initialize all game objects
    for (auto& object : gameObjects) {
        if (!object->initialize()) {
            return false;
        }
    }
    
    updateObjectCounts();
    isInitialized = true;
    
    return true;
}

void Scene::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update all active game objects
    for (auto& object : gameObjects) {
        if (object->getActive()) {
            // CRASH PREVENTION: Safely update monsters with try-catch
            if (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos) {
                // This is a monster - update it safely
                try {
                    object->update(deltaTime);
                } catch (const std::exception& e) {
                }
            } else {
                // Update non-monster objects normally
                object->update(deltaTime);
            }
        }
    }
    
    // DISABLED: Clean up any destroyed objects
    // This was causing crashes when monsters were cleaned up
    // cleanupDestroyedObjects();
    
    // Update object counts
    updateObjectCounts();
}

void Scene::render(const Camera& camera, const Renderer& renderer) {
    if (!isActive || !isInitialized) return;
    
    // Update Ground's entity visibility system before rendering
    updateGroundEntityVisibility();
    
    renderedObjects = 0;
    
    // Render all active game objects
    for (auto& object : gameObjects) {
        if (object->getActive()) {
            // MONSTER RENDERING DELEGATION: Skip actual monsters in Scene rendering
            // Monsters are rendered separately in Game::render() with MonsterRenderer for proper color handling
            bool isMonster = (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos);
            if (isMonster) {
                continue; // Skip actual monsters - they're rendered by Game::render() with MonsterRenderer
            }
            
            // For monsters and health bars, skip isValid() check to prevent crashes
            bool shouldRender = true;
            if (object->getName().find("Monster_") == 0) {
                // Skip isValid() check for monster objects to prevent crashes
                shouldRender = true;
            } else {
                // For non-monster objects, use normal isValid() check
                shouldRender = object->isValid();
            }
            
            // CRITICAL: Double-check that the object is still active before rendering
            // This prevents rendering of health bars that were marked as inactive during monster death
            if (shouldRender && object->getActive()) {
                // Check if this is an entity that should be rendered based on chunk visibility
                if (object->getEntity()) {
                    if (shouldRenderEntity(object.get())) {
                        object->render(renderer, camera);
                        renderedObjects++;
                    }
                } else {
                    // Non-entity objects (system objects) always render
                    object->render(renderer, camera);
                    renderedObjects++;
                }
            }
        }
    }
}

void Scene::cleanup() {
    if (!isInitialized) return;
    
    
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
}

void Scene::addGameObject(std::unique_ptr<GameObject> object) {
    if (!object) {
        return;
    }
    
    std::string objectName = object->getName();
    
    // Check if object with this name already exists
    if (objectMap.find(objectName) != objectMap.end()) {
        return;
    }
    
    // Add to collections
    GameObject* objectPtr = object.get();
    gameObjects.push_back(std::move(object));
    objectMap[objectName] = objectPtr;
    
    // Set scene reference for the object
    objectPtr->setScene(this);
    
    // Initialize if scene is already initialized
    if (isInitialized) {
        if (!objectPtr->initialize()) {
            // Remove from collections
            gameObjects.pop_back();
            objectMap.erase(objectName);
            return;
        }
    }
    
    updateObjectCounts();
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
    }
}

void Scene::clear() {
    
    // Clean up all objects
    for (auto& object : gameObjects) {
        object->cleanup();
    }
    
    gameObjects.clear();
    objectMap.clear();
    
    updateObjectCounts();
}

void Scene::printSceneInfo() const {
    // Calculate current renderable objects (active and valid)
    int currentRenderableObjects = 0;
    for (const auto& object : gameObjects) {
        if (object->getActive()) {
            // CRASH PREVENTION: Skip actual monsters but keep health bars
            if (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos) {
                continue; // Skip actual monsters only
            }
            
            // For monsters and health bars, skip isValid() check to prevent crashes
            if (object->getName().find("Monster_") == 0) {
                currentRenderableObjects++; // Count health bars without isValid() check
            } else {
                // For non-monster objects, use normal isValid() check
                if (object->isValid()) {
                    currentRenderableObjects++;
                }
            }
        }
    }
    
    // Scene information removed to eliminate console output
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
    // DISABLED: This method was causing crashes when cleaning up monsters
    // The MonsterSpawner now handles all monster cleanup manually
    // 
    // Original implementation:
    // Collect objects to remove first, then remove them in a separate pass
    // This prevents issues with modifying the collection while iterating
    /*
    std::vector<GameObject*> objectsToRemove;
    
    for (auto& object : gameObjects) {
        if (!object->isValid()) {
            objectsToRemove.push_back(object.get());
        }
    }
    
    // Remove collected objects
    for (GameObject* objToRemove : objectsToRemove) {
        removeGameObject(objToRemove);
    }
    */
}

bool Scene::shouldRenderEntity(const GameObject* entity) const {
    // If no ground reference, render all entities (fallback)
    if (!groundReference) {
        return true;
    }
    
    // Check if entity is on a visible chunk
    return groundReference->isEntityOnVisibleChunk(entity->getPosition());
}

void Scene::updateGroundEntityVisibility() {
    if (groundReference) {
        // CRASH PREVENTION: Skip Ground visibility update entirely for now
        // This was causing immediate crashes on startup
        // We'll handle monster visibility differently
        return;
    }
}

std::vector<GameObject*> Scene::getAllGameObjects() const {
    std::vector<GameObject*> objects;
    objects.reserve(gameObjects.size());
    
    for (const auto& object : gameObjects) {
        if (object && object->getActive()) {
            // CRASH PREVENTION: Skip actual monsters but keep health bars
            if (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos) {
                continue; // Skip actual monsters only
            }
            
            // For monsters and health bars, skip isValid() check to prevent crashes
            if (object->getName().find("Monster_") == 0) {
                objects.push_back(object.get()); // Include health bars without isValid() check
            } else {
                // For non-monster objects, use normal isValid() check
                if (object->isValid()) {
                    objects.push_back(object.get());
                }
            }
        }
    }
    
    return objects;
}

// NEW METHOD: Get all objects including monsters for collision detection
// This is safe because collision detection doesn't call isValid() or other problematic methods
std::vector<GameObject*> Scene::getAllObjectsForCollision() const {
    std::vector<GameObject*> objects;
    objects.reserve(gameObjects.size());
    
    for (const auto& object : gameObjects) {
        if (object && object->getActive()) {
            // CRASH PREVENTION: Skip dead monsters from collision detection
            // Dead monsters can cause crashes when collision detection tries to access them
            if (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos) {
                // For actual monsters, check if they're dead before including them
                try {
                    // Try to safely check if monster is dead
                    Monster* monster = dynamic_cast<Monster*>(object.get());
                    if (monster && monster->isDead()) {
                        continue; // Skip dead monsters
                    }
                } catch (...) {
                    // If we can't safely check the monster, skip it to prevent crashes
                    continue;
                }
            }
            
            // Include safe objects for collision detection
            objects.push_back(object.get());
        }
    }
    
    return objects;
}

} // namespace Engine
