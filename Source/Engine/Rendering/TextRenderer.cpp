/**
 * TextRenderer.cpp - Implementation of Text Rendering System using STB TrueType
 */

#include "TextRenderer.h"
#include "../Math/Math.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

// Include STB implementation
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../Extern/Text/stb_truetype.h"

namespace Engine {

TextRenderer::TextRenderer()
    : fontData(nullptr),
      fontDataSize(0),
      fontLoaded(false),
      atlasTexture(0),
      atlasWidth(512),
      atlasHeight(512),
      quadVAO(0),
      quadVBO(0),
      isInitialized(false),
      windowWidth(800),
      windowHeight(600) {
}

TextRenderer::~TextRenderer() {
    cleanup();
}

bool TextRenderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Load text shader
    if (!loadTextShader()) {
        return false;
    }
    
    // Setup quad for text rendering
    setupQuad();
    
    // Generate atlas texture
    glGenTextures(1, &atlasTexture);
    
    isInitialized = true;
    return true;
}

void TextRenderer::cleanup() {
    if (!isInitialized) return;
    
    cleanupQuad();
    
    if (atlasTexture) {
        glDeleteTextures(1, &atlasTexture);
        atlasTexture = 0;
    }
    
    if (fontData) {
        delete[] fontData;
        fontData = nullptr;
    }
    
    textShader.reset();
    fontLoaded = false;
    isInitialized = false;
    
}

bool TextRenderer::loadTextShader() {
    textShader = std::make_unique<Shader>("Resources/Shaders/text_vertex.glsl",
                                          "Resources/Shaders/text_fragment.glsl");
    
    if (!textShader || !textShader->isValid()) {
        return false;
    }
    
    return true;
}

bool TextRenderer::loadFont(const std::string& fontPath, float fontSize) {
    // Try to load a system font first, then fallback to embedded font
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        // Create a simple default font (this would normally be embedded font data)
        return loadDefaultFont(fontSize);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    fontDataSize = static_cast<int>(size);
    fontData = new unsigned char[fontDataSize];
    
    if (!file.read(reinterpret_cast<char*>(fontData), size)) {
        delete[] fontData;
        fontData = nullptr;
        return false;
    }
    
    return loadFontFromMemory(fontData, fontDataSize, fontSize);
}

bool TextRenderer::loadFontFromMemory(const unsigned char* data, int dataSize, float fontSize) {
    if (!data || dataSize <= 0) {
        return false;
    }
    
    // Initialize font
    if (!stbtt_InitFont(&fontInfo, data, stbtt_GetFontOffsetForIndex(data, 0))) {
        return false;
    }
    
    // Generate character atlas
    if (!generateAtlas(fontSize)) {
        return false;
    }
    
    fontLoaded = true;
    return true;
}

bool TextRenderer::loadDefaultFont(float fontSize) {
    // For now, we'll create a simple bitmap font
    // In a real implementation, you'd embed a default font
    
    // Create a simple white texture for now
    atlasWidth = 256;
    atlasHeight = 256;
    
    std::vector<unsigned char> whiteTexture(atlasWidth * atlasHeight, 255);
    
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, whiteTexture.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    fontLoaded = true;
    return true;
}

