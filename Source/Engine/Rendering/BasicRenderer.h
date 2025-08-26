/**
 * BasicRenderer.h - Concrete OpenGL Renderer Implementation
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include <memory>

namespace Engine {

class BasicRenderer : public Renderer {
private:
    int windowWidth;
    int windowHeight;
    Mat4 projectionMatrix;

    // Shader used for world objects
    std::unique_ptr<Shader> objectShader;
    bool isInitialized;

public:
    BasicRenderer();
    ~BasicRenderer() override;

    // Renderer interface
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
    
    // Additional rendering method with coloring mode control
    void renderMesh(const Mesh& mesh,
                    const Mat4& modelMatrix,
                    const Camera& camera,
                    const Vec3& color,
                    bool useHeightColoring) const;

    void renderCrosshair(const Camera& camera) const override;

    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    
    // Get shader for custom rendering
    Shader* getShader() const { return objectShader.get(); }

private:
    bool initializeOpenGL();
    bool loadObjectShader();
    void updateProjectionMatrix();
};

} // namespace Engine


