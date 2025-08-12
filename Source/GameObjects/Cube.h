/**
 * Cube.h - 3D Cube GameObject
 * 
 * OVERVIEW:
 * A simple 3D cube that can be rendered in the scene.
 * Inherits from GameObject and provides cube-specific functionality.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include <GL/glew.h>

namespace Engine {

/**
 * Cube - 3D Cube GameObject
 * 
 * Features:
 * - Standard cube geometry
 * - Configurable color
 * - Optional rotation animation
 * - Basic lighting support
 */
class Cube : public GameObject {
private:
    // Cube properties
    Vec3 color;
    bool isRotating;
    float rotationSpeed;
    
public:
    // Constructor/Destructor
    Cube(const std::string& name = "Cube", const Vec3& cubeColor = Vec3(1.0f, 1.0f, 1.0f));
    virtual ~Cube() = default;
    
    // Cube-specific methods
    void setColor(const Vec3& cubeColor) { color = cubeColor; }
    Vec3 getColor() const { return color; }
    
    void setRotating(bool rotating) { isRotating = rotating; }
    bool getRotating() const { return isRotating; }
    
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationSpeed() const { return rotationSpeed; }
    
    // Override GameObject methods
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
 protected:
    // Override GameObject setup methods
    virtual void setupMesh() override;
};

} // namespace Engine
