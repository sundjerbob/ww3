/**
 * LightingRenderer.cpp - Advanced Rendering with Lighting Implementation
 */

#include "LightingRenderer.h"
#include "../Core/GameObject.h"
#include "../Math/Camera.h"
#include <iostream>

namespace Engine {

LightingRenderer::LightingRenderer()
    : lightManager(std::make_unique<LightManager>()), 
      lightingShader(std::make_unique<Shader>()),
      shadowMap(std::make_unique<ShadowMap>(1024)),
      defaultMaterial(LightingMaterial::createDefault()) {
}

LightingRenderer::~LightingRenderer() {
    cleanup();
}

bool LightingRenderer::initialize(int width, int height) {
    // Initialize base renderer
    if (!BasicRenderer::initialize(width, height)) {
        return false;
    }
    
    // Load lighting shader
    if (!lightingShader->loadFromFiles("Resources/Shaders/lighting_vertex.glsl", "Resources/Shaders/lighting_fragment.glsl")) {
        return false;
    }
    
    // Initialize shadow mapping
    if (!shadowMap->initialize()) {
        return false;
    }
    
    // Setup default lighting
    setupDefaultLighting();
    
    return true;
}

void LightingRenderer::cleanup() {
    lightingShader.reset();
    lightManager.reset();
    shadowMap.reset();
    BasicRenderer::cleanup();
}

void LightingRenderer::setupDefaultLighting() {
    if (lightManager) {
        lightManager->setupDefaultLighting();
    }
}

void LightingRenderer::setupDayLighting() {
    if (lightManager) {
        lightManager->setupDayLighting();
    }
}

void LightingRenderer::setupNightLighting() {
    if (lightManager) {
        lightManager->setupNightLighting();
    }
}

void LightingRenderer::setupIndoorLighting() {
    if (lightManager) {
        lightManager->setupIndoorLighting();
    }
}

Mat3 LightingRenderer::calculateNormalMatrix(const Mat4& modelMatrix) const {
    // Extract the 3x3 rotation/scale matrix from the 4x4 model matrix
    Mat3 normalMatrix;
    const float* modelData = modelMatrix.data();
    float* normalData = normalMatrix.data();
    
    // Copy the upper-left 3x3 portion
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            normalData[i * 3 + j] = modelData[i * 4 + j];
        }
    }
    
    // Calculate the inverse transpose for proper normal transformation
    return Engine::transpose(Engine::inverse(normalMatrix));
}

void LightingRenderer::renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color) const {
    // Use default material with the provided color
    LightingMaterial material = defaultMaterial;
    material.setDiffuse(color);
    renderMeshWithMaterial(mesh, modelMatrix, camera, material);
}

void LightingRenderer::renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color, bool useHeightColoring) const {
    if (useHeightColoring) {
        // Fall back to basic rendering for height-based coloring
        BasicRenderer::renderMesh(mesh, modelMatrix, camera, color, useHeightColoring);
    } else {
        // Use lighting rendering
        renderMesh(mesh, modelMatrix, camera, color);
    }
}

void LightingRenderer::renderMeshWithMaterial(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const LightingMaterial& material) const {
    if (!lightingShader || !lightingShader->isValidShader()) {
        BasicRenderer::renderMesh(mesh, modelMatrix, camera, material.getDiffuse());
        return;
    }
    
    // Use lighting shader
    lightingShader->use();
    
    // Set transformation matrices
    lightingShader->setMat4("model", modelMatrix);
    lightingShader->setMat4("view", camera.getViewMatrix());
    lightingShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Calculate and set normal matrix
    Mat3 normalMatrix = calculateNormalMatrix(modelMatrix);
    lightingShader->setMat3("normalMatrix", normalMatrix);
    
    // Set camera position for view direction calculation
    lightingShader->setVec3("viewPos", camera.getPosition());
    
    // Update light uniforms
    updateLightUniforms(*lightingShader);
    
    // Update material uniforms
    updateMaterialUniforms(*lightingShader, material);
    
    // Render the mesh
    mesh.render();
}

void LightingRenderer::updateLightUniforms(const Shader& shader) const {
    if (!lightManager) return;
    
    // Update directional lights
    const auto& directionalLights = lightManager->getDirectionalLights();
    int dirCount = std::min(lightManager->getDirectionalLightCount(), 4);
    shader.setInt("numDirectionalLights", dirCount);
    
    for (int i = 0; i < dirCount; i++) {
        const auto& light = directionalLights[i];
        std::string prefix = "directionalLights[" + std::to_string(i) + "].";
        shader.setVec3(prefix + "direction", light->getDirection());
        shader.setVec3(prefix + "color", light->getColor());
        shader.setFloat(prefix + "intensity", light->getIntensity());
        shader.setInt(prefix + "isEnabled", light->isLightEnabled() ? 1 : 0);
    }
    
    // Update point lights
    const auto& pointLights = lightManager->getPointLights();
    int pointCount = std::min(lightManager->getPointLightCount(), 16);
    shader.setInt("numPointLights", pointCount);
    
    for (int i = 0; i < pointCount; i++) {
        const auto& light = pointLights[i];
        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        shader.setVec3(prefix + "position", light->getPosition());
        shader.setVec3(prefix + "color", light->getColor());
        shader.setFloat(prefix + "intensity", light->getIntensity());
        shader.setFloat(prefix + "constant", light->getConstant());
        shader.setFloat(prefix + "linear", light->getLinear());
        shader.setFloat(prefix + "quadratic", light->getQuadratic());
        shader.setInt(prefix + "isEnabled", light->isLightEnabled() ? 1 : 0);
    }
    
    // Update ambient lights
    const auto& ambientLights = lightManager->getAmbientLights();
    int ambientCount = std::min(lightManager->getAmbientLightCount(), 4);
    shader.setInt("numAmbientLights", ambientCount);
    
    for (int i = 0; i < ambientCount; i++) {
        const auto& light = ambientLights[i];
        std::string prefix = "ambientLights[" + std::to_string(i) + "].";
        shader.setVec3(prefix + "color", light->getColor());
        shader.setFloat(prefix + "intensity", light->getIntensity());
        shader.setInt(prefix + "isEnabled", light->isLightEnabled() ? 1 : 0);
    }
}

