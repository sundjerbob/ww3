/**
 * OBJLoader.cpp - Implementation of Wavefront OBJ File Loading System
 * 
 * Efficient implementation for loading large OBJ files with proper error handling
 * and memory management.
 */

#include "OBJLoader.h"
#include "../Rendering/MaterialLoader.h"
#include <functional>

namespace Engine {

OBJMeshData OBJLoader::loadOBJ(const std::string& filepath, float scale) {
    return loadOBJWithProgress(filepath, scale);
}

OBJMeshData OBJLoader::loadOBJWithProgress(
    const std::string& filepath, 
    float scale,
    LogInfoCallback logInfo,
    LogWarningCallback logWarning,
    LogErrorCallback logError
) {
    // Set default logging callbacks if none provided
    if (!logInfo) logInfo = [](const std::string& msg) { std::cout << msg << std::endl; };
    if (!logWarning) logWarning = [](const std::string& msg) { std::cout << "Warning: " << msg << std::endl; };
    if (!logError) logError = [](const std::string& msg) { std::cerr << "Error: " << msg << std::endl; };
    
    logInfo("Loading OBJ file: " + filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        logError("Could not open OBJ file: " + filepath);
        return OBJMeshData{};
    }
    
    // Get file size for progress tracking
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<TempVertex> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> texCoords;
    std::vector<Face> faces;
    std::vector<std::string> faceMaterialNames; // Material name for each face
    std::string currentMaterial = ""; // Current material being used
    
    // Reserve space for efficiency (estimate based on typical OBJ files)
    try {
        vertices.reserve(100000);
        normals.reserve(100000);
        texCoords.reserve(100000);
        faces.reserve(200000);
    }
    catch (const std::bad_alloc& e) {
        logError("Memory allocation failed during initialization: " + std::string(e.what()));
        return OBJMeshData{};
    }
    
    std::string line;
    std::streampos bytesRead = 0;
    int lineCount = 0;
    
    // Parse the OBJ file line by line
    while (std::getline(file, line)) {
        lineCount++;
        bytesRead += line.length() + 1; // +1 for newline
        
        // Progress reporting for large files
        /*if (lineCount % 10000 == 0) {
            float progress = static_cast<float>(bytesRead) / static_cast<float>(fileSize);
            std::cout << "Loading progress: " << static_cast<int>(progress * 80) << "%" << std::endl;
        }
        */
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse based on line type
        if (line.substr(0, 2) == "v ") {
            // Vertex position
            try {
                TempVertex vertex;
                vertex.position = parseVertex(line);
                vertices.push_back(vertex);
            }
            catch (const std::exception&) {
                logWarning("Failed to parse vertex on line " + std::to_string(lineCount) + ": " + line);
                continue;
            }
        }
        else if (line.substr(0, 3) == "vn ") {
            // Vertex normal
            try {
                normals.push_back(parseNormal(line));
            }
            catch (const std::exception&) {
                logWarning("Failed to parse normal on line " + std::to_string(lineCount) + ": " + line);
                continue;
            }
        }
        else if (line.substr(0, 3) == "vt ") {
            // Texture coordinate
            try {
                texCoords.push_back(parseTexCoord(line));
            }
            catch (const std::exception&) {
                logWarning("Failed to parse texture coordinate on line " + std::to_string(lineCount) + ": " + line);
                continue;
            }
        }
        else if (line.substr(0, 2) == "f ") {
            // Face (may be N-gon, will be triangulated)
            try {
                std::vector<Face> triangulatedFaces = parseFace(line);
                for (const auto& face : triangulatedFaces) {
                    if (face.v1 > 0) { // Valid face
                        faces.push_back(face);
                        // Associate this face with the current material
                        faceMaterialNames.push_back(currentMaterial);
                    }
                }
            }
            catch (const std::exception&) {
                logWarning("Failed to parse face on line " + std::to_string(lineCount) + ": " + line);
                continue;
            }
        }
        else if (line.substr(0, 7) == "mtllib ") {
            // Material library file
            std::string mtlFilename = line.substr(7);
            // Remove any trailing whitespace/newlines
            while (!mtlFilename.empty() && (mtlFilename.back() == ' ' || mtlFilename.back() == '\t' || mtlFilename.back() == '\r' || mtlFilename.back() == '\n')) {
                mtlFilename.pop_back();
            }
            logInfo("Found material library: " + mtlFilename);
            // MTL loading will be done after parsing the OBJ file
        }
        else if (line.substr(0, 7) == "usemtl ") {
            // Use material
            currentMaterial = line.substr(7);
            // Remove any trailing whitespace/newlines
            while (!currentMaterial.empty() && (currentMaterial.back() == ' ' || currentMaterial.back() == '\t' || currentMaterial.back() == '\r' || currentMaterial.back() == '\n')) {
                currentMaterial.pop_back();
            }
            logInfo("Using material: " + currentMaterial);
        }
    }
    
    file.close();
    
    logInfo("Parsed OBJ file:");
    logInfo("  Vertices: " + std::to_string(vertices.size()));
    logInfo("  Normals: " + std::to_string(normals.size()));
    logInfo("  Texture Coords: " + std::to_string(texCoords.size()));
    logInfo("  Faces: " + std::to_string(faces.size()));
    
    // Assign normals and texture coordinates to vertices
    logInfo("Processing normals and texture coordinates: 85%");
    
    // Assign texture coordinates if available
    if (texCoords.size() > 0) {
        for (auto& face : faces) {
            if (face.hasTexCoords) {
                if (face.t1 <= texCoords.size()) {
                    vertices[face.v1 - 1].texCoord = texCoords[face.t1 - 1];
                    vertices[face.v1 - 1].hasTexCoord = true;
                }
                if (face.t2 <= texCoords.size()) {
                    vertices[face.v2 - 1].texCoord = texCoords[face.t2 - 1];
                    vertices[face.v2 - 1].hasTexCoord = true;
                }
                if (face.t3 <= texCoords.size()) {
                    vertices[face.v3 - 1].texCoord = texCoords[face.t3 - 1];
                    vertices[face.v3 - 1].hasTexCoord = true;
                }
            }
        }
    }
    
    if (normals.size() > 0) {
        // Use provided normals
        for (auto& face : faces) {
            if (face.hasNormals) {
                if (face.n1 <= normals.size()) {
                    vertices[face.v1 - 1].normal = normals[face.n1 - 1];
                    vertices[face.v1 - 1].hasNormal = true;
                }
                if (face.n2 <= normals.size()) {
                    vertices[face.v2 - 1].normal = normals[face.n2 - 1];
                    vertices[face.v2 - 1].hasNormal = true;
                }
                if (face.n3 <= normals.size()) {
                    vertices[face.v3 - 1].normal = normals[face.n3 - 1];
                    vertices[face.v3 - 1].hasNormal = true;
                }
            }
        }
    } else {
        // Generate normals
        logInfo("Generating normals...");
        generateNormals(vertices, faces, logWarning);
    }
    
    logInfo("Building final mesh: 95%");
    
    // Validate parsed data before building mesh
    if (vertices.empty()) {
        logError("No valid vertices found in OBJ file");
        return OBJMeshData{};
    }
    
    if (faces.empty()) {
        logError("No valid faces found in OBJ file");
        return OBJMeshData{};
    }
    
    // Build final mesh data
    OBJMeshData meshData;
    try {
        buildFinalMesh(vertices, faces, meshData, scale);
        meshData.calculateBounds();
        
        // Copy face material associations
        meshData.faceMaterials = faceMaterialNames;
        logInfo("Associated " + std::to_string(meshData.faceMaterials.size()) + " faces with materials");
    }
    catch (const std::exception& e) {
        logError("Failed to build final mesh: " + std::string(e.what()));
        return OBJMeshData{};
    }
    
    // Load materials from MTL file
    logInfo("Loading materials from MTL file...");
    std::string mtlFilePath = MaterialLoader::getMTLPathFromOBJ(filepath);
    if (MaterialLoader::isValidMTLFile(mtlFilePath)) {
        meshData.materials = MaterialLoader::loadMTL(mtlFilePath);
        logInfo("Loaded " + std::to_string(meshData.materials.getMaterialCount()) + " materials");
    } else {
        logWarning("MTL file not found or invalid: " + mtlFilePath);
        // Create a default material
        Material defaultMaterial("default");
        defaultMaterial.diffuse = Vec3(0.8f, 0.8f, 0.8f); // Light gray
        meshData.materials.addMaterial(defaultMaterial);
    }
    
    logInfo("Loading complete: 100%");
    
    logInfo("OBJ loading complete!");
    logInfo("  Final vertices: " + std::to_string(meshData.vertices.size() / 8));
    logInfo("  Triangles: " + std::to_string(meshData.indices.size() / 3));
    logInfo("  Bounding box: (" + std::to_string(meshData.boundingBoxMin.x) + ", " + 
            std::to_string(meshData.boundingBoxMin.y) + ", " + std::to_string(meshData.boundingBoxMin.z) + 
            ") to (" + std::to_string(meshData.boundingBoxMax.x) + ", " + 
            std::to_string(meshData.boundingBoxMax.y) + ", " + std::to_string(meshData.boundingBoxMax.z) + ")");
    
    return meshData;
}

Vec3 OBJLoader::parseVertex(const std::string& line) {
    std::istringstream iss(line.substr(2)); // Skip "v "
    float x, y, z;
    
    if (!(iss >> x >> y >> z)) {
        throw std::runtime_error("Invalid vertex format: " + line);
    }
    
    // Check for valid floating point values
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        throw std::runtime_error("Invalid vertex coordinates (non-finite values): " + line);
    }
    
