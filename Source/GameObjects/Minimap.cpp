/**
 * Minimap.cpp - Implementation of 2D Minimap GameObject
 */

#include "Minimap.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Math/Camera.h"
#include "../Engine/Math/Math.h"
#include <iostream>
#include <algorithm>

namespace Engine {

Minimap::Minimap(const std::string& name, float size)
    : GameObject(name), 
      minimapWidth(256), 
      minimapHeight(256), 
      minimapSize(size),
      framebuffer(0),
      textureColorBuffer(0),
      renderbuffer(0),
      scene(nullptr),
      isFramebufferInitialized(false),
      isTextureValid(false) {
    
    // Set minimap to render in 2D screen space
    setPosition(Vec3(-0.8f, 0.8f, 0.0f)); // Top-left corner in NDC
    setScale(Vec3(minimapSize, minimapSize, 1.0f));
}

Minimap::~Minimap() {
    cleanup();
}

bool Minimap::initialize() {
    std::cout << "Initializing Minimap..." << std::endl;
    
    if (!GameObject::initialize()) {
        std::cerr << "Failed to initialize Minimap base class" << std::endl;
        return false;
    }
    
    // Initialize framebuffer for render-to-texture
    if (!initializeFramebuffer()) {
        std::cerr << "Failed to initialize minimap framebuffer" << std::endl;
        return false;
    }
    
    // Initialize shaders
    if (!initializeShaders()) {
        std::cerr << "Failed to initialize minimap shaders" << std::endl;
        return false;
    }
    
    // Setup orthographic camera
    orthographicCamera.setPosition(Vec3(0.0f, 20.0f, 0.0f)); // High up for bird's-eye view
    orthographicCamera.setRotation(Vec3(-90.0f, 0.0f, 0.0f)); // Looking straight down
    
    // Create a simple quad mesh for rendering the minimap texture
    setupMesh();
    
    std::cout << "Minimap initialized successfully" << std::endl;
    return true;
}

void Minimap::update(float deltaTime) {
    GameObject::update(deltaTime);
    
    // Update orthographic camera to follow player (if we have a player position)
    // For now, we'll keep it static
    // updateOrthographicCamera(playerPosition);
}

void Minimap::render(const Renderer& renderer, const Camera& camera) {
    if (!isValid()) {
        return;
    }
    
    // First, render the scene to our texture using orthographic projection
    renderSceneToTexture();
    
    // Then render the minimap texture to screen
    renderMinimapTexture();
}

void Minimap::cleanup() {
    cleanupFramebuffer();
    GameObject::cleanup();
}

bool Minimap::initializeFramebuffer() {
    // Create framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // Create texture attachment
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, minimapWidth, minimapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
    
    // Create renderbuffer for depth/stencil
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, minimapWidth, minimapHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        return false;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    isFramebufferInitialized = true;
    return true;
}

bool Minimap::initializeShaders() {
    // Create shader for rendering minimap texture to screen
    minimapShader = std::make_unique<Shader>();
    if (!minimapShader->loadFromFiles("Resources/Shaders/minimap_vertex.glsl", "Resources/Shaders/minimap_fragment.glsl")) {
        std::cerr << "Failed to load minimap shader" << std::endl;
        return false;
    }
    
    // Create shader for orthographic rendering (we'll use the basic shader for now)
    orthographicShader = std::make_unique<Shader>();
    if (!orthographicShader->loadFromFiles("Resources/Shaders/vertex.glsl", "Resources/Shaders/fragment.glsl")) {
        std::cerr << "Failed to load orthographic shader" << std::endl;
        return false;
    }
    
    return true;
}

void Minimap::setupMesh() {
    // Create a small quad for rendering the minimap texture in top-right corner
    // Position it in the top-right corner with proper size and indents
    // TODO: minimapSize (0.25f) makes minimap too small - increase for better visibility
    float right = 1.0f - minimapSize;  // Start from right edge with indent
    float left = 1.0f - minimapSize * 2.0f;  // Extend left by minimap size
    float top = 1.0f - minimapSize;  // Top edge with indent
    float bottom = 1.0f - minimapSize * 2.0f;  // Extend down by minimap size
    
    std::vector<float> vertices = {
        // Position (x, y)    // Texture coordinates (u, v)
        left,  top,    0.0f,   0.0f, 1.0f,  // Top-left
        right, top,    0.0f,   1.0f, 1.0f,  // Top-right
        right, bottom, 0.0f,   1.0f, 0.0f,  // Bottom-right
        left,  bottom, 0.0f,   0.0f, 0.0f   // Bottom-left
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMeshWithTexCoords(vertices, indices)) {
        std::cerr << "Failed to create minimap quad mesh" << std::endl;
    }
    
    std::cout << "Minimap quad created: left=" << left << ", right=" << right 
              << ", top=" << top << ", bottom=" << bottom << std::endl;
}

bool Minimap::aggregateSceneMeshes() {
    if (!scene) {
        std::cerr << "No scene reference for minimap mesh aggregation" << std::endl;
        return false;
    }
    
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;
    unsigned int vertexOffset = 0;
    
    // Get all game objects from the scene
    const auto& gameObjects = scene->getGameObjects();
    
    static int frameCount = 0;
    frameCount++;
    
    // Only print every 60 frames to avoid spam (once per second at 60fps)
    if (frameCount % 60 == 0) {
        std::cout << "Frame " << frameCount << ": Aggregating meshes from " << gameObjects.size() << " scene objects for minimap..." << std::endl;
    }
    
    for (const auto& gameObject : gameObjects) {
        // Skip crosshair (it's a 2D UI element)
        if (gameObject->getName() == "Crosshair") {
            continue;
        }
        
        // Get the mesh from the game object
        const Mesh* objectMesh = gameObject->getMesh();
        if (!objectMesh || !objectMesh->isValid()) {
            std::cout << "Skipping " << gameObject->getName() << " - no valid mesh" << std::endl;
            continue;
        }
        
        std::cout << "Adding " << gameObject->getName() << " to minimap mesh" << std::endl;
        
        // Get the object's transform
        Vec3 position = gameObject->getPosition();
        Vec3 scale = gameObject->getScale();
        Vec3 rotation = gameObject->getRotation();
        
        // Extract actual mesh data from the object
        const std::vector<float>& objectVertices = objectMesh->getVertices();
        const std::vector<unsigned int>& objectIndices = objectMesh->getIndices();
        
        // Only print every 60 frames to avoid spam
        if (frameCount % 100 == 0) {
            std::cout << "  - " << gameObject->getName() << ": pos(" << position.x << "," << position.y << "," << position.z 
                      << ") rot(" << rotation.x << "," << rotation.y << "," << rotation.z 
                      << ") scale(" << scale.x << "," << scale.y << "," << scale.z << ")" << std::endl;
            std::cout << "  - Original mesh: " << objectVertices.size() / 3 
                      << " vertices, " << objectIndices.size() << " indices" << std::endl;
        }
        
        // ============================================================================
        // CPU-BASED MESH AGGREGATION: Combining all scene objects into one minimap mesh
        // ============================================================================
        // 
        // BEFORE UPDATE (Rotation Missing):
        // - Only applied position and scale to vertices
        // - Rotation was completely ignored
        // - Result: Static cubes that never rotated in minimap
        // 
        // AFTER UPDATE (Rotation Added):
        // - Now applies position + scale + ROTATION to vertices
        // - Full 3D rotation matrix calculation on CPU
        // - Result: Rotating cubes visible in minimap
        // 
        // PERFORMANCE NOTE: This CPU approach is inefficient because:
        // - Heavy math calculations for every vertex every frame
        // - No parallelization (CPU is sequential)
        // - Future optimization: Move this to GPU via shader uniforms
        // ============================================================================
        
        // Apply transform to actual mesh vertices
        for (size_t i = 0; i < objectVertices.size(); i += 3) {
            Vec3 vertex(objectVertices[i], objectVertices[i+1], objectVertices[i+2]);
            
            // Apply scale
            vertex.x *= scale.x;
            vertex.y *= scale.y;
            vertex.z *= scale.z;
            
            // Apply rotation (convert degrees to radians and apply rotation matrix)
            float rotX = rotation.x * 3.14159f / 180.0f;
            float rotY = rotation.y * 3.14159f / 180.0f;
            float rotZ = rotation.z * 3.14159f / 180.0f;
            
            // Apply Y-axis rotation (most common for cubes)
            float tempX = vertex.x;
            float tempZ = vertex.z;
            vertex.x = tempX * cos(rotY) - tempZ * sin(rotY);
            vertex.z = tempX * sin(rotY) + tempZ * cos(rotY);
            
            // Apply X-axis rotation
            tempX = vertex.x;
            float tempY = vertex.y;
            vertex.x = tempX * cos(rotX) + tempY * sin(rotX);
            vertex.y = -tempX * sin(rotX) + tempY * cos(rotX);
            
            // Apply Z-axis rotation
            tempX = vertex.x;
            tempY = vertex.y;
            vertex.x = tempX * cos(rotZ) - tempY * sin(rotZ);
            vertex.y = tempX * sin(rotZ) + tempY * cos(rotZ);
            
            // Apply position
            vertex = vertex + position;
            
            // Add to all vertices
            allVertices.push_back(vertex.x);
            allVertices.push_back(vertex.y);
            allVertices.push_back(vertex.z);
        }
        
        // Add indices with offset
        for (unsigned int index : objectIndices) {
            allIndices.push_back(index + vertexOffset);
        }
        
        vertexOffset += objectVertices.size() / 3; // 3 floats per vertex
    }
    
    // Create the aggregated mesh
    aggregatedMesh = std::make_unique<Mesh>();
    if (!aggregatedMesh->createMesh(allVertices, allIndices)) {
        std::cerr << "Failed to create aggregated mesh for minimap" << std::endl;
        return false;
    }
    
    // Only print every 60 frames to avoid spam
    if (frameCount % 60 == 0) {
        std::cout << "Minimap mesh aggregation complete: " << allVertices.size() / 3 
                  << " vertices, " << allIndices.size() << " indices" << std::endl;
    }
    
    return true;
}

void Minimap::renderSceneToTexture() {
    // Always update mesh aggregation to capture dynamic changes (rotation, position, etc.)
    aggregateSceneMeshes();
    
    if (!aggregatedMesh || !isFramebufferInitialized) return;
    
    // Store current OpenGL state
    GLint currentFramebuffer;
    GLint currentViewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, minimapWidth, minimapHeight);
    