void LightingRenderer::updateMaterialUniforms(const Shader& shader, const LightingMaterial& material) const {
    shader.setVec3("material.ambient", material.getAmbient());
    shader.setVec3("material.diffuse", material.getDiffuse());
    shader.setVec3("material.specular", material.getSpecular());
    shader.setFloat("material.shininess", material.getShininess());
}

void LightingRenderer::renderSceneWithShadows(const std::vector<GameObject*>& sceneObjects, const Camera& camera) {
    if (!shadowMap || !shadowMap->isValid()) {
        return;
    }
    
    // First pass: Generate shadow maps
    generateShadowMaps(sceneObjects, camera);
    
    // Second pass: Render scene with shadows
    if (!lightingShader || !lightingShader->isValidShader()) {
        return;
    }
    
    // Use lighting shader for rendering with shadows
    lightingShader->use();
    
    // Setup shadow rendering
    setupShadowRendering(*lightingShader);
    
    // Set light space matrices for shadow mapping
    std::vector<Mat4> lightSpaceMatrices = calculateLightSpaceMatrices();
    for (size_t i = 0; i < lightSpaceMatrices.size() && i < 4; ++i) {
        std::string uniformName = "lightSpaceMatrix[" + std::to_string(i) + "]";
        lightingShader->setMat4(uniformName, lightSpaceMatrices[i]);
    }
    lightingShader->setInt("numLightSpaceMatrices", static_cast<int>(lightSpaceMatrices.size()));
    
    // Render all scene objects with shadows
    for (const auto& obj : sceneObjects) {
        if (obj && obj->getMesh()) {
            Mat4 modelMatrix = obj->getModelMatrix();
            LightingMaterial material = defaultMaterial;
            
            // Set transformation matrices
            lightingShader->setMat4("model", modelMatrix);
            lightingShader->setMat4("view", camera.getViewMatrix());
            lightingShader->setMat4("projection", camera.getProjectionMatrix());
            
            // Calculate and set normal matrix
            Mat3 normalMatrix = calculateNormalMatrix(modelMatrix);
            lightingShader->setMat3("normalMatrix", normalMatrix);
            
            // Set camera position
            lightingShader->setVec3("viewPos", camera.getPosition());
            
            // Update light and material uniforms
            updateLightUniforms(*lightingShader);
            updateMaterialUniforms(*lightingShader, material);
            
            // Render the mesh
            obj->getMesh()->render();
        }
    }
}

void LightingRenderer::generateShadowMaps(const std::vector<GameObject*>& sceneObjects, const Camera& camera) {
    if (!shadowMap || !shadowMap->isValid()) return;
    
    // Calculate light space matrices for all directional lights
    std::vector<Mat4> lightSpaceMatrices = calculateLightSpaceMatrices();
    
    if (lightSpaceMatrices.empty()) return;
    
    // Begin depth map generation
    shadowMap->beginDepthMapGeneration();
    
    // Use the first light space matrix for depth map generation
    if (shadowMap->getDepthMapShader() && !lightSpaceMatrices.empty()) {
        shadowMap->getDepthMapShader()->setMat4("lightSpaceMatrix", lightSpaceMatrices[0]);
    }
    
    // Render all scene objects to depth map
    for (const auto& obj : sceneObjects) {
        if (obj && obj->getMesh()) {
            Mat4 modelMatrix = obj->getModelMatrix();
            
                                // Set model matrix for depth map shader
                    if (shadowMap->getDepthMapShader()) {
                        shadowMap->getDepthMapShader()->setMat4("model", modelMatrix);
                    }
            
            // Render mesh to depth map
            obj->getMesh()->render();
        }
    }
    
    // End depth map generation
    shadowMap->endDepthMapGeneration();
}

std::vector<Mat4> LightingRenderer::calculateLightSpaceMatrices() const {
    std::vector<Mat4> matrices;
    
    if (!lightManager) return matrices;
    
    // Get directional lights for shadow mapping
    const auto& directionalLights = lightManager->getDirectionalLights();
    
    for (const auto& light : directionalLights) {
        if (light && light->isLightEnabled()) {
            Vec3 lightPos = light->getPosition();
            Vec3 lightDir = light->getDirection();
            
            // Calculate light space matrix
            Mat4 lightSpaceMatrix = shadowMap->calculateLightSpaceMatrix(lightPos, lightDir);
            matrices.push_back(lightSpaceMatrix);
            
            // Limit to 4 light sources for shader compatibility
            if (matrices.size() >= 4) break;
        }
    }
    
    return matrices;
}

void LightingRenderer::setupShadowRendering(const Shader& shader) const {
    if (!shadowMap || !shadowMap->isValid()) return;
    
    // Setup shadow mapping uniforms
    shadowMap->setupShadowRendering(const_cast<Shader&>(shader));
    
    // Bind shadow map texture
    shadowMap->bindShadowMap(1); // Texture unit 1
}

} // namespace Engine
