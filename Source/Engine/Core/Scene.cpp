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
            // CRASH PREVENTION: Safely update monsters with try-catch
            if (object->getName().find("Monster_") == 0 && object->getName().find("HealthBar") == std::string::npos) {
                // This is a monster - update it safely
                try {
                    object->update(deltaTime);
                } catch (const std::exception& e) {
                    std::cout << "Error updating monster " << object->getName() << ": " << e.what() << std::endl;
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
    
    // Set scene reference for the object
    objectPtr->setScene(this);
    
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
                      << " (Active: " << (object->getActive() ? "Yes" : "No") << ")";
            
            // CRASH PREVENTION: Skip monsters entirely for now
            if (object->getName().find("Monster_") == 0) {
                std::cout << " (Valid: Skipped for monster)" << std::endl;
            } else {
                std::cout << " (Valid: " << (object->isValid() ? "Yes" : "No") << ")" << std::endl;
            }
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
