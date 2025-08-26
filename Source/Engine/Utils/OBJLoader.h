/**
 * OBJLoader.h - Wavefront OBJ File Loading System
 * 
 * OVERVIEW:
 * Loads 3D models from Wavefront OBJ files, a standard format for 3D geometry.
 * Parses vertex positions, normals, texture coordinates, and face indices.
 * 
 * FEATURES:
 * - Efficient streaming parsing for large OBJ files
 * - Support for triangulated meshes
 * - Vertex normal and texture coordinate handling
 * - Memory-efficient data structures
 * - Integration with existing Mesh system
 * 
 * OBJ FORMAT SUPPORT:
 * - v x y z (vertex positions)
 * - vn x y z (vertex normals)
 * - vt u v (texture coordinates)
 * - f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 (faces)
 */

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include "../Math/Math.h"
#include "../Rendering/Material.h"

namespace Engine {

// Logging callback types
using LogInfoCallback = std::function<void(const std::string&)>;
using LogWarningCallback = std::function<void(const std::string&)>;
using LogErrorCallback = std::function<void(const std::string&)>;

/**
 * OBJ Mesh Data Structure
 * 
 * Contains parsed data from OBJ file ready for GPU upload.
 * Interleaved vertex format: [position.x, position.y, position.z, normal.x, normal.y, normal.z, texCoord.u, texCoord.v]
 */
struct OBJMeshData {
    std::vector<float> vertices;        // Interleaved position + normal + texture coordinate data
    std::vector<unsigned int> indices;  // Triangle indices
    unsigned int vertexCount = 0;
    unsigned int triangleCount = 0;
    
    // Material information
    MaterialLibrary materials;          // All materials loaded from .mtl file
    std::vector<std::string> faceMaterials; // Material name for each face/triangle
    
    // Mesh statistics
    Vec3 boundingBoxMin;
    Vec3 boundingBoxMax;
    Vec3 center;
    
    bool isValid() const {
        return !vertices.empty() && !indices.empty() && (indices.size() % 3 == 0) && (vertices.size() % 8 == 0);
    }
    
    void calculateBounds() {
        if (vertices.size() < 8) return; // Need at least one vertex (8 floats: pos + normal + texcoord)
        
        boundingBoxMin = Vec3(vertices[0], vertices[1], vertices[2]);
        boundingBoxMax = boundingBoxMin;
        
        // Iterate through positions (every 8 floats, take first 3)
        for (size_t i = 0; i < vertices.size(); i += 8) {
            Vec3 pos(vertices[i], vertices[i + 1], vertices[i + 2]);
            
            if (pos.x < boundingBoxMin.x) boundingBoxMin.x = pos.x;
            if (pos.y < boundingBoxMin.y) boundingBoxMin.y = pos.y;
            if (pos.z < boundingBoxMin.z) boundingBoxMin.z = pos.z;
            
            if (pos.x > boundingBoxMax.x) boundingBoxMax.x = pos.x;
            if (pos.y > boundingBoxMax.y) boundingBoxMax.y = pos.y;
            if (pos.z > boundingBoxMax.z) boundingBoxMax.z = pos.z;
        }
        
        center = Vec3(
            (boundingBoxMin.x + boundingBoxMax.x) * 0.5f,
            (boundingBoxMin.y + boundingBoxMax.y) * 0.5f,
            (boundingBoxMin.z + boundingBoxMax.z) * 0.5f
        );
    }
};

/**
 * OBJLoader Class - Wavefront OBJ File Parser
 * 
 * Efficiently loads and parses OBJ files into renderable mesh data.
 * Handles large files with streaming parsing and memory optimization.
 */
class OBJLoader {
public:
    /**
     * Load OBJ file and return mesh data (simplified interface)
     * 
     * @param filepath Path to the OBJ file
     * @param scale Optional uniform scale factor (default: 1.0)
     * @return OBJMeshData containing parsed geometry, or empty data on failure
     * @note This is a convenience wrapper around loadOBJWithProgress with default logging
     */
    static OBJMeshData loadOBJ(const std::string& filepath, float scale = 1.0f);
    
    /**
     * Load OBJ file with progress reporting for large files
     * 
     * @param filepath Path to the OBJ file
     * @param scale Optional uniform scale factor
     * @param logInfo Optional callback for info messages (default: stdout)
     * @param logWarning Optional callback for warning messages (default: stdout)
     * @param logError Optional callback for error messages (default: stderr)
     * @return OBJMeshData containing parsed geometry
     */
    static OBJMeshData loadOBJWithProgress(
        const std::string& filepath, 
        float scale = 1.0f,
        LogInfoCallback logInfo = nullptr,
        LogWarningCallback logWarning = nullptr,
        LogErrorCallback logError = nullptr
    );

private:
    // Helper structures for parsing
    struct TempVertex {
        Vec3 position;
        Vec3 normal;
        Vec2 texCoord;
        bool hasNormal = false;
        bool hasTexCoord = false;
    };
    
    struct Face {
        unsigned int v1, v2, v3;    // Vertex indices
        unsigned int t1, t2, t3;    // Texture coordinate indices
        unsigned int n1, n2, n3;    // Normal indices
        bool hasTexCoords = false;
        bool hasNormals = false;
    };
    
    // Parsing helper methods
    static Vec3 parseVertex(const std::string& line);
    static Vec3 parseNormal(const std::string& line);
    static Vec2 parseTexCoord(const std::string& line);
    static std::vector<Face> parseFace(const std::string& line);
    static void generateNormals(std::vector<TempVertex>& vertices, const std::vector<Face>& faces, LogWarningCallback logWarning = nullptr);
    static void buildFinalMesh(const std::vector<TempVertex>& vertices, 
                              const std::vector<Face>& faces, 
                              OBJMeshData& meshData, 
                              float scale);
};

} // namespace Engine