/**
 * Minimap.cpp - Implementation of 2D Minimap GameObject
 */

#include "Minimap.h"
#include "Ground.h"
#include "SimpleChunkTerrainGround.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Math/Camera.h"
#include "../Engine/Math/Math.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "Arrow.h"

namespace Engine {

Minimap::Minimap(const std::string& name, float size)
    : GameObject(name), 
      minimapWidth(256), 
      minimapHeight(256), 
      minimapSize(size),
      // Default orthographic scope (captures 30x30 world units)
      orthoLeft(-15.0f),
      orthoRight(15.0f),
      orthoBottom(-15.0f),
      orthoTop(15.0f),
      orthoNear(0.1f),
      orthoFar(100.0f),
      framebuffer(0),
      textureColorBuffer(0),
      renderbuffer(0),
      scene(nullptr),
      playerPosition(0.0f, 0.0f, 0.0f),
      playerYaw(0.0f),
      groundReference(nullptr),
      terrainReference(nullptr),
      playerArrow(nullptr),
      playerArrowPtr(nullptr),
      configurableScopeSize(24.0f),  // Default scope size
      sceneObjectsDirty(true),
      lastUpdateFrame(0),
      lastPlayerPosition(0.0f, 0.0f, 0.0f),
      isFramebufferInitialized(false),
      isTextureValid(false) {
    
    // Set minimap to render in 2D screen space
    setPosition(Vec3(-0.8f, 0.8f, 0.0f)); // Top-left corner in NDC
    setScale(Vec3(minimapSize, minimapSize, 1.0f));
    
    // Mark as system object (not an entity)
    setEntity(false);
}

void Minimap::setPlayerPosition(const Vec3& position) {
    // Calculate player movement since last frame
    Vec3 movement = position - lastPlayerPosition;
    
    // Check if player has moved significantly (to avoid unnecessary updates)
    float moveThreshold = 0.1f; // Small threshold to avoid micro-movements
    float distance = movement.length();
    
    if (distance > moveThreshold) {
        // CORRECT MINIMAP MOVEMENT: World moves opposite to player movement
        // When player moves forward (W), world moves opposite to create illusion arrow moves forward
        // Only X and Z coordinates matter for minimap (Y height is ignored in orthographic view)
        
        // Create movement vector for minimap (ignore Y component)
        Vec3 minimapMovement = Vec3(movement.x, 0.0f, movement.z);
        
        // World moves opposite to player movement
        // This makes the arrow appear to move forward in its pointing direction
        // BUT: The coordinate system for world movement might be different from arrow rotation
        // Let's try a different approach - use player position directly
        // worldOffset = worldOffset - minimapMovement; // Comment out the offset approach
        
        sceneObjectsDirty = true;
        lastPlayerPosition = position;
        
        // Debug movement calculation
        static int moveDebugCount = 0;
        moveDebugCount++;
        // if (moveDebugCount % 30 == 0) { // More frequent debug output
        //     std::cout << "=== MINIMAP MOVEMENT DEBUG ===" << std::endl;
        //     std::cout << "Player movement: (" << movement.x << ", " << movement.y << ", " << movement.z << ")" << std::endl;
        //     std::cout << "Minimap movement (X,Z only): (" << movement.x << ", 0, " << movement.z << ")" << std::endl;
        //     std::cout << "World offset BEFORE: (" << worldOffset.x << ", " << worldOffset.y << ", " << worldOffset.z << ")" << std::endl;
        //     std::cout << "Arrow direction: (" << playerForwardDirection.x << ", " << playerForwardDirection.y << ", " << playerForwardDirection.z << ")" << std::endl;
        //     std::cout << "Expected: W moves world opposite to player movement" << std::endl;
        // }
    }
    
    playerPosition = position;
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
    
    // Setup orthographic camera for true top-down view
    orthographicCamera.setPosition(Vec3(0.0f, 20.0f, 0.0f)); // Higher up for better view
    orthographicCamera.setTopDownView(); // Use special method for true top-down view
    
    // Debug: Print camera setup
    std::cout << "Minimap camera setup:" << std::endl;
    std::cout << "  Position: (" << orthographicCamera.getPosition().x << ", " 
              << orthographicCamera.getPosition().y << ", " << orthographicCamera.getPosition().z << ")" << std::endl;
    std::cout << "  Forward: (" << orthographicCamera.getForward().x << ", " 
              << orthographicCamera.getForward().y << ", " << orthographicCamera.getForward().z << ")" << std::endl;
    std::cout << "  Pitch: " << orthographicCamera.getPitch() * 180.0f / 3.14159f << "°" << std::endl;
    std::cout << "  Yaw: " << orthographicCamera.getYaw() * 180.0f / 3.14159f << "°" << std::endl;
    
    // Create a simple quad mesh for rendering the minimap texture
    setupMesh();
    
    // Create player direction arrow (will be positioned at player's world position)
    auto arrow = std::make_unique<Arrow>("PlayerArrow", 3.0f, Vec3(1.0f, 0.0f, 0.0f)); // Red arrow, larger size for visibility
    
    // Store reference
    playerArrow = arrow.get();
    
    // Initialize the arrow
    if (!playerArrow->initialize()) {
        std::cerr << "Failed to initialize player arrow!" << std::endl;
    } else {
        std::cout << "Player arrow initialized successfully" << std::endl;
        std::cout << "DEBUG: Arrow mesh after init: " << (playerArrow->getMesh() ? "EXISTS" : "NULL") << std::endl;
        if (playerArrow->getMesh()) {
            std::cout << "DEBUG: Arrow mesh valid: " << (playerArrow->getMesh()->isValid() ? "YES" : "NO") << std::endl;
        }
    }
    
    // Position arrow at world center (0,0,0) - it will stay fixed there
    playerArrow->setPosition(Vec3(0.0f, 0.0f, 0.0f));
    
    // Set initial direction (pointing north by default)
    playerArrow->setDirectionFromYaw(0.0f);
    
    // Store arrow for rendering (not as child, but as separate object)
    playerArrowPtr = std::move(arrow);
    
    std::cout << "Arrow created successfully at position: (" << playerArrow->getPosition().x << ", " << playerArrow->getPosition().y << ", " << playerArrow->getPosition().z << ")" << std::endl;
    std::cout << "Arrow mesh valid: " << (playerArrow->getMesh() ? (playerArrow->getMesh()->isValid() ? "YES" : "NO") : "NULL") << std::endl;
    
    std::cout << "Minimap initialized successfully" << std::endl;
    return true;
}

void Minimap::update(float deltaTime) {
    GameObject::update(deltaTime);
    
    // Update orthographic camera to follow player
    if (scene) {
        updateOrthographicCamera(playerPosition);
    }
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
    
    // Create shader for arrow rendering with color support
    arrowShader = std::make_unique<Shader>();
    if (!arrowShader->loadFromFiles("Resources/Shaders/arrow_vertex.glsl", "Resources/Shaders/arrow_fragment.glsl")) {
        std::cerr << "Failed to load arrow shader" << std::endl;
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

bool Minimap::updateSceneObjects() {
    if (!scene) {
        std::cerr << "No scene reference for minimap object update" << std::endl;
        return false;
    }
    
    // Clear previous frame's objects
    sceneObjects.clear();
    
    // Get all game objects from the scene
    const auto& gameObjects = scene->getGameObjects();
    
    // Phase 1: Use Ground's global visibility system (Approach 2)
    if (groundReference) {
        // Get visible entities from Ground's visibility system
        const auto& visibleEntities = groundReference->getVisibleEntities();
        
        // Add visible entities to minimap scene (with coordinate validation)
        for (const auto* entity : visibleEntities) {
            if (entity && entity->getMesh() && entity->getMesh()->isValid()) {
                // Phase 2: Double-check coordinate safety (can be removed after testing)
                if (isEntityInMinimapScope(entity)) {
                    sceneObjects.push_back(entity);
                } else {
                    std::cout << "Minimap: Skipping entity " << entity->getName() << " - outside scope" << std::endl;
                }
            }
        }
        
        std::cout << "Minimap: Using Ground visibility system - " << visibleEntities.size() << " visible entities" << std::endl;
    }
    
    static int frameCount = 0;
    frameCount++;

    // Only print every 100 frames to avoid spam
    if (frameCount % 1000 == 0) {
        std::cout << "Frame " << frameCount << ": Updating " << gameObjects.size() << " scene objects for GPU-based minimap rendering..." << std::endl;
    }
    
    for (const auto& gameObject : gameObjects) {
        // Skip crosshair (it's a 2D UI element)
        if (gameObject->getName() == "Crosshair") {
            continue;
        }
        
        // Special handling for Ground objects (both chunk-based and simple terrain)
        if (gameObject->getName() == "Ground" || gameObject->getName() == "SimpleTerrain") {
            // Handle Ground objects with chunks (old system)
            const Ground* ground = dynamic_cast<const Ground*>(gameObject.get());
            if (ground) {
                const auto& chunks = ground->getChunks();
                for (const auto& chunk : chunks) {
                    if (chunk && chunk->getActive()) {
                        const Mesh* chunkMesh = chunk->getMesh();
                        if (chunkMesh && chunkMesh->isValid()) {
                            // Add chunk to scene objects for rendering
                            sceneObjects.push_back(chunk.get());
                            
                            if (frameCount % 1000 == 0) {
                                Vec3 position = chunk->getPosition();
                                std::cout << "  - " << chunk->getName() << " (chunk): pos(" << position.x << "," << position.y << "," << position.z << ")" << std::endl;
                            }
                        }
                    }
                }
            }
            
            // Handle SimpleChunkTerrainGround (new chunk-based system)
            const SimpleChunkTerrainGround* simpleChunkTerrain = dynamic_cast<const SimpleChunkTerrainGround*>(gameObject.get());
            if (simpleChunkTerrain && simpleChunkTerrain->getActive()) {
                // Store reference to terrain for separate rendering
                terrainReference = const_cast<SimpleChunkTerrainGround*>(simpleChunkTerrain);
                
                if (frameCount % 1000 == 0) {
                    Vec3 position = simpleChunkTerrain->getPosition();
                    std::cout << "  - " << simpleChunkTerrain->getName() << " (simple chunk terrain): pos(" << position.x << "," << position.y << "," << position.z << ")" << std::endl;
                }
            }
            continue;
        }
        
        // Get the mesh from the game object
        const Mesh* objectMesh = gameObject->getMesh();
        if (!objectMesh || !objectMesh->isValid()) {
            if (frameCount % 1000 == 0) {
                std::cout << "Skipping " << gameObject->getName() << " - no valid mesh" << std::endl;
            }
            continue;
        }
        
        // For entities, check if they should be rendered based on chunk visibility
        if (gameObject->getEntity()) {
            // Simplified entity visibility: render all entities within minimap bounds
            Vec3 entityPos = gameObject->getPosition();
            bool withinMinimapBounds = (
                entityPos.x >= orthoLeft && entityPos.x <= orthoRight &&
                entityPos.z >= orthoBottom && entityPos.z <= orthoTop
            );
            
            if (withinMinimapBounds) {
                if (frameCount % 1000 == 0) {
                    std::cout << "Including entity " << gameObject->getName() << " in minimap - within bounds" << std::endl;
                }
            } else {
                if (frameCount % 1000 == 0) {
                    std::cout << "Skipping entity " << gameObject->getName() << " in minimap - outside bounds" << std::endl;
                }
                continue;
            }
        }
        
        // Add to scene objects for GPU rendering
        sceneObjects.push_back(gameObject.get());
        
        // Only print every 100 frames to avoid spam
        if (frameCount % 1000 == 0) {
            Vec3 position = gameObject->getPosition();
            Vec3 rotation = gameObject->getRotation();
            Vec3 scale = gameObject->getScale();
            std::cout << "  - " << gameObject->getName() << ": pos(" << position.x << "," << position.y << "," << position.z 
                      << ") rot(" << rotation.x << "," << rotation.y << "," << rotation.z 
                      << ") scale(" << scale.x << "," << scale.y << "," << scale.z << ")" << std::endl;
        }
    }
    
    if (frameCount % 1000 == 0) {
        std::cout << "GPU-based minimap objects prepared: " << sceneObjects.size() << " objects" << std::endl;
        std::cout << "Minimap bounds: X[" << orthoLeft << " to " << orthoRight << "] Z[" << orthoBottom << " to " << orthoTop << "]" << std::endl;
        std::cout << "Player position: (" << playerPosition.x << ", " << playerPosition.y << ", " << playerPosition.z << ")" << std::endl;
        if (playerArrow) {
            std::cout << "Arrow position: (" << playerArrow->getPosition().x << ", " << playerArrow->getPosition().y << ", " << playerArrow->getPosition().z << ")" << std::endl;
            std::cout << "Arrow rotation: (" << playerArrow->getRotation().x << ", " << playerArrow->getRotation().y << ", " << playerArrow->getRotation().z << ")" << std::endl;
            std::cout << "Arrow in bounds: " << (playerArrow->getPosition().x >= orthoLeft && playerArrow->getPosition().x <= orthoRight && 
                                                 playerArrow->getPosition().z >= orthoBottom && playerArrow->getPosition().z <= orthoTop ? "YES" : "NO") << std::endl;
        }
    }
    
    return true;
}

void Minimap::renderSceneToTexture() {
    // Only update scene objects when necessary (player moved, objects changed, etc.)
    if (sceneObjectsDirty) {
        updateSceneObjects();
        sceneObjectsDirty = false;
    }
    
    if (sceneObjects.empty() || !isFramebufferInitialized) return;
    
    // Store current OpenGL state
    GLint currentFramebuffer;
    GLint currentViewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, static_cast<GLsizei>(minimapWidth), static_cast<GLsizei>(minimapHeight));
    
    // Clear framebuffer
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f); calling this fucked up the clearclor in game, (sky color)
    glClear(/*GL_COLOR_BUFFER_BIT | do not need to reset COLOR_BUFFER_BIT*/ GL_DEPTH_BUFFER_BIT);
    
    // Use orthographic shader
    if (orthographicShader) {
        orthographicShader->use();
        
        // Set orthographic projection matrix using configurable scope
        Mat4 orthoMatrix = Engine::orthographic(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
        orthographicShader->setMat4("projection", orthoMatrix);
        orthographicShader->setMat4("view", orthographicCamera.getViewMatrix());
        
        // Disable height-based coloring for minimap (use object colors instead)
        orthographicShader->setInt("useHeightColoring", 0);
        
        // Debug transformation matrices
        static int matrixDebugCount = 0;
        matrixDebugCount++;
        if (matrixDebugCount % 5 == 0) { // Very frequent debug
            std::cout << "=== TRANSFORMATION MATRICES DEBUG ===" << std::endl;
            std::cout << "Orthographic matrix scope: L=" << orthoLeft << " R=" << orthoRight << " B=" << orthoBottom << " T=" << orthoTop << std::endl;
            
            // Debug a specific entity transformation
            for (const GameObject* obj : sceneObjects) {
                if (obj && obj->getName() == "RedCube") {
                    Vec3 worldPos = obj->getPosition();
                    std::cout << "RedCube world position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
                    
                    // Test coordinate transformation
                    // RedCube is at (5, 0, 3) in world coordinates
                    // With current scope, it should be visible
                    float normalizedX = (worldPos.x - orthoLeft) / (orthoRight - orthoLeft);
                    float normalizedY = (worldPos.z - orthoBottom) / (orthoTop - orthoBottom);
                    std::cout << "RedCube normalized position: (" << normalizedX << ", " << normalizedY << ")" << std::endl;
                    std::cout << "Scope: L=" << orthoLeft << " R=" << orthoRight << " B=" << orthoBottom << " T=" << orthoTop << std::endl;
                    std::cout << "Expected: RedCube should be in top-right area of minimap" << std::endl;
                    break;
                }
            }
        }
        
        // Debug: Print camera state during rendering
        static int debugFrameCount = 0;
        debugFrameCount++;
        if (debugFrameCount % 300 == 0) { // Print every 5 seconds (60fps * 5)
            std::cout << "Minimap rendering debug (MOVEMENT-BASED):" << std::endl;
            std::cout << "  Camera position: (" << orthographicCamera.getPosition().x << ", " 
                      << orthographicCamera.getPosition().y << ", " << orthographicCamera.getPosition().z << ") [FIXED]" << std::endl;
            std::cout << "  Camera forward: (" << orthographicCamera.getForward().x << ", " 
                      << orthographicCamera.getForward().y << ", " << orthographicCamera.getForward().z << ") [TOP-DOWN]" << std::endl;
            std::cout << "  Orthographic scope: [" << orthoLeft << ", " << orthoRight << ", " 
                      << orthoBottom << ", " << orthoTop << "] [MOVEMENT OFFSET]" << std::endl;
            std::cout << "  Player position: (" << playerPosition.x << ", " << playerPosition.y << ", " << playerPosition.z << ")" << std::endl;
            std::cout << "  Player forward: (" << playerForwardDirection.x << ", " << playerForwardDirection.y << ", " << playerForwardDirection.z << ")" << std::endl;
            std::cout << "  World offset: (" << worldOffset.x << ", " << worldOffset.y << ", " << worldOffset.z << ")" << std::endl;
            std::cout << "  Arrow position: (0, 0, 0) [FIXED AT CENTER]" << std::endl;
            std::cout << "  Expected: Movement-based world offset" << std::endl;
        }
        
        // ============================================================================
        // GPU-BASED RENDERING: Render each object individually with its own transform
        // ============================================================================
        // 
        // PERFORMANCE BENEFITS:
        // - No CPU vertex transformation calculations
        // - GPU handles all matrix multiplications in parallel
        // - Each object's transformation is applied via shader uniforms
        // - Much more efficient than CPU-based mesh aggregation
        // ============================================================================
        
        // Render each scene object individually
        for (const GameObject* gameObject : sceneObjects) {
            const Mesh* objectMesh = gameObject->getMesh();
            if (objectMesh && objectMesh->isValid()) {
                // Set the object's complete transformation matrix (includes rotation, position, scale)
                Mat4 modelMatrix = gameObject->getModelMatrix();
                orthographicShader->setMat4("model", modelMatrix);
                
                // Set the object's individual color (no height-based coloring in minimap)
                orthographicShader->setVec3("color", gameObject->getColor());
                
                // Render the object's mesh
                objectMesh->render();
            }
        }
        
        // Render terrain chunks separately
        if (terrainReference && terrainReference->getActive()) {
            for (const auto& chunkPair : terrainReference->getChunkMeshes()) {
                const Mesh* chunkMesh = chunkPair.second.get();
                if (chunkMesh && chunkMesh->isValid()) {
                    // Use terrain's model matrix and color
                    Mat4 modelMatrix = terrainReference->getModelMatrix();
                    orthographicShader->setMat4("model", modelMatrix);
                    orthographicShader->setVec3("color", terrainReference->getColor());
                    
                    // Render the chunk mesh
                    chunkMesh->render();
                }
            }
        }
        
        // Note: Center indicator will be rendered as 2D overlay after minimap texture display
    }
    
    // Render the center indicator (arrow) directly onto the minimap texture
    renderCenterIndicator();
    
    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
    glViewport(static_cast<GLint>(currentViewport[0]), static_cast<GLint>(currentViewport[1]), 
               static_cast<GLsizei>(currentViewport[2]), static_cast<GLsizei>(currentViewport[3]));
    
    isTextureValid = true;
}

// ============================================================================
// MINIMAP CONFIGURATION METHODS
// ============================================================================

void Minimap::setMinimapDimensions(int width, int height) {
    minimapWidth = width;
    minimapHeight = height;
    
    // Recreate framebuffer with new dimensions if already initialized
    if (isFramebufferInitialized) {
        cleanupFramebuffer();
        initializeFramebuffer();
    }
    
    std::cout << "Minimap dimensions set to: " << width << "x" << height << std::endl;
}

void Minimap::setOrthographicScope(float left, float right, float bottom, float top, float near, float far) {
    orthoLeft = left;
    orthoRight = right;
    orthoBottom = bottom;
    orthoTop = top;
    orthoNear = near;
    orthoFar = far;
    
    std::cout << "Orthographic scope set to: L=" << left << " R=" << right 
              << " B=" << bottom << " T=" << top << " N=" << near << " F=" << far << std::endl;
}

void Minimap::getOrthographicScope(float& left, float& right, float& bottom, float& top, float& near, float& far) const {
    left = orthoLeft;
    right = orthoRight;
    bottom = orthoBottom;
    top = orthoTop;
    near = orthoNear;
    far = orthoFar;
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

void Minimap::renderArrowOverlay() {
    // Debug: Check arrow state
    static int debugCount = 0;
    debugCount++;
    
    if (!playerArrow || !playerArrow->getMesh() || !playerArrow->getMesh()->isValid()) {
        if (debugCount % 100 == 0) {
            std::cout << "DEBUG: Arrow not ready for rendering" << std::endl;
        }
        return;
    }
    
    // Store current OpenGL state
    GLint currentViewport[4];
    GLboolean depthTestEnabled;
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    
    // Disable depth testing for 2D overlay
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for 2D overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Calculate minimap screen position and size
    // The minimap is positioned at (-0.8, 0.8) with scale (minimapSize, minimapSize)
    // We need to convert this to screen coordinates
    float minimapScreenX = (-0.8f + 1.0f) * 0.5f * currentViewport[2]; // Convert NDC to screen
    float minimapScreenY = (0.8f + 1.0f) * 0.5f * currentViewport[3];  // Convert NDC to screen
    float minimapScreenSize = minimapSize * 0.5f * currentViewport[2];  // Convert NDC to screen
    
    // Set viewport to minimap area
    glViewport(static_cast<GLint>(minimapScreenX), static_cast<GLint>(currentViewport[3] - minimapScreenY - minimapScreenSize), 
               static_cast<GLsizei>(minimapScreenSize), static_cast<GLsizei>(minimapScreenSize));
    
    // Use orthographic shader for 2D rendering
    if (orthographicShader) {
        orthographicShader->use();
        
        // Create orthographic projection for 2D overlay (-1 to 1 in both axes)
        Mat4 orthoMatrix = Engine::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        orthographicShader->setMat4("projection", orthoMatrix);
        
        // Identity view matrix for 2D rendering
        Mat4 viewMatrix = Mat4(); // Identity matrix
        orthographicShader->setMat4("view", viewMatrix);
        
        // Create model matrix for arrow at center (0,0,0)
        Mat4 modelMatrix = Mat4(); // Identity matrix
        
        // Apply arrow rotation around Z-axis (in/out of screen) for 2D rotation
        Vec3 rotation = playerArrow->getRotation();
        
        if (rotation.y != 0.0f) {
            Mat4 rotZ = Engine::rotateZ(rotation.y * 3.14159f / 180.0f);
            modelMatrix = Engine::multiply(modelMatrix, rotZ);
        }
        
        orthographicShader->setMat4("model", modelMatrix);
        orthographicShader->setVec3("color", playerArrow->getColor());
        
        // Render the arrow mesh
        playerArrow->getMesh()->render();
        
        if (debugCount % 100 == 0) {
            std::cout << "DEBUG: Arrow rendered successfully - rotation: " << rotation.y << " degrees" << std::endl;
        }
    }
    
    // Restore viewport
    glViewport(static_cast<GLint>(currentViewport[0]), static_cast<GLint>(currentViewport[1]), 
               static_cast<GLsizei>(currentViewport[2]), static_cast<GLsizei>(currentViewport[3]));
    
    // Restore depth testing state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    
    // Disable blending
    glDisable(GL_BLEND);
}

void Minimap::updateOrthographicCamera(const Vec3& playerPosition) {
    // FIXED CAMERA: Camera stays at fixed position above world center
    // This ensures consistent perspective and prevents diagonal movement
    Vec3 cameraPos = Vec3(0.0f, 20.0f, 0.0f); // Fixed camera above world center
    orthographicCamera.setPosition(cameraPos);
    
    // CORRECT MINIMAP MOVEMENT: Use accumulated world offset
    // The world offset is calculated based on player movement direction
    // This ensures world moves opposite to player movement
    
    // Center the orthographic scope on the player's current position
    // This should make the player appear at the center of the minimap
    // and all entities should appear at their correct relative positions
    orthoLeft = playerPosition.x - configurableScopeSize;   // Center on player X
    orthoRight = playerPosition.x + configurableScopeSize;  // Center on player X
    orthoBottom = playerPosition.z - configurableScopeSize; // Center on player Z
    orthoTop = playerPosition.z + configurableScopeSize;    // Center on player Z
    
    // Debug orthographic scope
    static int scopeDebugCount = 0;
    scopeDebugCount++;
    if (scopeDebugCount % 10 == 0) { // More frequent debug
        std::cout << "=== ORTHOGRAPHIC SCOPE DEBUG ===" << std::endl;
        std::cout << "World offset: (" << worldOffset.x << ", " << worldOffset.y << ", " << worldOffset.z << ")" << std::endl;
        std::cout << "Orthographic scope: L=" << orthoLeft << " R=" << orthoRight << " B=" << orthoBottom << " T=" << orthoTop << std::endl;
        std::cout << "Player position: (" << playerPosition.x << ", " << playerPosition.y << ", " << playerPosition.z << ")" << std::endl;
        std::cout << "Arrow rotation angle: " << (playerYaw + 90.0f + 180.0f) << " degrees" << std::endl;
        
        // Debug entity positions
        std::cout << "=== ENTITY POSITIONS DEBUG ===" << std::endl;
        for (const GameObject* obj : sceneObjects) {
            if (obj && obj->getEntity()) {
                Vec3 pos = obj->getPosition();
                std::cout << "Entity " << obj->getName() << ": world(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
                std::cout << " in scope: " << (pos.x >= orthoLeft && pos.x <= orthoRight && pos.z >= orthoBottom && pos.z <= orthoTop ? "YES" : "NO") << std::endl;
            }
        }
    }
    
    // Camera rotation is already set to top-down in initialize()
    // No need to change rotation during updates
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

// Note: shouldRenderEntityInMinimap method removed - now using simplified bounds checking inline

void Minimap::setPlayerDirection(const Vec3& direction) {
    if (playerArrow) {
        playerArrow->setDirection(direction);
    }
}

void Minimap::setPlayerDirectionFromYaw(float yawDegrees) {
    // Store the player yaw for arrow rotation
    playerYaw = yawDegrees;
    
    // Calculate player's forward direction vector from yaw
    float yawRadians = yawDegrees * 3.14159f / 180.0f;
    playerForwardDirection.x = cos(yawRadians);
    playerForwardDirection.y = 0.0f; // No vertical component for top-down view
    playerForwardDirection.z = sin(yawRadians);
    playerForwardDirection = playerForwardDirection.normalize();
    
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 100 == 0) {
        std::cout << "DEBUG: Player yaw updated to: " << yawDegrees << " degrees" << std::endl;
        std::cout << "DEBUG: Player forward direction: (" << playerForwardDirection.x << ", " 
                  << playerForwardDirection.y << ", " << playerForwardDirection.z << ")" << std::endl;
    }
    
    if (playerArrow) {
        // Arrow is rendered as 2D overlay at minimap center
        // Only rotation matters, position is handled in renderArrowOverlay()
        playerArrow->setDirectionFromYaw(yawDegrees);
    }
}

void Minimap::renderCenterIndicator() {
    // Render a rotating red arrow directly onto the minimap texture at its center
    // This is called during renderSceneToTexture() to draw the arrow onto the texture
    
    // Store current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for the arrow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use the arrow shader for 2D rendering with color support
    if (!arrowShader) {
        std::cout << "DEBUG: arrowShader is NULL in renderCenterIndicator" << std::endl;
        return;
    }
    
    arrowShader->use();
    
    // Create arrow shape (triangle pointing up by default)
    // Position it at center of minimap texture (0,0 in NDC)
    float arrowSize = 0.2f; // Arrow size in NDC units (bigger for better visibility)
    std::vector<float> vertices = {
        // Arrow head (triangle pointing up) - centered at origin
        // Revert back to pointing up
        0.0f, arrowSize, 0.0f,                    // 0: tip of arrow (pointing up)
        -arrowSize * 0.5f, -arrowSize, 0.0f,      // 1: left base
         arrowSize * 0.5f, -arrowSize, 0.0f       // 2: right base
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2  // Single triangle
    };
    
    // Create temporary mesh
    Mesh tempMesh;
    if (!tempMesh.createMesh(vertices, indices)) {
        std::cout << "DEBUG: Failed to create arrow mesh" << std::endl;
        return;
    }
    
    // Transform camera yaw to minimap coordinate system
    // Camera: 0° = negative Z, 90° = positive X, 180° = positive Z, 270° = negative X
    // Minimap: Arrow points up by default, so we need to adjust the rotation
    // When camera yaw is 0° (facing negative Z), arrow should point up (0° in minimap)
    // When camera yaw is 90° (facing positive X), arrow should point right (90° in minimap)
    // BUT: Arrow is pointing opposite direction, so add 180° more
    // Calculate arrow rotation from player's forward direction vector
    // Use atan2 to get the angle from the forward direction
    float rotationAngle = atan2(playerForwardDirection.x, playerForwardDirection.z) * 180.0f / 3.14159f;
    
    // Add 180° offset because the coordinate system might be inverted
    // Add another 180° to flip the direction
    rotationAngle += 180.0f + 180.0f;
    
    // Convert to degrees and wrap to 0-360 range
    while (rotationAngle < 0.0f) rotationAngle += 360.0f;
    while (rotationAngle >= 360.0f) rotationAngle -= 360.0f;
    
    // For testing: make the arrow rotate continuously to see if rotation works
    // Comment out this line to use actual player yaw instead of test rotation
    // static float testRotation = 0.0f;
    // testRotation += 2.0f; // Rotate 2 degrees per frame
    // if (testRotation > 360.0f) testRotation -= 360.0f;
    // rotationAngle = testRotation; // Use test rotation for now
    
    // Create transformation matrix for rotation around center
    Mat4 modelMatrix = Mat4(); // Identity matrix
    
    // Since the arrow vertices are already centered at (0,0,0), 
    // we only need to apply rotation - no translation needed
    // This ensures the arrow rotates around its own center
    Mat4 rotZ = Engine::rotateZ(rotationAngle * 3.14159f / 180.0f);
    modelMatrix = rotZ; // Just rotation, no translation
    
    // For 2D rendering, we only need the model matrix for rotation
    // The vertex shader will handle the 2D positioning directly
    arrowShader->setMat4("model", modelMatrix);
    
    // Set identity matrices for unused uniforms (to avoid shader errors)
    Mat4 identityMatrix = Mat4();
    arrowShader->setMat4("view", identityMatrix);
    arrowShader->setMat4("projection", identityMatrix);
    
    // Set red color for the arrow
    arrowShader->setVec3("color", Vec3(1.0f, 0.0f, 0.0f));
    
    // Render the arrow
    tempMesh.render();
    
    // Disable blending
    glDisable(GL_BLEND);
    
    // Restore depth testing
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 5 == 0) { // Very frequent debug
        std::cout << "=== ARROW ROTATION DEBUG ===" << std::endl;
        std::cout << "Player yaw: " << playerYaw << " degrees" << std::endl;
        std::cout << "Rotation angle: " << rotationAngle << " degrees" << std::endl;
        std::cout << "Player forward direction: (" << playerForwardDirection.x << ", " << playerForwardDirection.y << ", " << playerForwardDirection.z << ")" << std::endl;
        std::cout << "Expected: Arrow should point in the direction the player is facing" << std::endl;
        
        // Test arrow direction calculation
        float expectedRotation = atan2(playerForwardDirection.x, playerForwardDirection.z) * 180.0f / 3.14159f + 180.0f + 180.0f;
        while (expectedRotation < 0.0f) expectedRotation += 360.0f;
        while (expectedRotation >= 360.0f) expectedRotation -= 360.0f;
        std::cout << "Expected rotation: " << expectedRotation << " degrees" << std::endl;
        std::cout << "Actual rotation: " << rotationAngle << " degrees" << std::endl;
        std::cout << "Difference: " << (rotationAngle - expectedRotation) << " degrees" << std::endl;
        std::cout << "Forward vector: (" << playerForwardDirection.x << ", " << playerForwardDirection.z << ")" << std::endl;
    }
}

// ============================================================================
// COORDINATE VALIDATION SAFETY SYSTEM
// ============================================================================

bool Minimap::isEntityInMinimapScope(const GameObject* entity) const {
    if (!entity) return false;
    
    Vec3 entityPos = entity->getPosition();
    
    // Check if entity position is within minimap's orthographic scope
    bool withinScope = (
        entityPos.x >= orthoLeft && entityPos.x <= orthoRight &&
        entityPos.z >= orthoBottom && entityPos.z <= orthoTop
    );
    
    if (!withinScope) {
        std::cout << "WARNING: Entity " << entity->getName() << " at (" 
                  << entityPos.x << ", " << entityPos.z << ") is outside minimap scope [" 
                  << orthoLeft << " to " << orthoRight << ", " 
                  << orthoBottom << " to " << orthoTop << "]" << std::endl;
    }
    
    return withinScope;
}

void Minimap::renderPlayerPositionIndicator() {
    // Render a small dot at the player's position on the minimap
    // Since the camera follows the player, the player should always be at center (0,0)
    // This method is currently not needed since the arrow serves as the player indicator
    
    // For now, this is a placeholder - the arrow at center already shows player position
    // If we want a separate dot, we can implement it here later
}

} // namespace Engine
