/**
 * WaterRenderer.cpp - Implementation of Water Rendering System
 */

#include "WaterRenderer.h"
#include "../Math/Camera.h"
#include <iostream>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../../../Extern/Text/stb_image.h"

namespace Engine {

WaterRenderer::WaterRenderer()
    : windowWidth(0), windowHeight(0), projectionMatrix(Mat4()),
      duDvTexture(0), normalMapTexture(0), reflectionTexture(0), 
      refractionTexture(0), depthTexture(0), reflectionFBO(0), 
      refractionFBO(0), reflectionTextureID(0), refractionTextureID(0),
      refractionDepthTextureID(0), moveFactor(0.0f), waveSpeed(0.03f),
      distortionScale(0.01f), shineDamper(20.0f), reflectivity(0.6f),
      isInitialized(false) {
}

WaterRenderer::~WaterRenderer() {
    cleanup();
}

bool WaterRenderer::initialize(int width, int height) {
    if (isInitialized) return true;
    
    
    windowWidth = width;
    windowHeight = height;
    
    if (!initializeOpenGL()) {
        return false;
    }
    
    if (!loadWaterShader()) {
        return false;
    }
    
    if (!loadWaterTextures()) {
        return false;
    }
    
    if (!setupFramebuffers()) {
        return false;
    }
    
    updateProjectionMatrix();
    
    isInitialized = true;
    return true;
}

void WaterRenderer::cleanup() {
    if (!isInitialized) return;
    
    
    // Clean up textures
    if (duDvTexture) glDeleteTextures(1, &duDvTexture);
    if (normalMapTexture) glDeleteTextures(1, &normalMapTexture);
    if (reflectionTexture) glDeleteTextures(1, &reflectionTexture);
    if (refractionTexture) glDeleteTextures(1, &refractionTexture);
    if (depthTexture) glDeleteTextures(1, &depthTexture);
    
    // Clean up framebuffers
    if (reflectionFBO) glDeleteFramebuffers(1, &reflectionFBO);
    if (refractionFBO) glDeleteFramebuffers(1, &refractionFBO);
    if (reflectionTextureID) glDeleteTextures(1, &reflectionTextureID);
    if (refractionTextureID) glDeleteTextures(1, &refractionTextureID);
    if (refractionDepthTextureID) glDeleteTextures(1, &refractionDepthTextureID);
    
    // Clean up shader
    waterShader.reset();
    
    isInitialized = false;
}

void WaterRenderer::beginFrame() {
    if (!isInitialized) return;
    
    // Update move factor for wave animation
    updateMoveFactor(0.016f); // Assuming 60 FPS
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WaterRenderer::endFrame(GLFWwindow* window) {
    if (!isInitialized) return;
    
    glfwSwapBuffers(window);
}

void WaterRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    updateProjectionMatrix();
    glViewport(0, 0, width, height);
}

void WaterRenderer::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void WaterRenderer::renderMesh(const Mesh& mesh,
                               const Mat4& modelMatrix,
                               const Camera& camera,
                               const Vec3& color) const {
    // Default mesh rendering - delegate to water rendering
    renderWater(mesh, modelMatrix, camera, 0.0f);
}

void WaterRenderer::renderCrosshair(const Camera& camera) const {
    // Water renderer doesn't handle crosshair
}

float WaterRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& WaterRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}

