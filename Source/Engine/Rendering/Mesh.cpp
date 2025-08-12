/**
 * Mesh.cpp - Implementation of 3D Geometry Management System
 * 
 * Provides complete mesh lifecycle management with OpenGL buffer handling
 * and common mesh creation utilities.
 */

#include "Mesh.h"
#include <utility> // For std::move

namespace Engine {

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), isInitialized(false) {}

Mesh::~Mesh() {
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept 
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO), 
      vertices(std::move(other.vertices)), indices(std::move(other.indices)),
      isInitialized(other.isInitialized) {
    // Reset the moved-from object
    other.VAO = other.VBO = other.EBO = 0;
    other.isInitialized = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        // Clean up current resources
        cleanup();
        
        // Move resources from other
        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        vertices = std::move(other.vertices);
        indices = std::move(other.indices);
        isInitialized = other.isInitialized;
        
        // Reset the moved-from object
        other.VAO = other.VBO = other.EBO = 0;
        other.isInitialized = false;
    }
    return *this;
}

bool Mesh::createMesh(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData) {
    // Clean up any existing mesh
    cleanup();
    
    // Store data
    vertices = vertexData;
    indices = indexData;
    
    // Generate OpenGL objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Configure vertex attributes (assuming position only: 3 floats per vertex)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindVertexArray(0);
    
    isInitialized = true;
    return true;
}

bool Mesh::createMeshWithNormals(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData) {
    // Clean up any existing mesh
    cleanup();
    
    // Store data
    vertices = vertexData;
    indices = indexData;
    
    // Generate OpenGL objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Configure vertex attributes for position + normal (6 floats per vertex)
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindVertexArray(0);
    
    isInitialized = true;
    return true;
}

bool Mesh::createMeshWithTexCoords(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData) {
    // Clean up any existing mesh
    cleanup();
    
    // Store data
    vertices = vertexData;
    indices = indexData;
    
    // Generate OpenGL objects
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Configure vertex attributes for position + texture coordinates (5 floats per vertex)
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindVertexArray(0);
    
    isInitialized = true;
    return true;
}

void Mesh::cleanup() {
    if (isInitialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
        isInitialized = false;
    }
    vertices.clear();
    indices.clear();
}

void Mesh::render() const {
    if (isInitialized) {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

// Static helper methods for common shapes

Mesh Mesh::createCube() {
    std::vector<float> cubeVertices = {
        // Front face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        // Back face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    std::vector<unsigned int> cubeIndices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        7, 3, 0, 0, 4, 7,
        // Right face
        1, 5, 6, 6, 2, 1,
        // Top face
        3, 2, 6, 6, 7, 3,
        // Bottom face
        0, 1, 5, 5, 4, 0
    };
    
    Mesh cube;
    cube.createMesh(cubeVertices, cubeIndices);
    return cube;
}

Mesh Mesh::createGroundPlane(float size, float yPosition) {
    float halfSize = size * 0.5f;
    
    std::vector<float> groundVertices = {
        // Ground plane vertices (large horizontal quad)
        -halfSize, yPosition, -halfSize,  // Bottom-left
         halfSize, yPosition, -halfSize,  // Bottom-right  
         halfSize, yPosition,  halfSize,  // Top-right
        -halfSize, yPosition,  halfSize   // Top-left
    };

    std::vector<unsigned int> groundIndices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    Mesh ground;
    ground.createMesh(groundVertices, groundIndices);
    return ground;
}

} // namespace Engine