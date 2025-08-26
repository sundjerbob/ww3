/**
 * Material.h - Material Properties for 3D Rendering
 * 
 * OVERVIEW:
 * Stores material properties loaded from .mtl files for realistic rendering.
 * Supports ambient, diffuse, specular colors and material properties.
 * 
 * FEATURES:
 * - Material property storage (Kd, Ka, Ks, Ns, etc.)
 * - MTL file parsing support
 * - Multiple materials per object support
 */

#pragma once
#include "../Math/Math.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Engine {

/**
 * Material Class - Stores Material Properties
 * 
 * Represents a single material with its properties:
 * - Ambient color (Ka)
 * - Diffuse color (Kd) - main color
 * - Specular color (Ks) - reflection color
 * - Shininess (Ns) - specular highlight size
 * - Alpha (d) - transparency
 */
class Material {
public:
    std::string name;           // Material name from .mtl file
    
    // Color properties
    Vec3 ambient;               // Ka - ambient color
    Vec3 diffuse;               // Kd - main diffuse color
    Vec3 specular;              // Ks - specular reflection color
    Vec3 emissive;              // Ke - emissive color
    
    // Material properties
    float shininess;            // Ns - specular exponent (0-1000)
    float alpha;                // d - alpha/transparency (0-1)
    float refractionIndex;      // Ni - index of refraction
    int illuminationModel;      // illum - illumination model
    
    // Texture maps (for future extension)
    std::string diffuseTexture; // map_Kd
    std::string normalTexture;  // map_Bump
    std::string specularTexture;// map_Ks
    
    Material() 
        : name("default"),
          ambient(0.2f, 0.2f, 0.2f),
          diffuse(0.8f, 0.8f, 0.8f),
          specular(1.0f, 1.0f, 1.0f),
          emissive(0.0f, 0.0f, 0.0f),
          shininess(32.0f),
          alpha(1.0f),
          refractionIndex(1.0f),
          illuminationModel(2) {}
    
    Material(const std::string& materialName)
        : Material() {
        name = materialName;
    }
    
    // Get the main color for rendering (usually diffuse)
    Vec3 getMainColor() const { return diffuse; }
    
    // Check if material is valid
    bool isValid() const { return !name.empty(); }
};

/**
 * MaterialLibrary Class - Manages Multiple Materials
 * 
 * Stores and manages multiple materials loaded from .mtl files.
 * Provides access to materials by name and supports material lookup.
 */
class MaterialLibrary {
private:
    std::unordered_map<std::string, Material> materials;
    
public:
    MaterialLibrary() = default;
    
    // Add a material to the library
    void addMaterial(const Material& material) {
        materials[material.name] = material;
    }
    
    // Get material by name
    const Material* getMaterial(const std::string& name) const {
        auto it = materials.find(name);
        return (it != materials.end()) ? &it->second : nullptr;
    }
    
    // Get all materials
    const std::unordered_map<std::string, Material>& getAllMaterials() const {
        return materials;
    }
    
    // Check if material exists
    bool hasMaterial(const std::string& name) const {
        return materials.find(name) != materials.end();
    }
    
    // Get number of materials
    size_t getMaterialCount() const {
        return materials.size();
    }
    
    // Clear all materials
    void clear() {
        materials.clear();
    }
    
    // Get material names for debugging
    std::vector<std::string> getMaterialNames() const {
        std::vector<std::string> names;
        for (const auto& pair : materials) {
            names.push_back(pair.first);
        }
        return names;
    }
};

} // namespace Engine
