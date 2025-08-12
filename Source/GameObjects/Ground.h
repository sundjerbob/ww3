/**
 * Ground.h - Ground Plane GameObject
 * 
 * OVERVIEW:
 * A flat ground plane that serves as the terrain in the scene.
 * Inherits from GameObject and provides ground-specific functionality.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include <GL/glew.h>

namespace Engine {

/**
 * Ground - Ground Plane GameObject
 * 
 * Features:
 * - Large flat plane for terrain
 * - Configurable size and color
 * - Basic lighting support
 * - Positioned below other objects
 */
class Ground : public GameObject {
private:
    // Ground properties
    Vec3 color;
    float size;
    
public:
    // Constructor/Destructor
    Ground(const std::string& name = "Ground", float groundSize = 50.0f, const Vec3& groundColor = Vec3(0.4f, 0.3f, 0.2f));
    virtual ~Ground() = default;
    
    // Ground-specific methods
    void setColor(const Vec3& groundColor) { color = groundColor; }
    Vec3 getColor() const { return color; }
    
    void setSize(float groundSize) { size = groundSize; }
    float getSize() const { return size; }
    
protected:
    // Override GameObject setup methods
    virtual void setupMesh() override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
};

} // namespace Engine
