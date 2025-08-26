/**
 * Water.cpp - Implementation of Water Surface GameObject
 */

#include "Water.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/WaterRenderer.h"
#include <iostream>

namespace Engine {

Water::Water(const std::string& name, float height)
    : GameObject(name), waterHeight(height), waveSpeed(0.03f), 
      distortionScale(0.01f), shineDamper(20.0f), reflectivity(0.6f),
      waterRenderer(nullptr) {
    
    // Set water color (blue tint)
    setColor(Vec3(0.0f, 0.3f, 0.5f));
}

Water::~Water() {
    // Cleanup is handled by base class
}

bool Water::initialize() {
    if (isInitialized) return true;
    
    std::cout << "Initializing Water '" << name << "'..." << std::endl;
    
    // Setup the water mesh
    setupMesh();
    
    // Get water renderer from factory
    waterRenderer = dynamic_cast<WaterRenderer*>(RendererFactory::getInstance().getRenderer(RendererType::Water));
    if (waterRenderer) {
        // Configure water renderer parameters
        waterRenderer->setWaveSpeed(waveSpeed);
        waterRenderer->setDistortionScale(distortionScale);
        waterRenderer->setShineDamper(shineDamper);
        waterRenderer->setReflectivity(reflectivity);
        std::cout << "Water renderer configured for '" << name << "'" << std::endl;
    } else {
        std::cerr << "Warning: No water renderer available for '" << name << "'" << std::endl;
    }
    
    isInitialized = true;
    std::cout << "Water '" << name << "' initialized successfully" << std::endl;
    return true;
}

void Water::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update base GameObject
    GameObject::update(deltaTime);
    
    // Update water-specific parameters if needed
    if (waterRenderer) {
        waterRenderer->setWaveSpeed(waveSpeed);
        waterRenderer->setDistortionScale(distortionScale);
        waterRenderer->setShineDamper(shineDamper);
        waterRenderer->setReflectivity(reflectivity);
    }
}

void Water::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !mesh) return;

    // Use water renderer if available, otherwise fall back to default
    if (waterRenderer) {
        const Mat4 modelMatrix = getWaterModelMatrix(); // Use water-specific model matrix
        waterRenderer->renderWater(*mesh, modelMatrix, camera, waterHeight);
    } else {
        // Fall back to default rendering
        GameObject::render(renderer, camera);
    }
}

void Water::setupMesh() {
    // Create a massive plane for water surface (covers the entire world)
    const int width = 1000;  // 1000x1000 world units - massive
    const int length = 1000;
    const int resolution = 64;  // 64x64 vertices for smooth water
    
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> texCoords;
    std::vector<unsigned int> indices;
    
    // Generate vertices
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            // Position (centered around origin, Y is always 0 for the mesh)
            float xPos = (float(x) / resolution - 0.5f) * width;
            float zPos = (float(z) / resolution - 0.5f) * length;
            float yPos = 0.0f; // Mesh is created at Y=0, waterHeight is applied in shader
            
            vertices.push_back(Vec3(xPos, yPos, zPos));
            
            // Normal (pointing up for water surface)
            normals.push_back(Vec3(0.0f, 1.0f, 0.0f));
            
            // Texture coordinates
            float u = float(x) / resolution;
            float v = float(z) / resolution;
            texCoords.push_back(Vec2(u, v));
        }
    }
    
    // Generate indices
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            int topLeft = z * (resolution + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (resolution + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    // Create mesh with normals and texture coordinates
    mesh = std::make_unique<Mesh>();
    
    // Convert data to the format expected by Mesh class
    std::vector<float> vertexData;
    for (size_t i = 0; i < vertices.size(); i++) {
        // Position (3 floats)
        vertexData.push_back(vertices[i].x);
        vertexData.push_back(vertices[i].y);
        vertexData.push_back(vertices[i].z);
        
        // Normal (3 floats)
        vertexData.push_back(normals[i].x);
        vertexData.push_back(normals[i].y);
        vertexData.push_back(normals[i].z);
        
        // Texture coordinates (2 floats)
        vertexData.push_back(texCoords[i].x);
        vertexData.push_back(texCoords[i].y);
    }
    
    if (!mesh->createMeshWithNormalsAndTexCoords(vertexData, indices)) {
        std::cerr << "Failed to create water mesh" << std::endl;
        return;
    }
    
    std::cout << "Water mesh created with " << vertices.size() << " vertices and " 
              << indices.size() / 3 << " triangles" << std::endl;
}

Mat4 Water::getWaterModelMatrix() const {
    // Create a completely static model matrix for water
    // Water should be fixed in world space, not moving with camera
    Mat4 model = Mat4(); // Identity matrix
    
    // No translation - water is fixed at world origin
    // No rotation - water is always horizontal
    // No scale - use default size
    
    // The water height is applied in the shader, not in the model matrix
    return model;
}

} // namespace Engine
