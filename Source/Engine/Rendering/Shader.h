/**
 * Shader.h - OpenGL Shader Management System
 * 
 * OVERVIEW:
 * Encapsulates OpenGL shader creation, compilation, linking, and usage.
 * Provides a clean interface for loading shaders from files and managing uniforms.
 * 
 * FEATURES:
 * - Automatic shader compilation with error checking
 * - Uniform variable management
 * - Resource cleanup
 * - Easy shader program switching
 */

#pragma once
#include <string>
#include <GL/glew.h>
#include "../Math/Math.h"

namespace Engine {

/**
 * Shader Class - OpenGL Shader Program Management
 * 
 * Handles the complete lifecycle of OpenGL shader programs:
 * - Loading vertex and fragment shaders from files
 * - Compiling and linking shader programs
 * - Managing uniform variables
 * - Proper resource cleanup
 */
class Shader {
private:
    unsigned int programID;
    bool isValid;
    
    // Helper methods
    std::string loadShaderFromFile(const std::string& filePath);
    unsigned int compileShader(unsigned int type, const char* source);

public:
    // Constructor/Destructor
    Shader();
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    // Disable copy constructor and assignment (use move semantics if needed)
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    // Shader management
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromStrings(const char* vertexSource, const char* fragmentSource);
    void use() const;
    void cleanup();
    
    // Uniform setters
    void setMat4(const std::string& name, const Mat4& matrix) const;
    void setMat3(const std::string& name, const Mat3& matrix) const;
    void setVec3(const std::string& name, const Vec3& vector) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;
    
    // Utility
    bool isValidShader() const { return isValid; }
    unsigned int getProgramID() const { return programID; }
    int getUniformLocation(const std::string& name) const;
};

} // namespace Engine