/**
 * LightingRenderer.h - Advanced Rendering with Lighting Support
 * 
 * OVERVIEW:
 * Extends BasicRenderer with comprehensive lighting capabilities.
 * Integrates light management, material properties, and advanced shaders.
 * 
 * FEATURES:
 * - Multiple light type support (ambient, directional, point)
 * - Material-based rendering
 * - Normal-based lighting calculations
 * - Integration with existing rendering pipeline
 */

#pragma once
#include "BasicRenderer.h"
#include "LightManager.h"
#include "LightingMaterial.h"
#include "ShadowMap.h"
#include <memory>

namespace Engine {

// Forward declarations
class GameObject;
class Camera;

/**
 * LightingRenderer Class - Advanced Rendering with Lighting
 * 
 * Extends BasicRenderer with comprehensive lighting support:
 * - Manages multiple light sources
 * - Applies material properties
 * - Calculates realistic lighting
 * - Integrates with existing game objects
 */
class LightingRenderer : public BasicRenderer {
private:
    std::unique_ptr<LightManager> lightManager;
    std::unique_ptr<Shader> lightingShader;
    std::unique_ptr<ShadowMap> shadowMap;
    LightingMaterial defaultMaterial;
    
    // Normal matrix calculation
    Mat3 calculateNormalMatrix(const Mat4& modelMatrix) const;
    
    // Shadow mapping
    void generateShadowMaps(const std::vector<GameObject*>& sceneObjects, const Camera& camera);
    std::vector<Mat4> calculateLightSpaceMatrices() const;

public:
    // Constructor/Destructor
    LightingRenderer();
    ~LightingRenderer();
    
    // Initialization
    bool initialize(int width, int height) override;
    void cleanup() override;
    
    // Light management
    LightManager* getLightManager() { return lightManager.get(); }
    void setupDefaultLighting();
    void setupDayLighting();
    void setupNightLighting();
    void setupIndoorLighting();
    
    // Material management
    void setDefaultMaterial(const LightingMaterial& material) { defaultMaterial = material; }
    const LightingMaterial& getDefaultMaterial() const { return defaultMaterial; }
    
    // Enhanced rendering methods
    void renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color) const override;
    void renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color, bool useHeightColoring) const;
    void renderMeshWithMaterial(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const LightingMaterial& material) const;
    
    // Shadow mapping
    void renderSceneWithShadows(const std::vector<GameObject*>& sceneObjects, const Camera& camera);
    void setupShadowRendering(const Shader& shader) const;
    
    // Light uniform management
    void updateLightUniforms(const Shader& shader) const;
    void updateMaterialUniforms(const Shader& shader, const LightingMaterial& material) const;
};

} // namespace Engine
