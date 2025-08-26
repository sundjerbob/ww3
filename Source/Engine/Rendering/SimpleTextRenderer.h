/**
 * SimpleTextRenderer.h - Simplified Text Rendering System
 * 
 * OVERVIEW:
 * A simplified text rendering system that creates simple bitmap characters
 * for immediate use without external font dependencies.
 * 
 * FEATURES:
 * - Simple bitmap character generation
 * - Basic text positioning and scaling
 * - Color and transparency support
 * - No external font dependencies
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include <memory>
#include <string>

namespace Engine {

/**
 * SimpleTextRenderer - Basic Text Rendering System
 * 
 * Features:
 * - Renders simple bitmap text
 * - Supports text positioning, scaling, and coloring
 * - Handles transparency and blending
 * - No external dependencies
 */
class SimpleTextRenderer : public Renderer {
private:
    // Rendering
    std::unique_ptr<Shader> textShader;
    unsigned int quadVAO, quadVBO;
    unsigned int fontTexture;
    bool isInitialized;
    bool fontLoaded;
    
    // Window dimensions
    int windowWidth, windowHeight;
    
    // Character dimensions
    static const int CHAR_WIDTH = 8;
    static const int CHAR_HEIGHT = 16;
    static const int TEXTURE_WIDTH = 128;
    static const int TEXTURE_HEIGHT = 128;

public:
    // Constructor/Destructor
    SimpleTextRenderer();
    ~SimpleTextRenderer() override;
    
    // Initialization
    bool initialize(int width, int height) override;
    void cleanup() override;
    
    // Text rendering
    void renderText(const std::string& text, float x, float y, float scale, const Vec3& color) const;
    void renderTextCentered(const std::string& text, float x, float y, float scale, const Vec3& color) const;
    
    // Utility
    Vec2 getTextSize(const std::string& text, float scale) const;
    bool isFontLoaded() const { return fontLoaded; }
    
    // Renderer interface
    void beginFrame() override;
    void endFrame(GLFWwindow* window) override;
    void setViewport(int width, int height) override;
    void setClearColor(float r, float g, float b, float a = 1.0f) override;
    void renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color) const override;
    void renderCrosshair(const Camera& camera) const override;
    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    Shader* getShader() const override { return textShader.get(); }

private:
    // Internal methods
    bool createSimpleFontTexture();
    void setupQuad();
    void cleanupQuad();
    bool loadTextShader();
    void renderCharacter(char c, float x, float y, float scale, const Vec3& color) const;
};

} // namespace Engine
