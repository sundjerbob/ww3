/**
 * SimpleTextRenderer.cpp - Implementation of Simple Text Rendering System
 */

#include "SimpleTextRenderer.h"
#include "../Math/Math.h"
#include <iostream>
#include <vector>

namespace Engine {

SimpleTextRenderer::SimpleTextRenderer()
    : quadVAO(0),
      quadVBO(0),
      fontTexture(0),
      isInitialized(false),
      fontLoaded(false),
      windowWidth(800),
      windowHeight(600) {
}

SimpleTextRenderer::~SimpleTextRenderer() {
    cleanup();
}

bool SimpleTextRenderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Load text shader
    if (!loadTextShader()) {
        return false;
    }
    
    // Setup quad for text rendering
    setupQuad();
    
    // Create simple font texture
    if (!createSimpleFontTexture()) {
        return false;
    }
    fontLoaded = true;
    
    isInitialized = true;
    return true;
}

void SimpleTextRenderer::cleanup() {
    if (!isInitialized) return;
    
    cleanupQuad();
    
    if (fontTexture) {
        glDeleteTextures(1, &fontTexture);
        fontTexture = 0;
    }
    
    textShader.reset();
    isInitialized = false;
    
}

bool SimpleTextRenderer::loadTextShader() {
    textShader = std::make_unique<Shader>("Resources/Shaders/text_vertex.glsl",
                                          "Resources/Shaders/text_fragment.glsl");
    
    // Check if shader loaded properly
    if (!textShader) {
        return false;
    }
    
    // Try to use the shader to check if it's valid
    textShader->use();
    if (glGetError() != GL_NO_ERROR) {
        return false;
    }
    

    return true;
}

bool SimpleTextRenderer::createSimpleFontTexture() {
    // Create a simple bitmap font texture with basic ASCII characters
    std::vector<unsigned char> fontData(TEXTURE_WIDTH * TEXTURE_HEIGHT, 0);
    
    // For simplicity, we'll create simple block characters for digits and letters
    // This is a very basic implementation - normally you'd load a proper bitmap font
    
    // Create simple patterns for digits 0-9
    // Each character is 8x16 pixels
    const int charsPerRow = TEXTURE_WIDTH / CHAR_WIDTH;
    
    // Simple patterns for digits (very basic)
    unsigned char digitPatterns[10][16] = {
        // 0
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},
        // 1
        {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E},
        // 2
        {0x3C, 0x66, 0x66, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x7E},
        // 3
        {0x3C, 0x66, 0x66, 0x06, 0x06, 0x1C, 0x1C, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x66, 0x3C},
        // 4
        {0x0C, 0x1C, 0x3C, 0x6C, 0x6C, 0xCC, 0xCC, 0xFE, 0xFE, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C},
        // 5
        {0x7E, 0x60, 0x60, 0x60, 0x60, 0x7C, 0x7E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x66, 0x66, 0x66, 0x3C},
        // 6
        {0x3C, 0x66, 0x60, 0x60, 0x60, 0x7C, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},
        // 7
        {0x7E, 0x66, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30},
        // 8
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},
        // 9
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x3E, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3C}
    };
    
    // Place digit patterns in the texture
    for (int digit = 0; digit < 10; digit++) {
        int charIndex = digit + 48; // ASCII '0' = 48
        int charX = (charIndex % charsPerRow) * CHAR_WIDTH;
        int charY = (charIndex / charsPerRow) * CHAR_HEIGHT;
        
        for (int y = 0; y < CHAR_HEIGHT; y++) {
            unsigned char row = digitPatterns[digit][y];
            for (int x = 0; x < CHAR_WIDTH; x++) {
                int texX = charX + x;
                int texY = charY + y;
                int texIndex = texY * TEXTURE_WIDTH + texX;
                
                if (texIndex < fontData.size()) {
                    // Set pixel based on bit pattern
                    fontData[texIndex] = (row & (0x80 >> x)) ? 255 : 0;
                }
            }
        }
    }
    
    // Add simple patterns for letters and symbols we might need
    // For now, just make them solid blocks so they're visible
    for (int c = 65; c <= 90; c++) { // A-Z
        int charX = (c % charsPerRow) * CHAR_WIDTH;
        int charY = (c / charsPerRow) * CHAR_HEIGHT;
        
        for (int y = 0; y < CHAR_HEIGHT; y++) {
            for (int x = 0; x < CHAR_WIDTH; x++) {
                int texX = charX + x;
                int texY = charY + y;
                int texIndex = texY * TEXTURE_WIDTH + texX;
                
                if (texIndex < fontData.size()) {
                    // Simple block pattern for letters
                    if (x > 0 && x < CHAR_WIDTH-1 && y > 0 && y < CHAR_HEIGHT-1) {
                        fontData[texIndex] = 255;
                    }
                }
            }
        }
    }
    
    // Create OpenGL texture
    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, fontData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    

    return true;
}

