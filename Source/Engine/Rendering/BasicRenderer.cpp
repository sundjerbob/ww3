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
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return false;
    }

    if (!loadObjectShader()) {
        std::cerr << "Failed to load object shader" << std::endl;
        return false;
    }

    updateProjectionMatrix();
    setClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    isInitialized = true;
    return true;
}

bool BasicRenderer::initializeOpenGL() {
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    return true;
}

bool BasicRenderer::loadObjectShader() {
    objectShader = std::make_unique<Shader>();
    return objectShader->loadFromFiles("Resources/Shaders/vertex.glsl", "Resources/Shaders/fragment.glsl");
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
    glViewport(0, 0, width, height);
    updateProjectionMatrix();
}

void BasicRenderer::setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void BasicRenderer::renderMesh(const Mesh& mesh,
                               const Mat4& modelMatrix,
                               const Camera& camera,
                               const Vec3& color) const {
    if (!isInitialized || !objectShader) return;
    objectShader->use();
    objectShader->setMat4("model", modelMatrix);
    objectShader->setMat4("view", camera.getViewMatrix());
    objectShader->setMat4("projection", camera.getProjectionMatrix());
    objectShader->setVec3("color", color);
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


