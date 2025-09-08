/**
 * BasicRenderer.cpp - Concrete OpenGL Renderer Implementation
 */

#include "BasicRenderer.h"
#include <iostream>

namespace Engine {

BasicRenderer::BasicRenderer()
    : windowWidth(600), windowHeight(600), isInitialized(false) {}

BasicRenderer::~BasicRenderer() {
    cleanup();
}

bool BasicRenderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (!initializeOpenGL()) {
        return false;
    }

    if (!loadObjectShader()) {
        return false;
    }

    if (!loadTerrainShader()) {
        return false;
    }

    updateProjectionMatrix();
    setClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    isInitialized = true;
    return true;
}

bool BasicRenderer::initializeOpenGL() {
    glViewport(0, 0, static_cast<GLsizei>(windowWidth), static_cast<GLsizei>(windowHeight));
    glEnable(GL_DEPTH_TEST);
    return true;
}

bool BasicRenderer::loadObjectShader() {
    objectShader = std::make_unique<Shader>();
    return objectShader->loadFromFiles("Resources/Shaders/vertex.glsl", "Resources/Shaders/fragment.glsl");
}

bool BasicRenderer::loadTerrainShader() {
    terrainShader = std::make_unique<Shader>();
    return terrainShader->loadFromFiles("Resources/Shaders/terrain_vertex.glsl", "Resources/Shaders/terrain_fragment.glsl");
}

void BasicRenderer::updateProjectionMatrix() {
    const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const float fov = 45.0f * 3.14159f / 180.0f;
    const float nearPlane = 0.1f;
    const float farPlane = 100.0f;
    projectionMatrix = Engine::perspective(fov, aspect, nearPlane, farPlane);
}

void BasicRenderer::cleanup() {
    objectShader.reset();
    terrainShader.reset();
    isInitialized = false;
}

void BasicRenderer::beginFrame() {
    if (!isInitialized) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BasicRenderer::endFrame(GLFWwindow* window) {
    if (window) glfwSwapBuffers(window);
}

void BasicRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    updateProjectionMatrix();
}

void BasicRenderer::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void BasicRenderer::renderMesh(const Mesh& mesh,
                               const Mat4& modelMatrix,
                               const Camera& camera,
                               const Vec3& color) const {
    // Always use object colors (no height-based coloring)
    renderMesh(mesh, modelMatrix, camera, color, false);
}

void BasicRenderer::renderMesh(const Mesh& mesh,
                               const Mat4& modelMatrix,
                               const Camera& camera,
                               const Vec3& color,
                               bool useHeightColoring) const {
    if (!isInitialized) return;
    
    // Use terrain shader for height-based coloring, object shader for regular objects
    Shader* shader = useHeightColoring ? terrainShader.get() : objectShader.get();
    if (!shader) return;
    
    shader->use();
    
    // Set coloring mode based on parameter
    shader->setInt("useHeightColoring", useHeightColoring ? 1 : 0);

    shader->setMat4("model", modelMatrix);
    shader->setMat4("view", camera.getViewMatrix());
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setVec3("color", color);
    
    // Set lighting uniforms for terrain
    if (useHeightColoring) {
        // Directional light from the sun (slightly above and to the side)
        shader->setVec3("lightDirection", Vec3(0.5f, 0.8f, 0.3f));
        shader->setVec3("lightColor", Vec3(1.0f, 0.95f, 0.8f)); // Warm sunlight
        shader->setVec3("ambientColor", Vec3(0.3f, 0.3f, 0.4f)); // Blue-ish ambient
        shader->setFloat("ambientStrength", 0.3f);
        shader->setFloat("diffuseStrength", 0.7f);
    }
    
    mesh.render();
}

void BasicRenderer::renderCrosshair(const Camera& camera) const {}

float BasicRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& BasicRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}

} // namespace Engine


