/**
 * WeaponRenderer.cpp - Implementation of Specialized Weapon Renderer
 */

#include "WeaponRenderer.h"
#include "Mesh.h"
#include "../Math/Camera.h"
#include <GL/glew.h>
#include <iostream>

namespace Engine {

WeaponRenderer::WeaponRenderer()
    : windowWidth(800), windowHeight(600), isInitialized(false), 
      useTextureRendering(true), textureStrength(0.8f) {}

WeaponRenderer::~WeaponRenderer() {
    cleanup();
}

bool WeaponRenderer::initialize(int width, int height) {
    std::cout << "Initializing WeaponRenderer..." << std::endl;
    
    windowWidth = width;
    windowHeight = height;
    
    if (!initializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL for WeaponRenderer" << std::endl;
        return false;
    }
    
    if (!loadWeaponShader()) {
        std::cerr << "Failed to load weapon shader" << std::endl;
        return false;
    }
    
    // Initialize texture
    weaponTexture = std::make_unique<Texture>();
    
    updateProjectionMatrix();
    
    isInitialized = true;
    std::cout << "WeaponRenderer initialized successfully" << std::endl;
    return true;
}

void WeaponRenderer::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up WeaponRenderer..." << std::endl;
    
    weaponShader.reset();
    weaponTexture.reset();
    
    isInitialized = false;
}

void WeaponRenderer::beginFrame() {
    // Weapon rendering doesn't need frame clearing since it's an overlay
    // The main renderer handles frame clearing
}

void WeaponRenderer::endFrame(GLFWwindow* window) {
    // Weapon rendering doesn't need frame ending since it's an overlay
    // The main renderer handles frame ending
}

void WeaponRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    updateProjectionMatrix();
}

void WeaponRenderer::setClearColor(float r, float g, float b, float a) {
    // Weapon renderer doesn't handle clear color
    // This is handled by the main renderer
}

void WeaponRenderer::renderMesh(const Mesh& mesh,
                                const Mat4& modelMatrix,
                                const Camera& camera,
                                const Vec3& color) const {
    // Default implementation calls weapon-specific rendering
    renderWeaponMesh(mesh, modelMatrix, camera, color, useTextureRendering);
}

void WeaponRenderer::renderCrosshair(const Camera& camera) const {
    // Weapon renderer doesn't handle crosshair rendering
    // This is handled by the CrosshairRenderer
}

void WeaponRenderer::renderWeaponMesh(const Mesh& mesh,
                                     const Mat4& modelMatrix,
                                     const Camera& camera,
                                     const Vec3& color,
                                     bool useTexture) const {

    
    if (!isInitialized || !weaponShader) {
        std::cerr << "WeaponRenderer not properly initialized" << std::endl;
        return;
    }
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Disable depth testing for weapon overlay rendering
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for weapon rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use weapon shader
    weaponShader->use();
    
    // For FPS-style weapon rendering, we need to render in screen space
    // Use the model matrix as provided (should be in view space)
    weaponShader->setMat4("model", modelMatrix);
    
    // Use identity view matrix for screen space rendering
    Mat4 identityView = Mat4();
    weaponShader->setMat4("view", identityView);
    
    // For weapon overlay rendering, use orthographic projection for 2D screen space
    weaponShader->setMat4("projection", Engine::perspective(45.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 100.0f));
    
    // Set weapon color - use a realistic weapon color if texture is disabled
    Vec3 weaponColor = color;
    if (!useTexture || !weaponTexture || !weaponTexture->isValid()) {
        // For now, use the provided color (which could be a material color)
        // Later we can extend this to render with multiple materials
        weaponColor = color;
    }
    weaponShader->setVec3("color", weaponColor);
    
    // Handle texture rendering
    if (useTexture && weaponTexture && weaponTexture->isValid()) {
        weaponShader->setInt("useTexture", 1);
        weaponShader->setFloat("textureStrength", textureStrength);
        
        // Bind texture
        weaponTexture->bind(0);
        weaponShader->setInt("weaponTexture", 0);
        

    } else {
        weaponShader->setInt("useTexture", 0);
        

    }
    
    // Render the mesh
    mesh.render();
    
    // Restore OpenGL state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    
    glDisable(GL_BLEND);
}

void WeaponRenderer::renderWeaponTriangles(const Mesh& mesh,
                                          const Mat4& modelMatrix,
                                          const Camera& camera,
                                          const Vec3& color,
                                          const std::vector<unsigned int>& triangleIndices,
                                          bool useTexture) const {
    if (!isInitialized || !weaponShader) {
        std::cerr << "WeaponRenderer not properly initialized" << std::endl;
        return;
    }
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Disable depth testing for weapon overlay rendering
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for weapon rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use weapon shader
    weaponShader->use();
    
    // Set uniforms
    weaponShader->setMat4("model", modelMatrix);
    weaponShader->setMat4("projection", Engine::perspective(45.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 100.0f));
    weaponShader->setMat4("view", Mat4()); // Identity matrix for view
    
    // Set weapon color
    weaponShader->setVec3("color", color);
    
    // Handle texture rendering
    if (useTexture && weaponTexture && weaponTexture->isValid()) {
        weaponShader->setInt("useTexture", 1);
        weaponShader->setFloat("textureStrength", textureStrength);
        weaponTexture->bind(0);
        weaponShader->setInt("weaponTexture", 0);
    } else {
        weaponShader->setInt("useTexture", 0);
    }
    
    // Render only the specified triangles with the material color
    mesh.renderTriangles(triangleIndices);
    
    // Restore OpenGL state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    glDisable(GL_BLEND);
}

float WeaponRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& WeaponRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}

bool WeaponRenderer::loadWeaponTexture(const std::string& texturePath) {
    if (!weaponTexture) {
        std::cerr << "Weapon texture not initialized" << std::endl;
        return false;
    }
    
    return weaponTexture->loadFromFile(texturePath);
}

bool WeaponRenderer::initializeOpenGL() {
    // OpenGL is already initialized by the main renderer
    // Just verify we have a valid OpenGL context
    return true;
}

bool WeaponRenderer::loadWeaponShader() {
    weaponShader = std::make_unique<Shader>();
    
    if (!weaponShader->loadFromFiles("Resources/Shaders/weapon_vertex.glsl", 
                                    "Resources/Shaders/weapon_fragment.glsl")) {
        std::cerr << "Failed to load weapon shader files" << std::endl;
        return false;
    }
    
    std::cout << "Weapon shader loaded successfully" << std::endl;
    return true;
}

void WeaponRenderer::updateProjectionMatrix() {
    // Use perspective projection for weapon rendering
    float aspectRatio = getAspectRatio();
    projectionMatrix = Engine::perspective(45.0f, aspectRatio, 0.1f, 100.0f);
}

} // namespace Engine
