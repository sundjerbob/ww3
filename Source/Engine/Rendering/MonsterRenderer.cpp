/**
 * MonsterRenderer.cpp - Implementation of Specialized Monster Renderer
 */

#include "MonsterRenderer.h"
#include "Mesh.h"
#include "../Math/Camera.h"
#include <GL/glew.h>
#include <iostream>

namespace Engine {

MonsterRenderer::MonsterRenderer()
    : windowWidth(800), windowHeight(600), isInitialized(false), 
      useTextureRendering(true), textureStrength(0.8f) {
}

MonsterRenderer::~MonsterRenderer() {
    cleanup();
}

bool MonsterRenderer::initialize(int width, int height) {
    std::cout << "Initializing MonsterRenderer..." << std::endl;
    
    windowWidth = width;
    windowHeight = height;
    
    if (!initializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL for MonsterRenderer" << std::endl;
        return false;
    }
    
    if (!loadMonsterShader()) {
        std::cerr << "Failed to load monster shader" << std::endl;
        return false;
    }
    
    // if (!loadHealthBarShader()) {  // REMOVED: Using new texture-based system
    //     std::cerr << "Failed to load health bar shader" << std::endl;
    //     return false;
    // }
    
    // Initialize texture
    monsterTexture = std::make_unique<Texture>();
    
    updateProjectionMatrix();
    
    // Setup health bar quad - REMOVED: Using new texture-based system
    // setupHealthBarQuad();
    
    isInitialized = true;
    std::cout << "MonsterRenderer initialized successfully" << std::endl;
    return true;
}

void MonsterRenderer::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up MonsterRenderer..." << std::endl;
    
    // Cleanup health bar quad - REMOVED: Using new texture-based system
    // cleanupHealthBarQuad();
    
    monsterShader.reset();
    // healthBarShader.reset();  // REMOVED
    monsterTexture.reset();
    
    isInitialized = false;
}

void MonsterRenderer::beginFrame() {
    // Monster rendering doesn't need frame clearing since it's part of the main 3D scene
    // The main renderer handles frame clearing
}

void MonsterRenderer::endFrame(GLFWwindow* window) {
    // Monster rendering doesn't need frame ending since it's part of the main 3D scene
    // The main renderer handles frame ending
}

void MonsterRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    updateProjectionMatrix();
}

void MonsterRenderer::setClearColor(float r, float g, float b, float a) {
    // Monster renderer doesn't handle clear color
    // This is handled by the main renderer
}

void MonsterRenderer::renderMesh(const Mesh& mesh,
                                const Mat4& modelMatrix,
                                const Camera& camera,
                                const Vec3& color) const {
    // Default implementation calls monster-specific rendering
    renderMonsterMesh(mesh, modelMatrix, camera, color, useTextureRendering);
}

void MonsterRenderer::renderCrosshair(const Camera& camera) const {
    // Monster renderer doesn't handle crosshair rendering
    // This is handled by the CrosshairRenderer
}

void MonsterRenderer::renderMonsterMesh(const Mesh& mesh,
                                       const Mat4& modelMatrix,
                                       const Camera& camera,
                                       const Vec3& color,
                                       bool useTexture) const {

    
    if (!isInitialized || !monsterShader) {
        std::cerr << "MonsterRenderer not properly initialized" << std::endl;
        return;
    }
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Enable depth testing for proper 3D world-space rendering
    glEnable(GL_DEPTH_TEST);
    
    // Enable blending for monster rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use monster shader
    monsterShader->use();
    
    // For 3D world-space monster rendering, use proper view and projection matrices
    monsterShader->setMat4("model", modelMatrix);
    monsterShader->setMat4("view", camera.getViewMatrix());
    monsterShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Set monster color
    Vec3 monsterColor = color;
    if (!useTexture || !monsterTexture || !monsterTexture->isValid()) {
        // Use the provided color (which could be a material color)
        monsterColor = color;
    }
    
    // CRITICAL FIX: Disable height coloring for monsters and use the object color
    monsterShader->setInt("useHeightColoring", 0);
    monsterShader->setVec3("color", monsterColor);
    
    // Handle texture rendering
    if (useTexture && monsterTexture && monsterTexture->isValid()) {
        monsterShader->setInt("useTexture", 1);
        monsterShader->setFloat("textureStrength", textureStrength);
        
        // Bind texture
        monsterTexture->bind(0);
        monsterShader->setInt("monsterTexture", 0);
        

    } else {
        monsterShader->setInt("useTexture", 0);
        

    }
    
    // Render the mesh
    mesh.render();
    
    // Restore OpenGL state
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    
    glDisable(GL_BLEND);
}

