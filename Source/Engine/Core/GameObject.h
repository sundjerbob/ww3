/**
 * GameObject.h - Base Class for All Game Objects
 * 
 * OVERVIEW:
 * Base class for all objects that can be rendered in the scene.
 * Provides common functionality for position, rotation, scale, and rendering.
 * 
 * FEATURES:
 * - Transform management (position, rotation, scale)
 * - Lifecycle management (initialize, update, render, cleanup)
 * - Component-based architecture foundation
 * - Delta time integration
 */

#pragma once
#include "../Math/Math.h"
#include <memory>
#include <string>
#include <vector>

namespace Engine {

class Mesh;
class Renderer;
class Shader; // forward declaration for header completeness where referenced in inline getters previously
class Camera;

/**
 * GameObject - Base Class for All Scene Objects
 * 
 * Provides common functionality for:
 * - Transform management
 * - Rendering lifecycle
 * - Component system foundation
 * - Scene integration
 */
class GameObject {
protected:
    // Transform
    Vec3 position;
    Vec3 rotation;      // Euler angles in degrees
    Vec3 scale;
    
    // Rendering
    std::unique_ptr<Mesh> mesh;
    Renderer* objectRenderer; // non-owning; set by game/scene
    
    // Object state
    std::string name;
    bool isActive;
    bool isInitialized;
    
    // Update timing
    float lastUpdateTime;
    
public:
    // Constructor/Destructor
    GameObject(const std::string& objectName = "GameObject");
    virtual ~GameObject();
    
    // Lifecycle methods
    virtual bool initialize();
    virtual void update(float deltaTime);
    virtual void render(const Renderer& renderer, const Camera& camera);
    virtual void cleanup();
    
    // Transform methods
    void setPosition(const Vec3& pos) { position = pos; }
    void setRotation(const Vec3& rot) { rotation = rot; }
    void setScale(const Vec3& scl) { scale = scl; }
    
    Vec3 getPosition() const { return position; }
    Vec3 getRotation() const { return rotation; }
    Vec3 getScale() const { return scale; }
    
    // Transform matrix
    Mat4 getModelMatrix() const;
    
    // Object state
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }
    
    void setName(const std::string& objectName) { name = objectName; }
    const std::string& getName() const { return name; }
    
    // Utility
    bool isValid() const { return isInitialized && isActive; }
    void setRenderer(Renderer* renderer) { objectRenderer = renderer; }
    const Mesh* getMesh() const { return mesh.get(); }
    
protected:
    // Helper methods for derived classes
    virtual void updateTransform();
    virtual void setupMesh();
    
    // Getters for derived classes
    // (getMesh() is now public)
};

} // namespace Engine
