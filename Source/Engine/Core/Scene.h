/**
 * Scene.h - Scene Management System
 * 
 * OVERVIEW:
 * Manages all GameObjects in the scene and handles their lifecycle.
 * Provides centralized scene management for rendering and updates.
 * 
 * FEATURES:
 * - GameObject collection management
 * - Scene-wide update and render cycles
 * - Object lifecycle management
 * - Scene state management
 */

#pragma once
#include "GameObject.h"
#include "../Math/Camera.h"
#include "../Rendering/Renderer.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace Engine {

/**
 * Scene - Manages All GameObjects in the Scene
 * 
 * Provides functionality for:
 * - Adding/removing GameObjects
 * - Scene-wide updates and rendering
 * - Object lifecycle management
 * - Scene state tracking
 */
class Scene {
private:
    // Scene objects
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unordered_map<std::string, GameObject*> objectMap; // For quick lookup by name
    
    // Scene state
    std::string sceneName;
    bool isInitialized;
    bool isActive;
    
    // Scene statistics
    int totalObjects;
    int activeObjects;
    int renderedObjects;
    
public:
    // Constructor/Destructor
    Scene(const std::string& name = "DefaultScene");
    ~Scene();
    
    // Scene lifecycle
    bool initialize();
    void update(float deltaTime);
    void render(const Camera& camera, const Renderer& renderer);
    void cleanup();
    
    // Object management
    void addGameObject(std::unique_ptr<GameObject> object);
    GameObject* getGameObject(const std::string& name);
    void removeGameObject(const std::string& name);
    void removeGameObject(GameObject* object);
    const std::vector<std::unique_ptr<GameObject>>& getGameObjects() const { return gameObjects; }
    
    // Scene control
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }
    
    void setName(const std::string& name) { sceneName = name; }
    const std::string& getName() const { return sceneName; }
    
    // Scene information
    int getTotalObjects() const { return totalObjects; }
    int getActiveObjects() const { return activeObjects; }
    int getRenderedObjects() const { return renderedObjects; }
    
    // Utility
    bool isValid() const { return isInitialized && isActive; }
    void clear();
    
    // Debug
    void printSceneInfo() const;
    
private:
    // Helper methods
    void updateObjectCounts();
    void cleanupDestroyedObjects();
};

} // namespace Engine
