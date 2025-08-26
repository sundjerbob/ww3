/**
 * LightManager.h - Light Management System
 * 
 * OVERVIEW:
 * Manages multiple light sources and provides them to shaders.
 * Handles light collection, organization, and uniform distribution.
 * 
 * FEATURES:
 * - Multiple light management
 * - Light type organization
 * - Shader uniform distribution
 * - Efficient light updates
 */

#pragma once
#include "Light.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace Engine {

/**
 * LightManager Class - Light Collection and Management
 * 
 * Manages all light sources in the scene:
 * - Collects and organizes lights by type
 * - Provides lights to shaders via uniforms
 * - Handles light updates and state changes
 * - Efficient light management for rendering
 */
class LightManager {
private:
    // Light collections by type
    std::vector<std::shared_ptr<DirectionalLight>> directionalLights;
    std::vector<std::shared_ptr<PointLight>> pointLights;
    std::vector<std::shared_ptr<AmbientLight>> ambientLights;
    
    // Light lookup by name
    std::unordered_map<std::string, std::shared_ptr<Light>> lightMap;
    
    // Light limits for shader compatibility
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 16;
    static const int MAX_AMBIENT_LIGHTS = 4;

public:
    // Constructor/Destructor
    LightManager();
    ~LightManager();
    
    // Light management
    void addLight(std::shared_ptr<Light> light);
    void removeLight(const std::string& lightName);
    void clearAllLights();
    
    // Light access
    std::shared_ptr<Light> getLight(const std::string& lightName);
    std::shared_ptr<DirectionalLight> getDirectionalLight(const std::string& name);
    std::shared_ptr<PointLight> getPointLight(const std::string& name);
    std::shared_ptr<AmbientLight> getAmbientLight(const std::string& name);
    
    // Light collections
    const std::vector<std::shared_ptr<DirectionalLight>>& getDirectionalLights() const { return directionalLights; }
    const std::vector<std::shared_ptr<PointLight>>& getPointLights() const { return pointLights; }
    const std::vector<std::shared_ptr<AmbientLight>>& getAmbientLights() const { return ambientLights; }
    
    // Light counts
    int getDirectionalLightCount() const { return static_cast<int>(directionalLights.size()); }
    int getPointLightCount() const { return static_cast<int>(pointLights.size()); }
    int getAmbientLightCount() const { return static_cast<int>(ambientLights.size()); }
    int getTotalLightCount() const { return static_cast<int>(lightMap.size()); }
    
    // Utility
    bool hasLight(const std::string& lightName) const;
    void updateLight(const std::string& lightName, const Vec3& position, const Vec3& direction, const Vec3& color, float intensity);
    
    // Default light setup
    void setupDefaultLighting();
    void setupDayLighting();
    void setupNightLighting();
    void setupIndoorLighting();
};

} // namespace Engine
