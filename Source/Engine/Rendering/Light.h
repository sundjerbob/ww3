/**
 * Light.h - Lighting System for 3D Rendering
 * 
 * OVERVIEW:
 * Provides comprehensive lighting support for the game engine.
 * Supports multiple light types: directional, point, and ambient lighting.
 * 
 * FEATURES:
 * - Directional light (sun/moon)
 * - Point light (light bulbs, torches)
 * - Ambient light (global illumination)
 * - Configurable light properties
 * - Efficient light management
 */

#pragma once
#include "../Math/Math.h"
#include <string>

namespace Engine {

/**
 * Light Types
 */
enum class LightType {
    Directional,  // Sun, moon - infinite distance
    Point,        // Light bulbs, torches - finite distance
    Ambient       // Global illumination
};

/**
 * Light Class - Individual Light Source
 * 
 * Represents a single light source in the 3D world:
 * - Directional lights for sun/moon
 * - Point lights for local illumination
 * - Ambient light for global brightness
 */
class Light {
private:
    LightType type;
    std::string name;
    
    // Light properties
    Vec3 position;      // For point lights
    Vec3 direction;     // For directional lights
    Vec3 color;         // Light color (RGB)
    float intensity;    // Light brightness
    
    // Attenuation (for point lights)
    float constant;
    float linear;
    float quadratic;
    
    // Range (for point lights)
    float range;
    
    // Status
    bool isEnabled;

public:
    // Constructors
    Light();
    Light(const std::string& lightName, LightType lightType);
    
    // Virtual destructor for polymorphic behavior
    virtual ~Light() = default;
    
    // Getters
    LightType getType() const { return type; }
    const std::string& getName() const { return name; }
    const Vec3& getPosition() const { return position; }
    const Vec3& getDirection() const { return direction; }
    const Vec3& getColor() const { return color; }
    float getIntensity() const { return intensity; }
    float getConstant() const { return constant; }
    float getLinear() const { return linear; }
    float getQuadratic() const { return quadratic; }
    float getRange() const { return range; }
    bool isLightEnabled() const { return isEnabled; }
    
    // Setters
    void setPosition(const Vec3& pos) { position = pos; }
    void setDirection(const Vec3& dir) { direction = Engine::normalize(dir); }
    void setColor(const Vec3& col) { color = col; }
    void setIntensity(float inten) { intensity = inten; }
    void setAttenuation(float c, float l, float q) { constant = c; linear = l; quadratic = q; }
    void setRange(float r) { range = r; }
    void setEnabled(bool enabled) { isEnabled = enabled; }
    
    // Utility methods
    void calculateAttenuationFromRange(float lightRange);
    float calculateAttenuation(float distance) const;
};

/**
 * Directional Light - Sun/Moon
 * 
 * Creates a directional light (infinite distance)
 */
class DirectionalLight : public Light {
public:
    DirectionalLight(const std::string& name = "Sun");
    DirectionalLight(const std::string& name, const Vec3& direction, const Vec3& color, float intensity);
};

/**
 * Point Light - Light Bulbs, Torches
 * 
 * Creates a point light with attenuation
 */
class PointLight : public Light {
public:
    PointLight(const std::string& name = "PointLight");
    PointLight(const std::string& name, const Vec3& position, const Vec3& color, float intensity, float range);
};

/**
 * Ambient Light - Global Illumination
 * 
 * Creates ambient light for global brightness
 */
class AmbientLight : public Light {
public:
    AmbientLight(const std::string& name = "Ambient");
    AmbientLight(const std::string& name, const Vec3& color, float intensity);
};

} // namespace Engine