    return Vec3(x, y, z);
}

Vec3 OBJLoader::parseNormal(const std::string& line) {
    std::istringstream iss(line.substr(3)); // Skip "vn "
    float x, y, z;
    
    if (!(iss >> x >> y >> z)) {
        throw std::runtime_error("Invalid normal format: " + line);
    }
    
    // Check for valid floating point values
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        throw std::runtime_error("Invalid normal coordinates (non-finite values): " + line);
    }
    
    return Vec3(x, y, z);
}

Vec2 OBJLoader::parseTexCoord(const std::string& line) {
    std::istringstream iss(line.substr(3)); // Skip "vt "
    float u, v;
    
    if (!(iss >> u >> v)) {
        throw std::runtime_error("Invalid texture coordinate format: " + line);
    }
    
    // Check for valid floating point values
    if (!std::isfinite(u) || !std::isfinite(v)) {
        throw std::runtime_error("Invalid texture coordinates (non-finite values): " + line);
    }
    
    return Vec2(u, v);
}

std::vector<OBJLoader::Face> OBJLoader::parseFace(const std::string& line) {
    std::vector<Face> triangulatedFaces;
    std::istringstream iss(line.substr(2)); // Skip "f "
    
    // Parse vertex indices (format: v/vt/vn or v//vn or v/vt or v)
    struct VertexIndices {
        unsigned int vertex = 0;
        unsigned int texCoord = 0;
        unsigned int normal = 0;
    };
    
    auto parseVertexIndex = [](const std::string& vertexStr) -> VertexIndices {
        VertexIndices indices;
        size_t slash1 = vertexStr.find('/');
        
        if (slash1 == std::string::npos) {
            // Format: v
            indices.vertex = static_cast<unsigned int>(std::stoi(vertexStr));
            return indices;
        }
        
        // Get vertex index
        indices.vertex = static_cast<unsigned int>(std::stoi(vertexStr.substr(0, slash1)));
        
        size_t slash2 = vertexStr.find('/', slash1 + 1);
        if (slash2 == std::string::npos) {
            // Format: v/vt (no normal)
            if (slash1 + 1 < vertexStr.length()) {
                indices.texCoord = static_cast<unsigned int>(std::stoi(vertexStr.substr(slash1 + 1)));
            }
            return indices;
        }
        
        // Format: v/vt/vn or v//vn
        if (slash2 > slash1 + 1) {
            // Has texture coordinate
            indices.texCoord = static_cast<unsigned int>(std::stoi(vertexStr.substr(slash1 + 1, slash2 - slash1 - 1)));
        }
        
        if (slash2 + 1 < vertexStr.length()) {
            // Has normal
            indices.normal = static_cast<unsigned int>(std::stoi(vertexStr.substr(slash2 + 1)));
        }
        
        return indices;
    };
    
    try {
        // Parse all vertices in the face
        std::vector<VertexIndices> faceVertices;
        std::string vertexStr;
        while (iss >> vertexStr) {
            faceVertices.push_back(parseVertexIndex(vertexStr));
        }
        
        // Need at least 3 vertices for a valid face
        if (faceVertices.size() < 3) {
            Face invalidFace = {};
            invalidFace.v1 = 0; // Mark as invalid
            triangulatedFaces.push_back(invalidFace);
            return triangulatedFaces;
        }
        
        // Triangulate using fan triangulation (all triangles share first vertex)
        for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
            Face triangle = {};
            
            // Triangle vertices: 0, i, i+1
            triangle.v1 = faceVertices[0].vertex;
            triangle.v2 = faceVertices[i].vertex;
            triangle.v3 = faceVertices[i + 1].vertex;
            
            triangle.t1 = faceVertices[0].texCoord;
            triangle.t2 = faceVertices[i].texCoord;
            triangle.t3 = faceVertices[i + 1].texCoord;
            
            triangle.n1 = faceVertices[0].normal;
            triangle.n2 = faceVertices[i].normal;
            triangle.n3 = faceVertices[i + 1].normal;
            
            triangle.hasTexCoords = (triangle.t1 > 0 && triangle.t2 > 0 && triangle.t3 > 0);
            triangle.hasNormals = (triangle.n1 > 0 && triangle.n2 > 0 && triangle.n3 > 0);
            
            triangulatedFaces.push_back(triangle);
        }
    }
    catch (const std::exception&) {
        // Note: Cannot use logError here as it's not available in this static context
        // This error will be handled by the caller
        Face invalidFace = {};
        invalidFace.v1 = 0; // Mark as invalid
        triangulatedFaces.push_back(invalidFace);
    }
    
    return triangulatedFaces;
}

