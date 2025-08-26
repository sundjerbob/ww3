/**
 * CrosshairRenderer.h - Renderer for 2D crosshair overlay
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include <memory>

namespace Engine {

class CrosshairRenderer : public Renderer {
private:
    int windowWidth;
    int windowHeight;
    Mat4 projectionMatrix; // Unused but required by interface; identity

    std::unique_ptr<Shader> crosshairShader;
    bool isInitialized;

public:
    CrosshairRenderer();
    ~CrosshairRenderer() override;

    bool initialize(int width, int height) override;
    void cleanup() override;

    void beginFrame() override;
    void endFrame(GLFWwindow* window) override;

    void setViewport(int width, int height) override;
    void setClearColor(float r, float g, float b, float a = 1.0f) override;

    void renderMesh(const Mesh& mesh,
                    const Mat4& modelMatrix,
                    const Camera& camera,
                    const Vec3& color) const override;

    void renderCrosshair(const Camera& camera) const override;

    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    
    // Get shader for custom rendering
    Shader* getShader() const override { return crosshairShader.get(); }

private:
    bool loadCrosshairShader();
};

} // namespace Engine


