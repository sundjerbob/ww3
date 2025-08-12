/**
 * Mesh.h - 3D Geometry Management System
 * 
 * OVERVIEW:
 * Encapsulates OpenGL vertex buffer management and geometry rendering.
 * Provides clean interface for creating, storing, and rendering 3D meshes.
 * 
 * FEATURES:
 * - Vertex Array Object (VAO) management
 * - Vertex Buffer Object (VBO) and Element Buffer Object (EBO) handling
 * - Automatic resource cleanup
 * - Simple rendering interface
 */

#pragma once
#include <vector>
#include <GL/glew.h>
#include "Math.h"

namespace Engine {

/**
 * Mesh Class - 3D Geometry Management
 * 
 * Handles OpenGL vertex data storage and rendering:
 * - Manages VAO, VBO, and EBO creation and cleanup
 * - Stores vertex and index data
 * - Provides simple rendering interface
 * - Automatic resource management
 */
class Mesh {
private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    bool isInitialized;

public:
    // Constructor/Destructor
    Mesh();
    ~Mesh();
    
    // Disable copy constructor and assignment, enable move semantics
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    
    // Mesh creation
    bool createMesh(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData);
    bool createMeshWithNormals(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData);
    bool createMeshWithTexCoords(const std::vector<float>& vertexData, const std::vector<unsigned int>& indexData);
    void cleanup();
    
    // Rendering
    void render() const;
    
    // Utility
    bool isValid() const { return isInitialized; }
    unsigned int getVertexCount() const { return static_cast<unsigned int>(vertices.size() / 3); } // Assuming 3 floats per vertex
    unsigned int getVertexCountWithNormals() const { return static_cast<unsigned int>(vertices.size() / 6); } // 6 floats per vertex (pos + normal)
    unsigned int getIndexCount() const { return static_cast<unsigned int>(indices.size()); }
    
    // Data access
    const std::vector<float>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    
    // Static helper methods for common shapes
    static Mesh createCube();
    static Mesh createGroundPlane(float size = 50.0f, float yPosition = -2.0f);
};

} // namespace Engine