void MonsterRenderer::renderMonsterTriangles(const Mesh& mesh,
                                            const Mat4& modelMatrix,
                                            const Camera& camera,
                                            const Vec3& color,
                                            const std::vector<unsigned int>& triangleIndices,
                                            bool useTexture) const {
    if (!isInitialized || !monsterShader) {
        std::cerr << "MonsterRenderer not properly initialized" << std::endl;
        return;
    }
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Enable depth testing for proper 3D world-space rendering
    glEnable(GL_DEPTH_TEST);
    
    // Enable blending for monster rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use monster shader
    monsterShader->use();
    
    // Set uniforms for 3D world-space rendering
    monsterShader->setMat4("model", modelMatrix);
    monsterShader->setMat4("view", camera.getViewMatrix());
    monsterShader->setMat4("projection", camera.getProjectionMatrix());
    
    // CRITICAL FIX: Disable height coloring for monsters and use the object color
    monsterShader->setInt("useHeightColoring", 0);
    monsterShader->setVec3("color", color);
    
    // Handle texture rendering
    if (useTexture && monsterTexture && monsterTexture->isValid()) {
        monsterShader->setInt("useTexture", 1);
        monsterShader->setFloat("textureStrength", textureStrength);
        monsterTexture->bind(0);
        monsterShader->setInt("monsterTexture", 0);
    } else {
        monsterShader->setInt("useTexture", 0);
    }
    
    // Render only the specified triangles with the material color
    mesh.renderTriangles(triangleIndices);
    
    // Restore OpenGL state
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    glDisable(GL_BLEND);
}

float MonsterRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& MonsterRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}

bool MonsterRenderer::loadMonsterTexture(const std::string& texturePath) {
    if (!monsterTexture) {
        std::cerr << "Monster texture not initialized" << std::endl;
        return false;
    }
    
    return monsterTexture->loadFromFile(texturePath);
}

bool MonsterRenderer::initializeOpenGL() {
    // OpenGL is already initialized by the main renderer
    // Just verify we have a valid OpenGL context
    return true;
}

bool MonsterRenderer::loadMonsterShader() {
    monsterShader = std::make_unique<Shader>();
    
    // Use the same shaders as the basic renderer for now
    // TODO: Create monster-specific shaders if needed
    if (!monsterShader->loadFromFiles("Resources/Shaders/vertex.glsl", 
                                     "Resources/Shaders/fragment.glsl")) {
        std::cerr << "Failed to load monster shader files" << std::endl;
        return false;
    }
    
    std::cout << "Monster shader loaded successfully" << std::endl;
    return true;
}

// Health bar shader loading - REMOVED: Using new texture-based system
// bool MonsterRenderer::loadHealthBarShader() { ... }

void MonsterRenderer::updateProjectionMatrix() {
    // Use perspective projection for 3D world-space monster rendering
    float aspectRatio = getAspectRatio();
    projectionMatrix = Engine::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

// Health bar quad setup - REMOVED: Using new texture-based system
// void MonsterRenderer::setupHealthBarQuad() { ... }

// Health bar quad cleanup - REMOVED: Using new texture-based system
// void MonsterRenderer::cleanupHealthBarQuad() { ... }

// Health bar rendering - REMOVED: Using new texture-based system
// void MonsterRenderer::renderHealthBar(const Vec3& monsterPosition, float healthPercentage, const Camera& camera) const { ... }

} // namespace Engine
