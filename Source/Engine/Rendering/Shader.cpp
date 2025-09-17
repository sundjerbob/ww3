/**
 * Shader.cpp - Implementation of OpenGL Shader Management System
 * 
 * Provides complete shader lifecycle management with proper error handling
 * and resource cleanup for OpenGL shader programs.
 */

#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace Engine {

Shader::Shader() : programID(0), isValid(false) {}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) 
    : programID(0), isValid(false) {
    loadFromFiles(vertexPath, fragmentPath);
}

Shader::~Shader() {
    cleanup();
}

std::string Shader::loadShaderFromFile(const std::string& filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;
    
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        shaderFile.open(filePath);
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        return "";
    }
}

unsigned int Shader::compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    // Clean up any existing shader
    cleanup();
    
    // Load shader sources from files
    std::string vertexShaderSource = loadShaderFromFile(vertexPath);
    std::string fragmentShaderSource = loadShaderFromFile(fragmentPath);
    
    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return false;
    }
    
    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource.c_str());
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource.c_str());
    
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return false;
    }

    // Create shader program
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        glDeleteProgram(programID);
        programID = 0;
        isValid = false;
    } else {
        isValid = true;
    }

    // Clean up individual shaders (they're now part of the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return isValid;
}

bool Shader::loadFromStrings(const char* vertexSource, const char* fragmentSource) {
    // Clean up any existing shader
    cleanup();
    
    if (!vertexSource || !fragmentSource) {
        return false;
    }
    
    // Compile shaders
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) glDeleteShader(vertexShader);
        if (fragmentShader != 0) glDeleteShader(fragmentShader);
        return false;
    }

    // Create shader program
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        glDeleteProgram(programID);
        programID = 0;
        isValid = false;
    } else {
        isValid = true;
    }

    // Clean up individual shaders (they're now part of the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return isValid;
}

void Shader::use() const {
    if (isValid) {
        glUseProgram(programID);
    }
}

void Shader::cleanup() {
    if (programID != 0) {
        glDeleteProgram(programID);
        programID = 0;
    }
    isValid = false;
}

int Shader::getUniformLocation(const std::string& name) const {
    if (!isValid) return -1;
    return glGetUniformLocation(programID, name.c_str());
}

void Shader::setMat4(const std::string& name, const Mat4& matrix) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, matrix.data());
    }
}

void Shader::setMat3(const std::string& name, const Mat3& matrix) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix3fv(location, 1, GL_FALSE, matrix.data());
    }
}

void Shader::setVec3(const std::string& name, const Vec3& vector) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform3f(location, vector.x, vector.y, vector.z);
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setInt(const std::string& name, int value) const {
    int location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

} // namespace Engine