bool TextRenderer::generateAtlas(float fontSize) {
    // Allocate bitmap
    std::vector<unsigned char> bitmap(atlasWidth * atlasHeight);
    
    // Character range (ASCII printable characters)
    int firstChar = 32;  // Space
    int numChars = 96;   // Printable ASCII characters
    
    // Allocate character data
    std::vector<stbtt_bakedchar> bakedChars(numChars);
    
    // Bake characters into bitmap
    stbtt_BakeFontBitmap(fontData, 0, fontSize, bitmap.data(), atlasWidth, atlasHeight,
                         firstChar, numChars, bakedChars.data());
    
    // Store character data
    charData.clear();
    for (int i = 0; i < numChars; ++i) {
        char c = static_cast<char>(firstChar + i);
        charData[c] = bakedChars[i];
    }
    
    // Upload texture to GPU
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void TextRenderer::setupQuad() {
    // Create a simple quad for text rendering
    float vertices[] = {
        // positions   // texture coords
         0.0f,  1.0f,   0.0f, 0.0f,
         1.0f,  0.0f,   1.0f, 1.0f,
         0.0f,  0.0f,   0.0f, 1.0f,
         
         0.0f,  1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 0.0f,
         1.0f,  0.0f,   1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRenderer::cleanupQuad() {
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}

void TextRenderer::renderText(const std::string& text, float x, float y, float scale, const Vec3& color) const {
    if (!isInitialized || !fontLoaded || !textShader) {
        return;
    }
    
    // Enable blending for text transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    // Use text shader
    textShader->use();
    
    // Set projection matrix (orthographic for 2D text)
    Mat4 projection = Engine::ortho(0.0f, static_cast<float>(windowWidth),
                                   0.0f, static_cast<float>(windowHeight));
    textShader->setMat4("projection", projection);
    textShader->setVec3("textColor", color);
    textShader->setFloat("alpha", 1.0f);
    
    // Bind font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    textShader->setInt("textTexture", 0);
    
    glBindVertexArray(quadVAO);
    
    // Render each character
    float currentX = x;
    for (char c : text) {
        if (charData.find(c) != charData.end()) {
            stbtt_bakedchar& ch = charData[c];
            
            float w = (ch.x1 - ch.x0) * scale;
            float h = (ch.y1 - ch.y0) * scale;
            float xpos = currentX + ch.xoff * scale;
            float ypos = y - (ch.y1 - ch.y0) * scale - ch.yoff * scale;
            
            // Update VBO for each character
            float vertices[6][4] = {
                { xpos,     ypos + h,   ch.x0/atlasWidth, ch.y0/atlasHeight },
                { xpos,     ypos,       ch.x0/atlasWidth, ch.y1/atlasHeight },
                { xpos + w, ypos,       ch.x1/atlasWidth, ch.y1/atlasHeight },
                
                { xpos,     ypos + h,   ch.x0/atlasWidth, ch.y0/atlasHeight },
                { xpos + w, ypos,       ch.x1/atlasWidth, ch.y1/atlasHeight },
                { xpos + w, ypos + h,   ch.x1/atlasWidth, ch.y0/atlasHeight }
            };
            
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            currentX += ch.xadvance * scale;
        } else {
            // For missing characters, advance by a default amount
            currentX += 10.0f * scale;
        }
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void TextRenderer::renderTextCentered(const std::string& text, float x, float y, float scale, const Vec3& color) const {
    Vec2 textSize = getTextSize(text, scale);
    renderText(text, x - textSize.x * 0.5f, y, scale, color);
}

Vec2 TextRenderer::getTextSize(const std::string& text, float scale) const {
    if (!fontLoaded) {
        return Vec2(0.0f, 0.0f);
    }
    
    float width = 0.0f;
    float height = 0.0f;
    
    for (char c : text) {
        if (charData.find(c) != charData.end()) {
            const stbtt_bakedchar& ch = charData.at(c);
            width += ch.xadvance * scale;
            height = std::max(height, (ch.y1 - ch.y0) * scale);
        } else {
            width += 10.0f * scale; // Default character width
        }
    }
    
    return Vec2(width, height);
}

// Renderer interface implementations
void TextRenderer::beginFrame() {
    // Not needed for text renderer
}

void TextRenderer::endFrame(GLFWwindow* window) {
    // Not needed for text renderer
}

void TextRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void TextRenderer::setClearColor(float r, float g, float b, float a) {
    // Not needed for text renderer
}

void TextRenderer::renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color) const {
    // Not used for text rendering
}

float TextRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& TextRenderer::getProjectionMatrix() const {
    static Mat4 projection = Engine::ortho(0.0f, static_cast<float>(windowWidth),
                                          0.0f, static_cast<float>(windowHeight));
    return projection;
}

} // namespace Engine