void SimpleTextRenderer::setupQuad() {
    // Create a simple quad for character rendering
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

void SimpleTextRenderer::cleanupQuad() {
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
}

void SimpleTextRenderer::renderText(const std::string& text, float x, float y, float scale, const Vec3& color) const {
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
    Mat4 projection = Engine::orthographic(0.0f, static_cast<float>(windowWidth),
                                          0.0f, static_cast<float>(windowHeight),
                                          -1.0f, 1.0f);
    textShader->setMat4("projection", projection);
    textShader->setVec3("textColor", color);
    textShader->setFloat("alpha", 1.0f);
    
    // Bind font texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    textShader->setInt("textTexture", 0);
    
    // Render each character
    float currentX = x;
    for (char c : text) {
        renderCharacter(c, currentX, y, scale, color);
        currentX += CHAR_WIDTH * scale;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void SimpleTextRenderer::renderCharacter(char c, float x, float y, float scale, const Vec3& color) const {
    // Calculate texture coordinates for this character
    const int charsPerRow = TEXTURE_WIDTH / CHAR_WIDTH;
    int charIndex = static_cast<int>(c);
    
    float charX = (charIndex % charsPerRow) * CHAR_WIDTH;
    float charY = (charIndex / charsPerRow) * CHAR_HEIGHT;
    
    float u0 = charX / static_cast<float>(TEXTURE_WIDTH);
    float v0 = charY / static_cast<float>(TEXTURE_HEIGHT);
    float u1 = (charX + static_cast<float>(CHAR_WIDTH)) / static_cast<float>(TEXTURE_WIDTH);
    float v1 = (charY + static_cast<float>(CHAR_HEIGHT)) / static_cast<float>(TEXTURE_HEIGHT);
    
    float w = CHAR_WIDTH * scale;
    float h = CHAR_HEIGHT * scale;
    
    // Update VBO for this character
    float vertices[6][4] = {
        { x,     y + h,   u0, v0 },
        { x,     y,       u0, v1 },
        { x + w, y,       u1, v1 },
        
        { x,     y + h,   u0, v0 },
        { x + w, y,       u1, v1 },
        { x + w, y + h,   u1, v0 }
    };
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SimpleTextRenderer::renderTextCentered(const std::string& text, float x, float y, float scale, const Vec3& color) const {
    Vec2 textSize = getTextSize(text, scale);
    renderText(text, x - textSize.x * 0.5f, y, scale, color);
}

Vec2 SimpleTextRenderer::getTextSize(const std::string& text, float scale) const {
    float width = text.length() * CHAR_WIDTH * scale;
    float height = CHAR_HEIGHT * scale;
    return Vec2(width, height);
}

// Renderer interface implementations
void SimpleTextRenderer::beginFrame() {
    // Not needed for text renderer
}

void SimpleTextRenderer::endFrame(GLFWwindow* window) {
    // Not needed for text renderer
}

void SimpleTextRenderer::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void SimpleTextRenderer::setClearColor(float r, float g, float b, float a) {
    // Not needed for text renderer
}

void SimpleTextRenderer::renderMesh(const Mesh& mesh, const Mat4& modelMatrix, const Camera& camera, const Vec3& color) const {
    // Not used for text rendering
}

void SimpleTextRenderer::renderCrosshair(const Camera& camera) const {
    // Not used for text rendering - SimpleTextRenderer is only for text
}

float SimpleTextRenderer::getAspectRatio() const {
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}

const Mat4& SimpleTextRenderer::getProjectionMatrix() const {
    static Mat4 projection = Engine::orthographic(0.0f, static_cast<float>(windowWidth),
                                                  0.0f, static_cast<float>(windowHeight),
                                                  -1.0f, 1.0f);
    return projection;
}

} // namespace Engine
