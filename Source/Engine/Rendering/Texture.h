/**
 * Texture.h - OpenGL Texture Management System
 * 
 * OVERVIEW:
 * Handles loading, binding, and management of OpenGL textures.
 * Supports common image formats for weapon material textures.
 * 
 * FEATURES:
 * - Image loading from various formats
 * - OpenGL texture object management
 * - Automatic resource cleanup
 * - Texture parameter configuration
 */

#pragma once
#include <GL/glew.h>
#include <string>
#include <memory>

namespace Engine {

/**
 * Texture Class - OpenGL Texture Management
 * 
 * Handles loading and managing OpenGL textures:
 * - Image loading and format conversion
 * - OpenGL texture object creation and binding
 * - Automatic resource cleanup
 * - Texture parameter configuration
 */
class Texture {
private:
    unsigned int textureID;
    int width;
    int height;
    int channels;
    bool isInitialized;
    std::string filepath;

public:
    // Constructor/Destructor
    Texture();
    ~Texture();
    
    // Disable copy constructor and assignment, enable move semantics
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    
    // Texture loading and management
    bool loadFromFile(const std::string& path);
    bool loadFromMemory(const unsigned char* data, int w, int h, int ch);
    void bind(unsigned int slot = 0) const;
    void unbind() const;
    void cleanup();
    
    // Utility
    bool isValid() const { return isInitialized; }
    unsigned int getID() const { return textureID; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getChannels() const { return channels; }
    const std::string& getFilepath() const { return filepath; }
    
    // Texture parameters
    void setFiltering(GLenum minFilter, GLenum magFilter);
    void setWrapping(GLenum sWrap, GLenum tWrap);
    
private:
    // Helper methods
    bool initializeTexture();
    void setDefaultParameters();
    GLenum getFormat(int channels) const;
    GLenum getInternalFormat(int channels) const;
};

} // namespace Engine
