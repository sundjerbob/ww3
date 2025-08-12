/**
 * CrosshairRenderer.cpp - Renderer for 2D crosshair overlay
 */

#include "CrosshairRenderer.h"
#include "Mesh.h"
#include "Renderer.h"
#include <vector>

namespace Engine {

CrosshairRenderer::CrosshairRenderer()
    : windowWidth(600), windowHeight(600), isInitialized(false) {
    // Initialize projection matrix as identity (unused for 2D overlay)
    projectionMatrix = Mat4();
}

CrosshairRenderer::~CrosshairRenderer() { cleanup(); }

bool CrosshairRenderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    isInitialized = loadCrosshairShader();
    return isInitialized;
}

void CrosshairRenderer::cleanup() {
    crosshairShader.reset();
    isInitialized = false;
}

void CrosshairRenderer::beginFrame() {}

void CrosshairRenderer::endFrame(GLFWwindow* /*window*/) {}

void CrosshairRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void CrosshairRenderer::setClearColor(float, float, float, float) {}

void CrosshairRenderer::renderMesh(const Mesh& mesh,
                                   const Mat4& modelMatrix,
                                   const Camera& camera,
                                   const Vec3& color) const {
    (void)color; // current fragment shader ignores color uniform
    if (!isInitialized || !crosshairShader) return;
    glDisable(GL_DEPTH_TEST);
    crosshairShader->use();
    Mat4 identity; // default constructor yields identity
    crosshairShader->setMat4("model", modelMatrix);
    crosshairShader->setMat4("view", identity);
    crosshairShader->setMat4("projection", identity);
    mesh.render();
    glEnable(GL_DEPTH_TEST);
}

void CrosshairRenderer::renderCrosshair(const Camera& camera) const {}

float CrosshairRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& CrosshairRenderer::getProjectionMatrix() const {
    return projectionMatrix;
}

bool CrosshairRenderer::loadCrosshairShader() {
    crosshairShader = std::make_unique<Shader>();
    return crosshairShader->loadFromFiles("Resources/Shaders/crosshair_vertex.glsl", "Resources/Shaders/crosshair_fragment.glsl");
}

} // namespace Engine


