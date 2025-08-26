/**
 * Arrow.h - 2D Direction Arrow GameObject for Minimap
 * 
 * OVERVIEW:
 * A 2D arrow GameObject specifically designed for minimap use.
 * Renders as a flat arrow that rotates around its center for top-down view.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Math/Math.h"
#include <GL/glew.h>
#include <vector>

namespace Engine {

/**
 * Arrow - 2D Direction Indicator for Minimap
 * 
 * Features:
 * - 2D arrow mesh designed for top-down orthographic view
 * - Rotates around Z-axis (in/out of screen) for proper minimap orientation
 * - Flat design that appears correctly when viewed from above
 * - Configurable color and size
 */
class Arrow : public GameObject {
private:
    // Arrow properties
    Vec3 color;
    float arrowSize;
    float arrowLength;  // Length of the arrow
    float arrowWidth;   // Width of the arrow head
    
public:
    // Constructor/Destructor
    Arrow(const std::string& name = "Arrow", float size = 1.0f, const Vec3& arrowColor = Vec3(1.0f, 0.0f, 0.0f));
    virtual ~Arrow() = default;
    
    // Arrow-specific methods
    void setColor(const Vec3& arrowColor) { color = arrowColor; }
    Vec3 getColor() const { return color; }
    
    void setArrowSize(float size) { arrowSize = size; }
    float getArrowSize() const { return arrowSize; }
    
    // Direction methods - for minimap, we rotate around Z-axis (in/out of screen)
    void setDirection(const Vec3& direction);
    void setDirectionFromYaw(float yawDegrees);
    
protected:
    // Override GameObject setup methods
    virtual void setupMesh() override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
private:
    // Helper methods
    void create2DArrowMesh();
};

} // namespace Engine