void WaterRenderer::renderWater(const Mesh& mesh,
                                const Mat4& modelMatrix,
                                const Camera& camera,
                                float waterHeight) const {
    if (!isInitialized || !waterShader) return;
    
    waterShader->use();
    
    // Create a completely static model matrix for water
    // This bypasses any GameObject transformation issues
    Mat4 staticModelMatrix = Mat4(); // Identity matrix - no transformation
    
    // Set matrices
    waterShader->setMat4("model", staticModelMatrix);
    waterShader->setMat4("view", camera.getViewMatrix());
    waterShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Set camera position
    waterShader->setVec3("cameraPosition", camera.getPosition());
    
    // Set water parameters
    waterShader->setFloat("time", static_cast<float>(glfwGetTime()));
    waterShader->setFloat("moveFactor", moveFactor);
    waterShader->setFloat("distortionScale", distortionScale);
    waterShader->setFloat("shineDamper", shineDamper);
    waterShader->setFloat("reflectivity", reflectivity);
    waterShader->setFloat("waterHeight", waterHeight);
    
    // Bind water textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, duDvTexture);
    waterShader->setInt("duDvTexture", 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMapTexture);
    waterShader->setInt("normalMap", 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, reflectionTextureID);
    waterShader->setInt("reflectionTexture", 2);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, refractionTextureID);
    waterShader->setInt("refractionTexture", 3);
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, refractionDepthTextureID);
    waterShader->setInt("depthMap", 4);
    
    // Render the mesh
    mesh.render();
    
    // Unbind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void WaterRenderer::bindReflectionTexture() const {
    glBindTexture(GL_TEXTURE_2D, reflectionTextureID);
}

void WaterRenderer::bindRefractionTexture() const {
    glBindTexture(GL_TEXTURE_2D, refractionTextureID);
}

void WaterRenderer::unbindCurrentFramebuffer() const {
    unbindFramebuffer();
}

bool WaterRenderer::initializeOpenGL() {
    // Enable required OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

bool WaterRenderer::loadWaterShader() {
    waterShader = std::make_unique<Shader>();
    
    // Load water shaders from files
    if (!waterShader->loadFromFiles("Resources/Shaders/water_vertex.glsl", 
                                   "Resources/Shaders/water_fragment.glsl")) {
        return false;
    }
    
    return true;
}

bool WaterRenderer::loadWaterTextures() {
    
    // Load DuDv map
    int width, height, channels;
    unsigned char* data = stbi_load("Resources/Images/water_du_dv.png", &width, &height, &channels, 0);
    if (!data) {
        return false;
    }
    
    
    glGenTextures(1, &duDvTexture);
    glBindTexture(GL_TEXTURE_2D, duDvTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    
    // Load normal map
    data = stbi_load("Resources/Images/water_normals.png", &width, &height, &channels, 0);
    if (!data) {
        return false;
    }
    
    
    glGenTextures(1, &normalMapTexture);
    glBindTexture(GL_TEXTURE_2D, normalMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    
    return true;
}

bool WaterRenderer::setupFramebuffers() {
    
    // Create reflection framebuffer
    glGenFramebuffers(1, &reflectionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
    
    glGenTextures(1, &reflectionTextureID);
    glBindTexture(GL_TEXTURE_2D, reflectionTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTextureID, 0);
    
    
    // Create refraction framebuffer
    glGenFramebuffers(1, &refractionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
    
    glGenTextures(1, &refractionTextureID);
    glBindTexture(GL_TEXTURE_2D, refractionTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionTextureID, 0);
    
    
    glGenTextures(1, &refractionDepthTextureID);
    glBindTexture(GL_TEXTURE_2D, refractionDepthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, refractionDepthTextureID, 0);
    
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}

void WaterRenderer::updateProjectionMatrix() {
    float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    projectionMatrix = perspective(45.0f, aspectRatio, 0.1f, 1000.0f);
}

void WaterRenderer::updateMoveFactor(float deltaTime) {
    moveFactor += waveSpeed * deltaTime;
    moveFactor = fmod(moveFactor, 1.0f);
}

void WaterRenderer::bindReflectionFramebuffer() const {
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
    glViewport(0, 0, windowWidth, windowHeight);
}

void WaterRenderer::bindRefractionFramebuffer() const {
    glBindFramebuffer(GL_FRAMEBUFFER, refractionFBO);
    glViewport(0, 0, windowWidth, windowHeight);
}

void WaterRenderer::unbindFramebuffer() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
}

} // namespace Engine
