/**
 * WaterRenderer.h - Specialized Renderer for Water Effects
 * 
 * Handles water rendering with reflection, refraction, and wave animation.
 * Uses multiple render passes and texture sampling for realistic water effects.
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include "Mesh.h"
#include <memory>
#include <GL/glew.h>

namespace Engine {

class WaterRenderer : public Renderer {
private:
    int windowWidth;
    int windowHeight;
    Mat4 projectionMatrix;

    // Water-specific shader
    std::unique_ptr<Shader> waterShader;
    
    // Water textures
    GLuint duDvTexture;
    GLuint normalMapTexture;
    GLuint reflectionTexture;
    GLuint refractionTexture;
    GLuint depthTexture;
    
    // Framebuffers for reflection/refraction
    GLuint reflectionFBO;
    GLuint refractionFBO;
    GLuint reflectionTextureID;
    GLuint refractionTextureID;
    GLuint refractionDepthTextureID;
    
    // Water parameters
    float moveFactor;
    float waveSpeed;
    float distortionScale;
    float shineDamper;
    float reflectivity;
    
    bool isInitialized;

public:
    WaterRenderer();
    ~WaterRenderer() override;

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

    void renderCrosshair(const Camera& camera) const override;

    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    
    // Get shader for custom rendering
    Shader* getShader() const { return waterShader.get(); }

    // Water-specific methods
    void renderWater(const Mesh& mesh,
                     const Mat4& modelMatrix,
                     const Camera& camera,
                     float waterHeight) const;
    
    void bindReflectionTexture() const;
    void bindRefractionTexture() const;
    void unbindCurrentFramebuffer() const;
    
    // Framebuffer management (public for game loop access)
    void bindReflectionFramebuffer() const;
    void bindRefractionFramebuffer() const;
    void unbindFramebuffer() const;
    
    // Water parameter setters
    void setWaveSpeed(float speed) { waveSpeed = speed; }
    void setDistortionScale(float scale) { distortionScale = scale; }
    void setShineDamper(float damper) { shineDamper = damper; }
    void setReflectivity(float reflect) { reflectivity = reflect; }

private:
    bool initializeOpenGL();
    bool loadWaterShader();
    bool loadWaterTextures();
    bool setupFramebuffers();
    void updateProjectionMatrix();
    void updateMoveFactor(float deltaTime);
};

} // namespace Engine
