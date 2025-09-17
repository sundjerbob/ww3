/**
 * TextureHealthBar.cpp - Implementation of 2D Texture-Based Health Bar System
 */

#include "TextureHealthBar.h"
#include "../Math/Camera.h"
#include <iostream>
#include <cmath>

namespace Engine {

TextureHealthBar::TextureHealthBar(float width, float height, float offset)
    : barWidth(width),
      barHeight(height),
      offsetY(offset),
      currentHealth(100.0f),
      maxHealth(100.0f),
      targetHealth(100.0f),
      healthTransitionSpeed(5.0f),
      backgroundColor(0.1f, 0.1f, 0.1f),  // Dark gray background
      healthColor(0.0f, 1.0f, 0.0f),      // Green health
      borderColor(0.8f, 0.8f, 0.8f),      // Light gray border
      alpha(0.8f),
      alwaysFaceCamera(true),
      billboardUp(0.0f, 1.0f, 0.0f),
      pulseTimer(0.0f),
      pulseSpeed(3.0f),
      isPulsing(false),
      isInitialized(false),
      isActive(true),
      healthBarVAO(0),
      healthBarVBO(0),
      healthBarEBO(0) {
}

TextureHealthBar::~TextureHealthBar() {
    cleanup();
}

bool TextureHealthBar::initialize() {
    if (isInitialized) return true;
    
    std::cout << "Initializing TextureHealthBar..." << std::endl;
    
    // Setup geometry
    setupGeometry();
    
    // Setup shader
    setupShader();
    
    // Generate initial texture
    generateHealthBarTexture();
    
    isInitialized = true;
    std::cout << "TextureHealthBar initialized successfully" << std::endl;
    return true;
}

void TextureHealthBar::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up TextureHealthBar..." << std::endl;
    
    cleanupGeometry();
    
    healthBarShader.reset();
    healthBarTexture.reset();
    
    isInitialized = false;
}