void OBJLoader::generateNormals(std::vector<TempVertex>& vertices, const std::vector<Face>& faces, LogWarningCallback logWarning) {
    // Set default logging callback if none provided
    if (!logWarning) logWarning = [](const std::string& msg) { std::cout << "Warning: " << msg << std::endl; };
    // Initialize all normals to zero
    for (auto& vertex : vertices) {
        vertex.normal = Vec3(0, 0, 0);
        vertex.hasNormal = false;
    }
    
    int degenerateTriangles = 0;
    
    // Calculate face normals and accumulate to vertices
    for (const auto& face : faces) {
        if (face.v1 == 0 || face.v1 > vertices.size() || 
            face.v2 == 0 || face.v2 > vertices.size() || 
            face.v3 == 0 || face.v3 > vertices.size()) {
            continue; // Invalid face
        }
        
        Vec3& p1 = vertices[face.v1 - 1].position;
        Vec3& p2 = vertices[face.v2 - 1].position;
        Vec3& p3 = vertices[face.v3 - 1].position;
        
        // Calculate face normal using cross product
        Vec3 edge1 = Vec3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
        Vec3 edge2 = Vec3(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
        
        // Check for degenerate edges (zero length)
        float edge1Length = std::sqrt(edge1.x * edge1.x + edge1.y * edge1.y + edge1.z * edge1.z);
        float edge2Length = std::sqrt(edge2.x * edge2.x + edge2.y * edge2.y + edge2.z * edge2.z);
        
        if (edge1Length < 1e-7f || edge2Length < 1e-7f) {
            degenerateTriangles++;
            continue; // Skip degenerate triangle with zero-length edges
        }
        
        Vec3 faceNormal = Engine::cross(edge1, edge2);
        float normalLength = std::sqrt(faceNormal.x * faceNormal.x + faceNormal.y * faceNormal.y + faceNormal.z * faceNormal.z);
        
        // Check for degenerate triangle (collinear vertices resulting in zero-length normal)
        if (normalLength < 1e-7f) {
            degenerateTriangles++;
            continue; // Skip degenerate triangle
        }
        
        // Normalize the face normal
        faceNormal.x /= normalLength;
        faceNormal.y /= normalLength;
        faceNormal.z /= normalLength;
        
        // Accumulate to vertex normals
        vertices[face.v1 - 1].normal = Vec3(
            vertices[face.v1 - 1].normal.x + faceNormal.x,
            vertices[face.v1 - 1].normal.y + faceNormal.y,
            vertices[face.v1 - 1].normal.z + faceNormal.z
        );
        vertices[face.v2 - 1].normal = Vec3(
            vertices[face.v2 - 1].normal.x + faceNormal.x,
            vertices[face.v2 - 1].normal.y + faceNormal.y,
            vertices[face.v2 - 1].normal.z + faceNormal.z
        );
        vertices[face.v3 - 1].normal = Vec3(
            vertices[face.v3 - 1].normal.x + faceNormal.x,
            vertices[face.v3 - 1].normal.y + faceNormal.y,
            vertices[face.v3 - 1].normal.z + faceNormal.z
        );
    }
    
    // Normalize accumulated normals
    int verticesWithoutNormals = 0;
    for (auto& vertex : vertices) {
        float normalLength = std::sqrt(vertex.normal.x * vertex.normal.x + vertex.normal.y * vertex.normal.y + vertex.normal.z * vertex.normal.z);
        
        if (normalLength < 1e-7f) {
            // Vertex has no valid face normals contributing to it, use default up vector
            vertex.normal = Vec3(0, 1, 0);
            verticesWithoutNormals++;
        } else {
            // Normalize the accumulated normal
            vertex.normal.x /= normalLength;
            vertex.normal.y /= normalLength;
            vertex.normal.z /= normalLength;
        }
        vertex.hasNormal = true;
    }
    
    if (degenerateTriangles > 0) {
        logWarning("Skipped " + std::to_string(degenerateTriangles) + " degenerate triangles during normal generation");
    }
    if (verticesWithoutNormals > 0) {
        logWarning(std::to_string(verticesWithoutNormals) + " vertices had no valid face normals, using default normals");
    }
}

void OBJLoader::buildFinalMesh(const std::vector<TempVertex>& vertices, 
                              const std::vector<Face>& faces, 
                              OBJMeshData& meshData, 
                              float scale) {
    // Reserve space for efficiency
    meshData.vertices.reserve(faces.size() * 3 * 8); // 3 vertices per face, 8 floats per vertex
    meshData.indices.reserve(faces.size() * 3);
    
    unsigned int currentIndex = 0;
    
    for (const auto& face : faces) {
        if (face.v1 == 0 || face.v1 > vertices.size() || 
            face.v2 == 0 || face.v2 > vertices.size() || 
            face.v3 == 0 || face.v3 > vertices.size()) {
            continue; // Skip invalid faces
        }
        
        // Add vertices (position + normal + texture coordinate interleaved)
        const TempVertex& v1 = vertices[face.v1 - 1];
        const TempVertex& v2 = vertices[face.v2 - 1];
        const TempVertex& v3 = vertices[face.v3 - 1];
        
        // Vertex 1
        meshData.vertices.push_back(v1.position.x * scale);
        meshData.vertices.push_back(v1.position.y * scale);
        meshData.vertices.push_back(v1.position.z * scale);
        meshData.vertices.push_back(v1.normal.x);
        meshData.vertices.push_back(v1.normal.y);
        meshData.vertices.push_back(v1.normal.z);
        meshData.vertices.push_back(v1.hasTexCoord ? v1.texCoord.x : 0.0f);
        meshData.vertices.push_back(v1.hasTexCoord ? v1.texCoord.y : 0.0f);
        
        // Vertex 2
        meshData.vertices.push_back(v2.position.x * scale);
        meshData.vertices.push_back(v2.position.y * scale);
        meshData.vertices.push_back(v2.position.z * scale);
        meshData.vertices.push_back(v2.normal.x);
        meshData.vertices.push_back(v2.normal.y);
        meshData.vertices.push_back(v2.normal.z);
        meshData.vertices.push_back(v2.hasTexCoord ? v2.texCoord.x : 0.0f);
        meshData.vertices.push_back(v2.hasTexCoord ? v2.texCoord.y : 0.0f);
        
        // Vertex 3
        meshData.vertices.push_back(v3.position.x * scale);
        meshData.vertices.push_back(v3.position.y * scale);
        meshData.vertices.push_back(v3.position.z * scale);
        meshData.vertices.push_back(v3.normal.x);
        meshData.vertices.push_back(v3.normal.y);
        meshData.vertices.push_back(v3.normal.z);
        meshData.vertices.push_back(v3.hasTexCoord ? v3.texCoord.x : 0.0f);
        meshData.vertices.push_back(v3.hasTexCoord ? v3.texCoord.y : 0.0f);
        
        // Add indices
        meshData.indices.push_back(currentIndex);
        meshData.indices.push_back(currentIndex + 1);
        meshData.indices.push_back(currentIndex + 2);
        
        currentIndex += 3;
    }
    
    meshData.vertexCount = currentIndex;
    meshData.triangleCount = static_cast<unsigned int>(meshData.indices.size() / 3);
}

} // namespace Engine