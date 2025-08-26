#pragma once

#include <GL/glew.h>
#include <memory>
#include <vector>
#include "../Math/Math.h"
#include "Shader.h"

namespace Engine {

/**
 * ShadowMap - Handles shadow mapping for realistic shadows
 * 
 * Features:
 * - Depth map generation from light perspective
 * - Shadow mapping with PCF filtering
 * - Support for multiple light sources
 * - Efficient shadow rendering
 */
class ShadowMap {
private:
    // Shadow map properties
    unsigned int shadowMapFBO;
    unsigned int shadowMapTexture;
    int shadowMapWidth;
    int shadowMapHeight;
    
    // Light view matrices for each light
    std::vector<Mat4> lightSpaceMatrices;
    
    // Shadow mapping shaders
    std::unique_ptr<Shader> depthMapShader;
    std::unique_ptr<Shader> shadowShader;
    
    // Shadow map settings
    float shadowBias;
    float shadowBiasMin;
    float shadowBiasMax;
    int shadowMapResolution;
    
    // State
    bool isInitialized;
    bool isDepthMapGenerated;

public:
    ShadowMap(int resolution = 1024);
    ~ShadowMap();
    
    // Lifecycle
    bool initialize();
    void cleanup();
    
    // Shadow map generation
    void beginDepthMapGeneration();
    void endDepthMapGeneration();
    void generateDepthMap(const std::vector<Mat4>& lightSpaceMatrices);
    
    // Shadow rendering
    void setupShadowRendering(Shader& shader);
    void bindShadowMap(unsigned int textureUnit = 1);
    
    // Configuration
    void setShadowBias(float bias, float minBias = 0.005f, float maxBias = 0.05f);
    void setShadowMapResolution(int resolution);
    
    // Getters
    unsigned int getShadowMapTexture() const { return shadowMapTexture; }
    int getShadowMapWidth() const { return shadowMapWidth; }
    int getShadowMapHeight() const { return shadowMapHeight; }
    bool isValid() const { return isInitialized && isDepthMapGenerated; }
    Shader* getDepthMapShader() const { return depthMapShader.get(); }
    
    // Utility
    Mat4 calculateLightSpaceMatrix(const Vec3& lightPos, const Vec3& lightDir, 
                                  float nearPlane = 0.1f, float farPlane = 100.0f);
};

} // namespace Engine
