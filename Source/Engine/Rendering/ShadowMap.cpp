#include "ShadowMap.h"
#include <iostream>

namespace Engine {

ShadowMap::ShadowMap(int resolution)
    : shadowMapFBO(0), shadowMapTexture(0), shadowMapWidth(resolution), shadowMapHeight(resolution),
      shadowBias(0.005f), shadowBiasMin(0.005f), shadowBiasMax(0.05f), shadowMapResolution(resolution),
      isInitialized(false), isDepthMapGenerated(false) {
}

ShadowMap::~ShadowMap() {
    cleanup();
}

bool ShadowMap::initialize() {
    if (isInitialized) {
        return true;
    }
    
    
    // Create framebuffer for shadow mapping
    glGenFramebuffers(1, &shadowMapFBO);
    
    // Create depth texture
    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters for shadow mapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to prevent shadow artifacts
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
    
    // Tell OpenGL we don't need color attachments for shadow mapping
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Load shadow mapping shaders
    depthMapShader = std::make_unique<Shader>();
    if (!depthMapShader->loadFromFiles("Resources/Shaders/depth_map_vertex.glsl", "Resources/Shaders/depth_map_fragment.glsl")) {
        return false;
    }
    
    shadowShader = std::make_unique<Shader>();
    if (!shadowShader->loadFromFiles("Resources/Shaders/shadow_vertex.glsl", "Resources/Shaders/shadow_fragment.glsl")) {
        return false;
    }
    
    isInitialized = true;
    return true;
}

void ShadowMap::cleanup() {
    if (shadowMapFBO) {
        glDeleteFramebuffers(1, &shadowMapFBO);
        shadowMapFBO = 0;
    }
    
    if (shadowMapTexture) {
        glDeleteTextures(1, &shadowMapTexture);
        shadowMapTexture = 0;
    }
    
    depthMapShader.reset();
    shadowShader.reset();
    
    isInitialized = false;
    isDepthMapGenerated = false;
}

void ShadowMap::beginDepthMapGeneration() {
    if (!isInitialized) return;
    
    // Bind shadow map framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    
    // Clear depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Use depth map shader
    if (depthMapShader) {
        depthMapShader->use();
    }
}

void ShadowMap::endDepthMapGeneration() {
    if (!isInitialized) return;
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Reset viewport (this should be set by the calling renderer)
    // glViewport(0, 0, windowWidth, windowHeight);
    
    isDepthMapGenerated = true;
}

void ShadowMap::generateDepthMap(const std::vector<Mat4>& lightSpaceMatrices) {
    if (!isInitialized || lightSpaceMatrices.empty()) return;
    
    beginDepthMapGeneration();
    
    // Store light space matrices for later use
    this->lightSpaceMatrices = lightSpaceMatrices;
    
    // The actual rendering of objects should be done by the calling renderer
    // This method just sets up the depth map generation
    
    endDepthMapGeneration();
}

void ShadowMap::setupShadowRendering(Shader& shader) {
    if (!isValid()) return;
    
    // Set shadow map texture
    shader.setInt("shadowMap", 1); // Texture unit 1
    
    // Set shadow bias parameters
    shader.setFloat("shadowBias", shadowBias);
    shader.setFloat("shadowBiasMin", shadowBiasMin);
    shader.setFloat("shadowBiasMax", shadowBiasMax);
    
    // Set light space matrices
    for (size_t i = 0; i < lightSpaceMatrices.size() && i < 4; ++i) {
        std::string uniformName = "lightSpaceMatrix[" + std::to_string(i) + "]";
        shader.setMat4(uniformName, lightSpaceMatrices[i]);
    }
    
    // Set number of light space matrices
    shader.setInt("numLightSpaceMatrices", static_cast<int>(lightSpaceMatrices.size()));
}

void ShadowMap::bindShadowMap(unsigned int textureUnit) {
    if (!isValid()) return;
    
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
}

void ShadowMap::setShadowBias(float bias, float minBias, float maxBias) {
    shadowBias = bias;
    shadowBiasMin = minBias;
    shadowBiasMax = maxBias;
}

void ShadowMap::setShadowMapResolution(int resolution) {
    if (resolution != shadowMapResolution) {
        shadowMapResolution = resolution;
        shadowMapWidth = resolution;
        shadowMapHeight = resolution;
        
        // Recreate shadow map texture with new resolution
        if (isInitialized) {
            glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }
    }
}

Mat4 ShadowMap::calculateLightSpaceMatrix(const Vec3& lightPos, const Vec3& lightDir, float nearPlane, float farPlane) {
    // Calculate light space matrix for directional light
    Vec3 lightPosition = lightPos;
    Vec3 lightDirection = lightDir;
    
    // Create orthographic projection for directional light
    float orthoSize = 20.0f; // Size of the light's view frustum
    Mat4 lightProjection = Engine::orthographic(-orthoSize, orthoSize, -orthoSize, orthoSize, nearPlane, farPlane);
    
    // Create light view matrix
    Vec3 target = lightPosition + lightDirection;
    Vec3 up(0.0f, 1.0f, 0.0f);
    Mat4 lightView = Engine::lookAt(lightPosition, target, up);
    
    // Combine projection and view matrices
    return lightProjection * lightView;
}

} // namespace Engine
