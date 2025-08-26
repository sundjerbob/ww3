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
#include "../Rendering/RendererFactory.h"
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
    
    // Parent-child system
    GameObject* parent;
    std::vector<std::unique_ptr<GameObject>> children;
    
    // Rendering
    std::unique_ptr<Mesh> mesh;
    Renderer* objectRenderer; // non-owning; set by game/scene
    Vec3 color; // Object color for rendering
    
    // Object state
    std::string name;
    bool isActive;
    bool isInitialized;
    bool isEntity;  // Flag to identify entity objects (cubes, NPCs, etc.) vs system objects (ground, UI, etc.)
    
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
    
    // World transform (includes parent transforms)
    Vec3 getWorldPosition() const;
    Vec3 getWorldRotation() const;
    Vec3 getWorldScale() const;
    Mat4 getWorldModelMatrix() const;
    
    // Object state
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }
    
    void setName(const std::string& objectName) { name = objectName; }
    const std::string& getName() const { return name; }
    
    // Entity system
    void setEntity(bool entity) { isEntity = entity; }
    bool getEntity() const { return isEntity; }
    
    // Color system
    void setColor(const Vec3& objectColor) { color = objectColor; }
    Vec3 getColor() const { return color; }
    
    // Parent-child system
    void addChild(std::unique_ptr<GameObject> child);
    void removeChild(GameObject* child);
    void setParent(GameObject* newParent);
    GameObject* getParent() const { return parent; }
    const std::vector<std::unique_ptr<GameObject>>& getChildren() const { return children; }
    
    // Utility
    bool isValid() const { return isInitialized && isActive; }
    void setRenderer(Renderer* renderer) { objectRenderer = renderer; }
    Renderer* getRenderer() const { return objectRenderer; }
    const Mesh* getMesh() const { return mesh.get(); }
    
    // Renderer selection - derived classes can override to choose their renderer
    virtual RendererType getPreferredRendererType() const { return RendererType::Basic; }
    
protected:
    // Helper methods for derived classes
    virtual void updateTransform();
    virtual void setupMesh();
    
    // Getters for derived classes
    // (getMesh() is now public)
};

} // namespace Engine
