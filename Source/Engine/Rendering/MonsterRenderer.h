/**
 * MonsterRenderer.h - Specialized Renderer for 3D Monster Objects with Multi-Material Support
 * 
 * OVERVIEW:
 * Extends the basic renderer to support multi-material rendering for 3D monsters in world space.
 * Designed specifically for monsters that need to be rendered in 3D world coordinates,
 * unlike WeaponRenderer which is for FPS-style screen space overlays.
 * 
 * FEATURES:
 * - Multi-material support for OBJ models
 * - Material-based color rendering for each triangle group
 * - Proper 3D world-space rendering with depth testing
 * - Support for monster-specific shaders
 * - Damage flash effects
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
 * MonsterRenderer - Specialized Renderer for 3D Monster Objects
 * 
 * Provides multi-material rendering capabilities for 3D monsters in world space.
 * Designed to render monsters with proper 3D positioning, depth testing, and
 * multi-material support from MTL files.
 */
class MonsterRenderer : public Renderer {
private:
    int windowWidth;
    int windowHeight;
    Mat4 projectionMatrix;

    // Monster-specific shaders
    std::unique_ptr<Shader> monsterShader;
    std::unique_ptr<Texture> monsterTexture;
    
    // Rendering state
    bool isInitialized;
    bool useTextureRendering;
    float textureStrength;

public:
    MonsterRenderer();
    ~MonsterRenderer() override;

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

    // Monster-specific rendering
    void renderMonsterMesh(const Mesh& mesh,
                          const Mat4& modelMatrix,
                          const Camera& camera,
                          const Vec3& color,
                          bool useTexture = true) const;
                          
    // Multi-material rendering - render specific triangles with specific color
    void renderMonsterTriangles(const Mesh& mesh,
                               const Mat4& modelMatrix,
                               const Camera& camera,
                               const Vec3& color,
                               const std::vector<unsigned int>& triangleIndices,
                               bool useTexture = true) const;

    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    
    // Get shader for custom rendering
    Shader* getShader() const override { return monsterShader.get(); }

    // Texture management
    bool loadMonsterTexture(const std::string& texturePath);
    void setTextureStrength(float strength) { textureStrength = strength; }
    float getTextureStrength() const { return textureStrength; }
    void setUseTexture(bool use) { useTextureRendering = use; }
    bool getUseTexture() const { return useTextureRendering; }

private:
    bool initializeOpenGL();
    bool loadMonsterShader();
    void updateProjectionMatrix();
};

} // namespace Engine