void TextureHealthBar::setupGeometry() {
    // Create a simple quad for health bar rendering
    // Define health bar quad with actual size (no matrix scaling needed)
    float halfWidth = barWidth * 0.5f;
    float halfHeight = barHeight * 0.5f;
    float vertices[] = {
        // positions (3D)   // texture coords
        -halfWidth, -halfHeight, 0.0f,   0.0f, 0.0f,  // bottom left
         halfWidth, -halfHeight, 0.0f,   1.0f, 0.0f,  // bottom right
         halfWidth,  halfHeight, 0.0f,   1.0f, 1.0f,  // top right
        -halfWidth,  halfHeight, 0.0f,   0.0f, 1.0f   // top left
    };
    
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    
    glGenVertexArrays(1, &healthBarVAO);
    glGenBuffers(1, &healthBarVBO);
    glGenBuffers(1, &healthBarEBO);
    
    std::cout << "=== VAO CREATION DEBUG ===" << std::endl;
    std::cout << "VAO ID: " << healthBarVAO << std::endl;
    std::cout << "VBO ID: " << healthBarVBO << std::endl;
    std::cout << "EBO ID: " << healthBarEBO << std::endl;
    std::cout << "=========================" << std::endl;
    
    glBindVertexArray(healthBarVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, healthBarVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, healthBarEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextureHealthBar::setupShader() {
    healthBarShader = std::make_unique<Shader>();
    
    // Use the same health bar shaders as before
    if (!healthBarShader->loadFromFiles("Resources/Shaders/healthbar_vertex.glsl", 
                                       "Resources/Shaders/healthbar_fragment.glsl")) {
        std::cerr << "Failed to load health bar shader files" << std::endl;
        return;
    }
    
    std::cout << "Health bar shader loaded successfully" << std::endl;
}

void TextureHealthBar::cleanupGeometry() {
    if (healthBarVAO) {
        glDeleteVertexArrays(1, &healthBarVAO);
        healthBarVAO = 0;
    }
    if (healthBarVBO) {
        glDeleteBuffers(1, &healthBarVBO);
        healthBarVBO = 0;
    }
    if (healthBarEBO) {
        glDeleteBuffers(1, &healthBarEBO);
        healthBarEBO = 0;
    }
}

void TextureHealthBar::generateHealthBarTexture() {
    // Create a simple texture for the health bar
    // For now, we'll create a basic texture and update it based on health
    const int textureWidth = 256;
    const int textureHeight = 64;
    
    std::vector<unsigned char> textureData(textureWidth * textureHeight * 4); // RGBA
    
    // Fill with background color
    for (int i = 0; i < textureWidth * textureHeight; ++i) {
        textureData[i * 4 + 0] = static_cast<unsigned char>(backgroundColor.x * 255); // R
        textureData[i * 4 + 1] = static_cast<unsigned char>(backgroundColor.y * 255); // G
        textureData[i * 4 + 2] = static_cast<unsigned char>(backgroundColor.z * 255); // B
        textureData[i * 4 + 3] = 255; // A
    }
    
    // Create texture
    healthBarTexture = std::make_unique<Texture>();
    healthBarTexture->loadFromMemory(textureData.data(), textureWidth, textureHeight, 4);
    
    std::cout << "Health bar texture generated successfully" << std::endl;
}

void TextureHealthBar::updateHealthBarTexture() {
    if (!healthBarTexture) return;
    
    // Update texture based on current health percentage
    // This is where we could implement more sophisticated texture generation
    // For now, we'll use the shader to handle the health percentage
}

void TextureHealthBar::setHealth(float health, float maxHealth) {
    this->currentHealth = health;
    this->maxHealth = maxHealth;
    this->targetHealth = health;
}

void TextureHealthBar::setTargetHealth(float target) {
    targetHealth = target;
}

void TextureHealthBar::setColors(const Vec3& background, const Vec3& health, const Vec3& border) {
    backgroundColor = background;
    healthColor = health;
    borderColor = border;
}

void TextureHealthBar::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update health transition
    updateHealthTransition(deltaTime);
    
    // Update pulse animation
    updatePulseAnimation(deltaTime);
    
    // Update texture if needed
    updateHealthBarTexture();
}

void TextureHealthBar::updateHealthTransition(float deltaTime) {
    if (std::abs(targetHealth - currentHealth) > 0.01f) {
        float diff = targetHealth - currentHealth;
        float step = healthTransitionSpeed * deltaTime;
        
        if (std::abs(diff) <= step) {
            currentHealth = targetHealth;
        } else {
            currentHealth += (diff > 0 ? step : -step);
        }
    }
}

void TextureHealthBar::updatePulseAnimation(float deltaTime) {
    if (isPulsing) {
        pulseTimer += deltaTime * pulseSpeed;
        if (pulseTimer > 2.0f * 3.14159f) {
            pulseTimer -= 2.0f * 3.14159f;
        }
    }
}

Mat4 TextureHealthBar::getBillboardMatrix(const Vec3& monsterPosition, const Camera& camera) const {
    // Position health bar above the monster with slight depth offset for proper occlusion
    // Small Z offset (0.1f) prevents z-fighting when health bar is at same level as monster parts
    Vec3 healthBarWorldPos = monsterPosition + Vec3(0.0f, offsetY, 0.1f);

    // Debug output to see positioning
    static int positionDebugCount = 0;
    positionDebugCount++;
    if (positionDebugCount % 600 == 0) { // Print every 10 seconds
        Vec3 cameraPos = camera.getPosition();
        float distanceToCamera = sqrt(
            (healthBarWorldPos.x - cameraPos.x) * (healthBarWorldPos.x - cameraPos.x) +
            (healthBarWorldPos.y - cameraPos.y) * (healthBarWorldPos.y - cameraPos.y) +
            (healthBarWorldPos.z - cameraPos.z) * (healthBarWorldPos.z - cameraPos.z)
        );
        
        std::cout << "=== HEALTH BAR POSITIONING ===" << std::endl;
        std::cout << "Monster position: (" << monsterPosition.x << ", " << monsterPosition.y << ", " << monsterPosition.z << ")" << std::endl;
        std::cout << "Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
        std::cout << "Offset Y: " << offsetY << std::endl;
        std::cout << "Health bar world pos: (" << healthBarWorldPos.x << ", " << healthBarWorldPos.y << ", " << healthBarWorldPos.z << ")" << std::endl;
        std::cout << "Height above monster: " << (healthBarWorldPos.y - monsterPosition.y) << std::endl;
        std::cout << "Distance to camera: " << distanceToCamera << std::endl;
        std::cout << "Health bar size: " << barWidth << " x " << barHeight << std::endl;
        std::cout << "==============================" << std::endl;
    }

    // CORRECT BILLBOARD: Only use player position, ignore camera yaw/pitch
    Vec3 cameraPos = camera.getPosition();
    
    // IMMEDIATE DEBUG: Confirm new calculation is running
    static int calcCount = 0;
    calcCount++;
    if (calcCount % 30 == 0) {
        std::cout << "*** NEW BILLBOARD CALC RUNNING: " << calcCount << " ***" << std::endl;
    }
    
    // ENHANCED BILLBOARD: Include both horizontal and vertical rotation to track player height
    Vec3 directionToPlayer = Vec3(
        cameraPos.x - healthBarWorldPos.x,
        cameraPos.y - healthBarWorldPos.y,  // Include Y difference for height tracking
        cameraPos.z - healthBarWorldPos.z
    );
    
    // Calculate total distance (including height)
    float totalDistance = sqrt(directionToPlayer.x * directionToPlayer.x + 
                              directionToPlayer.y * directionToPlayer.y + 
                              directionToPlayer.z * directionToPlayer.z);
    
    Mat4 modelMatrix = Mat4(); // Start with identity
    
    if (totalDistance > 0.001f) {
        // Construct proper billboard basis vectors including height
        Vec3 forward = directionToPlayer.normalize();  // Full 3D direction to player
        Vec3 billboardUp = Vec3(0.0f, 1.0f, 0.0f);    // World up vector
        Vec3 right = billboardUp.cross(forward).normalize();
        Vec3 up = forward.cross(right).normalize();
        
        // Build matrix using basis vectors (faces player including height)
        modelMatrix.m[0] = right.x;   modelMatrix.m[1] = right.y;   modelMatrix.m[2] = right.z;   modelMatrix.m[3] = 0;
        modelMatrix.m[4] = up.x;      modelMatrix.m[5] = up.y;      modelMatrix.m[6] = up.z;      modelMatrix.m[7] = 0;
        modelMatrix.m[8] = forward.x; modelMatrix.m[9] = forward.y; modelMatrix.m[10] = forward.z; modelMatrix.m[11] = 0;
    }
    
    // Set translation directly in the matrix (no rotation affects this)
    modelMatrix.m[12] = healthBarWorldPos.x;  // X position
    modelMatrix.m[13] = healthBarWorldPos.y;  // Y position
    modelMatrix.m[14] = healthBarWorldPos.z;  // Z position
    modelMatrix.m[15] = 1.0f;                 // W component
    
    // Debug: Show basis construction values and final position
    static int debugFrameCounter = 0;
    debugFrameCounter++;
    if (debugFrameCounter % 60 == 0) {  // Every 1 second
        Vec3 finalPos = Vec3(modelMatrix.m[12], modelMatrix.m[13], modelMatrix.m[14]);
        
        std::cout << "=== ENHANCED BILLBOARD DEBUG (WITH HEIGHT) ===" << std::endl;
        std::cout << "Health bar position: (" << healthBarWorldPos.x << ", " << healthBarWorldPos.y << ", " << healthBarWorldPos.z << ")" << std::endl;
        std::cout << "Player position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
        std::cout << "Direction to player: (" << directionToPlayer.x << ", " << directionToPlayer.y << ", " << directionToPlayer.z << ")" << std::endl;
        std::cout << "Total distance: " << totalDistance << " (includes height)" << std::endl;
        std::cout << "Using enhanced billboard with height tracking" << std::endl;
        std::cout << "=============================================" << std::endl;
    }

    return modelMatrix;
}

Vec3 TextureHealthBar::getHealthColorForPercentage(float percentage) const {
    if (percentage > 0.6f) {
        return Vec3(0.2f, 0.8f, 0.2f); // Green
    } else if (percentage > 0.3f) {
        return Vec3(0.9f, 0.6f, 0.1f); // Orange-yellow
    } else {
        return Vec3(0.8f, 0.2f, 0.2f); // Red
    }
}

void TextureHealthBar::render(const Vec3& monsterPosition, const Camera& camera) {
    // Debug render calls - reduced frequency
    static int renderCallCount = 0;
    renderCallCount++;
    if (renderCallCount % 600 == 0) {  // Every 10 seconds
        std::cout << "*** TEXTURE HEALTH BAR RENDER CALLED " << renderCallCount << " times ***" << std::endl;
    }
    
    if (!isActive || !isInitialized || !healthBarShader) {
        std::cout << "=== HEALTH BAR RENDER SKIPPED ===" << std::endl;
        std::cout << "Active: " << (isActive ? "YES" : "NO") << std::endl;
        std::cout << "Initialized: " << (isInitialized ? "YES" : "NO") << std::endl;
        std::cout << "Shader: " << (healthBarShader ? "YES" : "NO") << std::endl;
        std::cout << "================================" << std::endl;
        return;
    }
    
    // Debug output - faster for investigation
    static int renderCount = 0;
    renderCount++;
    if (renderCount % 600 == 0) { // Print every 10 seconds
        std::cout << "=== TEXTURE HEALTH BAR RENDER ===" << std::endl;
        std::cout << "Received monster position: (" << monsterPosition.x << ", " << monsterPosition.y << ", " << monsterPosition.z << ")" << std::endl;
        std::cout << "Calculated health bar world pos: (" << monsterPosition.x << ", " << (monsterPosition.y + offsetY) << ", " << (monsterPosition.z + 0.1f) << ")" << std::endl;
        std::cout << "Offset Y: " << offsetY << std::endl;
        std::cout << "Health percentage: " << (getHealthPercentage() * 100.0f) << "%" << std::endl;
        std::cout << "Health bar size: " << barWidth << " x " << barHeight << std::endl;
        std::cout << "=================================" << std::endl;
    }
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable depth testing for proper occlusion with monster parts
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Allow equal depth values, render closer objects first
    
    // Disable culling for billboard effect
    glDisable(GL_CULL_FACE);
    
    // Use health bar shader
    healthBarShader->use();
    
    // Get billboard matrix for 3D positioning
    Mat4 billboardMatrix = getBillboardMatrix(monsterPosition, camera);
    
    // Set matrices for 3D rendering
    healthBarShader->setMat4("model", billboardMatrix);
    healthBarShader->setMat4("view", camera.getViewMatrix());
    healthBarShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Set health percentage for shader
    healthBarShader->setFloat("healthPercentage", getHealthPercentage());
    
    // Set EXTREMELY VISIBLE health bar colors for debugging
    healthBarShader->setVec3("backgroundColor", Vec3(1.0f, 0.0f, 0.0f)); // Bright red background
    healthBarShader->setVec3("healthColor", Vec3(0.0f, 1.0f, 0.0f)); // Bright green health
    healthBarShader->setVec3("borderColor", Vec3(1.0f, 1.0f, 1.0f)); // Pure white border
    healthBarShader->setFloat("alpha", 1.0f); // Full opacity
    
    // Debug shader uniforms
    static int shaderDebugCount = 0;
    shaderDebugCount++;
    if (shaderDebugCount % 300 == 0) { // Print every 5 seconds
        std::cout << "=== SHADER UNIFORMS DEBUG ===" << std::endl;
        std::cout << "Health percentage: " << getHealthPercentage() << std::endl;
        std::cout << "Background color: (" << backgroundColor.x << ", " << backgroundColor.y << ", " << backgroundColor.z << ")" << std::endl;
        std::cout << "Health color: (" << getHealthColorForPercentage(getHealthPercentage()).x << ", " << getHealthColorForPercentage(getHealthPercentage()).y << ", " << getHealthColorForPercentage(getHealthPercentage()).z << ")" << std::endl;
        std::cout << "Border color: (" << borderColor.x << ", " << borderColor.y << ", " << borderColor.z << ")" << std::endl;
        std::cout << "Alpha: " << alpha << std::endl;
        std::cout << "=============================" << std::endl;
    }
    
    // SIMPLIFIED: No texture binding needed for simplified shader
    // healthBarTexture->bind(0);
    // healthBarShader->setInt("healthBarTexture", 0);
    
    // Render the quad
    glBindVertexArray(healthBarVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "=== OPENGL ERROR IN HEALTH BAR RENDER ===" << std::endl;
        std::cout << "OpenGL Error: " << error << std::endl;
        std::cout << "=========================================" << std::endl;
    }
    
    // Debug rendering
    static int renderDebugCount = 0;
    renderDebugCount++;
    if (renderDebugCount % 300 == 0) { // Print every 5 seconds
        std::cout << "=== RENDERING DEBUG ===" << std::endl;
        std::cout << "VAO: " << healthBarVAO << std::endl;
        std::cout << "Drawing 6 elements (2 triangles)" << std::endl;
        std::cout << "=======================" << std::endl;
    }
    
    // Restore OpenGL state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

} // namespace Engine
