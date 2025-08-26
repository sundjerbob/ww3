/**
 * Texture.cpp - Implementation of OpenGL Texture Management System
 */

#include "Texture.h"
#include <iostream>
#include <fstream>
#include <vector>

namespace Engine {

Texture::Texture()
    : textureID(0), width(0), height(0), channels(0), isInitialized(false) {}

Texture::~Texture() {
    cleanup();
}

Texture::Texture(Texture&& other) noexcept
    : textureID(other.textureID), width(other.width), height(other.height), 
      channels(other.channels), isInitialized(other.isInitialized), filepath(std::move(other.filepath)) {
    other.textureID = 0;
    other.isInitialized = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        cleanup();
        textureID = other.textureID;
        width = other.width;
        height = other.height;
        channels = other.channels;
        isInitialized = other.isInitialized;
        filepath = std::move(other.filepath);
        
        other.textureID = 0;
        other.isInitialized = false;
    }
    return *this;
}

bool Texture::loadFromFile(const std::string& path) {
    filepath = path;
    
    // For now, create a simple procedural texture
    // In a full implementation, this would load actual image files
    // For weapon materials, we'll create a basic metallic texture pattern
    
    std::cout << "Loading texture from: " << path << std::endl;
    
    // Create a simple 64x64 metallic texture pattern
    width = 64;
    height = 64;
    channels = 3; // RGB
    
    std::vector<unsigned char> textureData(width * height * channels);
    
    // Generate a simple metallic pattern
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * channels;
            
            // Create a subtle metallic pattern
            float noise = (sin(x * 0.1f) + cos(y * 0.1f)) * 0.1f;
            unsigned char value = static_cast<unsigned char>(128 + noise * 127);
            
            // Metallic gray with slight variation
            textureData[index] = value;     // R
            textureData[index + 1] = value; // G  
            textureData[index + 2] = value; // B
        }
    }
    
    return loadFromMemory(textureData.data(), width, height, channels);
}

bool Texture::loadFromMemory(const unsigned char* data, int w, int h, int ch) {
    width = w;
    height = h;
    channels = ch;
    
    if (!initializeTexture()) {
        return false;
    }
    
    // Upload texture data
    GLenum format = getFormat(channels);
    GLenum internalFormat = getInternalFormat(channels);
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set default parameters
    setDefaultParameters();
    
    isInitialized = true;
    
    std::cout << "Texture loaded successfully: " << width << "x" << height << " (" << channels << " channels)" << std::endl;
    return true;
}

void Texture::bind(unsigned int slot) const {
    if (!isInitialized) return;
    
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::cleanup() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    isInitialized = false;
    width = height = channels = 0;
}

void Texture::setFiltering(GLenum minFilter, GLenum magFilter) {
    if (!isInitialized) return;
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::setWrapping(GLenum sWrap, GLenum tWrap) {
    if (!isInitialized) return;
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
}

bool Texture::initializeTexture() {
    glGenTextures(1, &textureID);
    if (textureID == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        return false;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    return true;
}

void Texture::setDefaultParameters() {
    // Set default filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Set default wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

GLenum Texture::getFormat(int channels) const {
    switch (channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGB;
    }
}

GLenum Texture::getInternalFormat(int channels) const {
    switch (channels) {
        case 1: return GL_RED;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGB;
    }
}

} // namespace Engine
