/**
 * TextRenderer.h - Text Rendering System using STB TrueType
 * 
 * OVERVIEW:
 * A text rendering system that uses STB TrueType library to render
 * text as textured quads. Supports TrueType fonts with proper
 * kerning and anti-aliasing.
 * 
 * FEATURES:
 * - TrueType font loading and rendering
 * - Character atlas generation
 * - Proper text positioning and scaling
 * - Color and transparency support
 * - Efficient batch rendering
 */

#pragma once
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>
#include <string>
#include <unordered_map>

// STB TrueType library (forward declaration)
struct stbtt_fontinfo;
struct stbtt_bakedchar;

namespace Engine {

/**
 * TextRenderer - Text Rendering System
 * 
 * Features:
 * - Loads and renders TrueType fonts
 * - Generates character atlases for efficient rendering
 * - Supports text positioning, scaling, and coloring
 * - Handles transparency and blending
 */
class TextRenderer : public Renderer {
private:
    // Font data
    stbtt_fontinfo fontInfo;
    unsigned char* fontData;
    int fontDataSize;
    bool fontLoaded;
    
    // Character atlas
    unsigned int atlasTexture;
    int atlasWidth, atlasHeight;
    std::unordered_map<char, stbtt_bakedchar> charData;
    
    // Rendering
    std::unique_ptr<Shader> textShader;
    unsigned int quadVAO, quadVBO;
    bool isInitialized;
    
    // Window dimensions
    int windowWidth, windowHeight;

public:
    // Constructor/Destructor
    TextRenderer();
    ~TextRenderer() override;
    
    // Initialization
    bool initialize(int width, int height) override;
    void cleanup() override;
    
    // Font loading
    bool loadFont(const std::string& fontPath, float fontSize = 32.0f);
    bool loadFontFromMemory(const unsigned char* data, int dataSize, float fontSize = 32.0f);
    bool loadDefaultFont(float fontSize = 32.0f);
    
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
    float getAspectRatio() const override;
    const Mat4& getProjectionMatrix() const override;
    Shader* getShader() const override { return textShader.get(); }

private:
    // Internal methods
    bool generateAtlas(float fontSize);
    void setupQuad();
    void cleanupQuad();
    bool loadTextShader();
};

} // namespace Engine
