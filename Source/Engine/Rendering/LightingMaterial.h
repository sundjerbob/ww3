/**
 * LightingMaterial.h - Material Properties for Lighting System
 * 
 * OVERVIEW:
 * Defines material properties for lighting calculations.
 * Works with the lighting system to create realistic surface appearances.
 * 
 * FEATURES:
 * - Ambient, diffuse, and specular properties
 * - Shininess for specular reflection control
 * - Material presets for common materials
 * - Integration with lighting system
 */

#pragma once
#include "../Math/Math.h"
#include <string>

namespace Engine {

/**
 * LightingMaterial Class - Surface Properties for Lighting
 * 
 * Defines how a surface interacts with light:
 * - Ambient: How much ambient light the surface reflects
 * - Diffuse: How much directional light the surface reflects
 * - Specular: How much specular reflection the surface has
 * - Shininess: How focused the specular reflection is
 */
class LightingMaterial {
private:
    std::string name;
    Vec3 ambient;    // Ambient reflection coefficient
    Vec3 diffuse;    // Diffuse reflection coefficient
    Vec3 specular;   // Specular reflection coefficient
    float shininess; // Specular exponent (higher = more focused)

public:
    // Constructors
    LightingMaterial();
    LightingMaterial(const std::string& materialName);
    LightingMaterial(const std::string& materialName, const Vec3& amb, const Vec3& diff, const Vec3& spec, float shine);
    
    // Getters
    const std::string& getName() const { return name; }
    const Vec3& getAmbient() const { return ambient; }
    const Vec3& getDiffuse() const { return diffuse; }
    const Vec3& getSpecular() const { return specular; }
    float getShininess() const { return shininess; }
    
    // Setters
    void setAmbient(const Vec3& amb) { ambient = amb; }
    void setDiffuse(const Vec3& diff) { diffuse = diff; }
    void setSpecular(const Vec3& spec) { specular = spec; }
    void setShininess(float shine) { shininess = shine; }
    
    // Material presets
    static LightingMaterial createDefault();
    static LightingMaterial createMetal();
    static LightingMaterial createPlastic();
    static LightingMaterial createWood();
    static LightingMaterial createStone();
    static LightingMaterial createGlass();
    static LightingMaterial createRubber();
    static LightingMaterial createFabric();
    static LightingMaterial createLeather();
    static LightingMaterial createCeramic();
};

} // namespace Engine
