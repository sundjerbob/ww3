/**
 * WeaponRenderer.h - Specialized Renderer for Weapon Objects
 * 
 * OVERVIEW:
 * Extends the basic renderer to support texture rendering for weapons.
 * Maintains backward compatibility with solid color rendering.
 * 
 * FEATURES:
 * - Texture support for weapon materials
 * - Fallback to solid colors when no texture is available
 * - Specialized weapon shaders
 * - Proper blending for weapon overlays
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>
#include <vector>

namespace Engine {

class Mesh;
class Camera;

/**
 * WeaponRenderer - Specialized Renderer for Weapon Objects
 * 
 * Provides texture rendering capabilities for weapons while maintaining
 * backward compatibility with solid color rendering.
 */
class WeaponRenderer : public Renderer {
private:
    int windowWidth;
    int windowHeight;
    Mat4 projectionMatrix;

    // Weapon-specific shaders
    std::unique_ptr<Shader> weaponShader;
    std::unique_ptr<Texture> weaponTexture;
    
    // Rendering state
    bool isInitialized;
    bool useTextureRendering;
    float textureStrength;

public:
    WeaponRenderer();
    ~WeaponRenderer() override;

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

    // Weapon-specific rendering
    void renderWeaponMesh(const Mesh& mesh,
                         const Mat4& modelMatrix,
                         const Camera& camera,
                         const Vec3& color,
                         bool useTexture = true) const;
                         
    // Multi-material rendering - render specific triangles with specific color
    void renderWeaponTriangles(const Mesh& mesh,
                              const Mat4& modelMatrix,
                              const Camera& camera,
                              const Vec3& color,
                              const std::vector<unsigned int>& triangleIndices,
                              bool useTexture = true) const;


    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    
    // Get shader for custom rendering
    Shader* getShader() const override { return weaponShader.get(); }

    // Texture management
    bool loadWeaponTexture(const std::string& texturePath);
    void setTextureStrength(float strength) { textureStrength = strength; }
    float getTextureStrength() const { return textureStrength; }
    void setUseTexture(bool use) { useTextureRendering = use; }
    bool getUseTexture() const { return useTextureRendering; }

private:
    bool initializeOpenGL();
    bool loadWeaponShader();
    void updateProjectionMatrix();
};

} // namespace Engine