    // Clear framebuffer
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f); calling this fucked up the clearclor in game, (sky color)
    glClear(/*GL_COLOR_BUFFER_BIT | do not need to reset COLOR_BUFFER_BIT*/ GL_DEPTH_BUFFER_BIT);
    
    // Use orthographic shader
    if (orthographicShader) {
        orthographicShader->use();
        
        // Set orthographic projection matrix
        Mat4 orthoMatrix = Engine::orthographic(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 100.0f);
        orthographicShader->setMat4("projection", orthoMatrix);
        orthographicShader->setMat4("view", orthographicCamera.getViewMatrix());
        
        // Render aggregated mesh
        if (aggregatedMesh) {
            Mat4 modelMatrix = Mat4(); // Identity matrix
            orthographicShader->setMat4("model", modelMatrix);
            // Color is handled automatically by the fragment shader based on Y position
            aggregatedMesh->render();
        }
    }
    
    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
    glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);
    
    isTextureValid = true;
}

void Minimap::renderMinimapTexture() {
    if (!isTextureValid || !minimapShader || !mesh) return;
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Disable depth testing for 2D rendering
    glDisable(GL_DEPTH_TEST);
    
    // Use minimap shader
    minimapShader->use();
    
    // Bind the texture we rendered to
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    minimapShader->setInt("minimapTexture", 0);
    
    // Render the quad
    mesh->render();
    
    // Restore depth testing state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Minimap::updateOrthographicCamera(const Vec3& playerPosition) {
    // Position camera above player for bird's-eye view
    Vec3 cameraPos = playerPosition + Vec3(0.0f, 20.0f, 0.0f);
    orthographicCamera.setPosition(cameraPos);
}

void Minimap::cleanupFramebuffer() {
    if (framebuffer) {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
    
    if (textureColorBuffer) {
        glDeleteTextures(1, &textureColorBuffer);
        textureColorBuffer = 0;
    }
    
    if (renderbuffer) {
        glDeleteRenderbuffers(1, &renderbuffer);
        renderbuffer = 0;
    }
    
    isFramebufferInitialized = false;
    isTextureValid = false;
}

} // namespace Engine
