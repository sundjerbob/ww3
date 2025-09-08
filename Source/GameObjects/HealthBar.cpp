/**
 * HealthBar.cpp - Implementation of 3D Health Bar System for Monsters
 */

#include "HealthBar.h"
#include "Monster.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Math/Math.h"
#include <iostream>

namespace Engine {

HealthBar::HealthBar(const std::string& name, GameObject* monster)
    : GameObject(name),
      currentHealth(100.0f),
      maxHealth(100.0f),
      targetHealth(100.0f),
      healthTransitionSpeed(5.0f),
      backgroundColor(0.1f, 0.1f, 0.1f),  // Dark gray background for better contrast
      healthColor(0.0f, 1.0f, 0.0f),
      borderColor(0.8f, 0.8f, 0.8f),  // Light gray border for subtle appearance
      barWidth(3.0f),  // Reasonable size for health bar
      barHeight(0.4f),  // Reasonable height for health bar
      borderThickness(0.05f), // Thin border for clean look
      offsetY(2.5f), // Position closer to monster for better visual connection
      alwaysFaceCamera(true),
      billboardUp(0.0f, 1.0f, 0.0f),
      pulseTimer(0.0f),
      pulseSpeed(3.0f),
      isPulsing(false),
      targetMonster(monster) {
    
    // Set as entity so it's included in visibility system
    setEntity(true);
    
    // Set initial position
    if (targetMonster) {
        Vec3 monsterPos = targetMonster->getPosition();
        setPosition(Vec3(monsterPos.x, monsterPos.y + offsetY, monsterPos.z));
    }
}

bool HealthBar::initialize() {
    if (isInitialized) return true;
    
    std::cout << "Initializing HealthBar: " << getName() << std::endl;
    std::cout << "Target monster: " << (targetMonster ? targetMonster->getName() : "null") << std::endl;
    
    // Setup meshes
    setupMeshes();
    
    // Setup shader
    setupShader();
    
    // Meshes are created in setupMeshes() method
    
    // Update position from monster
    updatePositionFromMonster();
    
    isInitialized = true;
    std::cout << "HealthBar initialized successfully: " << getName() << std::endl;
    return true;
}

void HealthBar::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update position from monster
    updatePositionFromMonster();
    
    // Update health transition
    updateHealthTransition(deltaTime);
    
    // Update pulse animation
    updatePulseAnimation(deltaTime);
    
    // Call base update
    GameObject::update(deltaTime);
}

void HealthBar::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !isVisible()) {
        return;
    }
    
    // Debug output to see if health bars are being rendered
    static int renderCount = 0;
    renderCount++;
    if (renderCount % 60 == 0) { // Print every 60 frames (about every second)
        std::cout << "=== HEALTH BAR RENDER DEBUG ===" << std::endl;
        std::cout << "HealthBar: " << getName() << " is rendering!" << std::endl;
        std::cout << "Position: (" << getPosition().x << ", " << getPosition().y << ", " << getPosition().z << ")" << std::endl;
        std::cout << "Active: " << (isActive ? "true" : "false") << std::endl;
        std::cout << "Initialized: " << (isInitialized ? "true" : "false") << std::endl;
        std::cout << "Visible: " << (isVisible() ? "true" : "false") << std::endl;
        std::cout << "Health: " << currentHealth << "/" << maxHealth << std::endl;
        std::cout << "===============================" << std::endl;
    }
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable depth testing for proper depth ordering
    // Health bars should respect depth order (closer bars in front of distant ones)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // Allow equal depth values
    
    // Get billboard matrix for proper orientation
    Mat4 billboardMatrix = getBillboardMatrix(camera);
    
    // RENDER HEALTH BAR USING HEALTH BAR SHADER
    if (healthBarShader) {
        healthBarShader->use();
        
        // Set the transformation matrices
        healthBarShader->setMat4("model", billboardMatrix);
        healthBarShader->setMat4("view", camera.getViewMatrix());
        healthBarShader->setMat4("projection", camera.getProjectionMatrix());
        
        // Debug shader uniforms
        static int uniformDebugCount = 0;
        uniformDebugCount++;
        if (uniformDebugCount % 300 == 0) { // Print every 5 seconds
            std::cout << "=== HEALTH BAR SHADER DEBUG ===" << std::endl;
            std::cout << "Shader active: " << (healthBarShader ? "true" : "false") << std::endl;
            std::cout << "Border mesh: " << (borderMesh ? "valid" : "null") << std::endl;
            std::cout << "Background mesh: " << (backgroundMesh ? "valid" : "null") << std::endl;
            std::cout << "Health mesh: " << (healthMesh ? "valid" : "null") << std::endl;
            std::cout << "===============================" << std::endl;
        }
        
        // Render border (outer edge)
        if (borderMesh) {
            healthBarShader->setVec3("color", borderColor);
            healthBarShader->setFloat("alpha", 1.0f);
            borderMesh->render();
        }
        
        // Render background (dark fill)
        if (backgroundMesh) {
            healthBarShader->setVec3("color", backgroundColor);
            healthBarShader->setFloat("alpha", 0.7f);  // More transparent for cleaner look
            backgroundMesh->render();
        }
        
        // Render health fill (colored based on health percentage)
        if (healthMesh) {
            // Get health color based on percentage
            Vec3 healthColor = getHealthColorForPercentage(getHealthPercentage());
            
            // Apply pulse effect if pulsing
            if (isPulsing) {
                float pulse = (std::sin(pulseTimer) + 1.0f) * 0.5f;
                healthColor = healthColor * (0.8f + pulse * 0.2f);  // Subtle pulse
            }
            
            healthBarShader->setVec3("color", healthColor);
            healthBarShader->setFloat("alpha", 0.8f);  // More transparent for cleaner look
            healthMesh->render();
        }
    }
    
    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void HealthBar::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up HealthBar: " << getName() << std::endl;
    
    backgroundMesh.reset();
    healthMesh.reset();
    borderMesh.reset();
    healthBarShader.reset();
    targetMonster = nullptr;
    
    GameObject::cleanup();
}

void HealthBar::setHealth(float health, float max) {
    float oldHealth = currentHealth;
    float oldPercentage = getHealthPercentage();
    
    maxHealth = max;
    // Manual clamp instead of std::clamp
    currentHealth = (health < 0.0f) ? 0.0f : ((health > maxHealth) ? maxHealth : health);
    targetHealth = currentHealth;
    
    // Update health color based on percentage
    float percentage = getHealthPercentage();
    healthColor = getHealthColorForPercentage(percentage);
    
    // CRITICAL: Update health mesh immediately when health changes
    updateHealthMesh();
    
    onHealthChanged(oldHealth, currentHealth);
    onHealthPercentageChanged(oldPercentage, percentage);
}

void HealthBar::setCurrentHealth(float health) {
    setHealth(health, maxHealth);
}

void HealthBar::setMaxHealth(float max) {
    setHealth(currentHealth, max);
}

void HealthBar::setBarSize(float width, float height) {
    barWidth = width;
    barHeight = height;
    
    // Recreate meshes with new size
    if (isInitialized) {
        setupMeshes();
    }
}

bool HealthBar::isVisible() const {
    // Health bar should only be visible if it's active and the target monster is alive
    if (!getActive()) {
        return false; // Health bar is inactive
    }
    
    if (targetMonster) {
        // Cast to Monster to check if it's dead
        Monster* monster = dynamic_cast<Monster*>(targetMonster);
        if (monster && monster->isDead()) {
            return false; // Target monster is dead
        }
    }
    
    return true; // Health bar is active and monster is alive
}

Vec3 HealthBar::getHealthColorForPercentage(float percentage) const {
    if (percentage > 0.6f) {
        // Natural green for high health (60%+)
        return Vec3(0.2f, 0.8f, 0.2f); // Softer green
    } else if (percentage > 0.3f) {
        // Natural yellow/orange for medium health (30-60%)
        return Vec3(0.9f, 0.6f, 0.1f); // Orange-yellow
    } else {
        // Natural red for low health (0-30%)
        return Vec3(0.8f, 0.2f, 0.2f); // Softer red
    }
}

Mat4 HealthBar::getBillboardMatrix(const Camera& camera) const {
    if (!alwaysFaceCamera) {
        return getModelMatrix();
    }
    
    // DUAL-AXIS BILLBOARD CALCULATION
    // Health bars face player position, NOT camera direction
    // Y-axis rotation (horizontal) + X-axis rotation (vertical) for height changes
    Vec3 cameraPos = camera.getPosition();
    Vec3 healthBarPos = getPosition();
    
    // Calculate full 3D direction from health bar to camera
    Vec3 toCamera = cameraPos - healthBarPos;
    float distance = toCamera.length();
    
    if (distance < 0.001f) {
        // If camera is at the same position, use default orientation
        return getModelMatrix();
    }
    
    // Normalize the direction
    toCamera = toCamera.normalize();
    
    // Calculate horizontal direction (XZ plane) for Y-axis rotation
    Vec3 horizontalDir = Vec3(toCamera.x, 0.0f, toCamera.z);
    float horizontalDistance = sqrt(horizontalDir.x * horizontalDir.x + horizontalDir.z * horizontalDir.z);
    
    if (horizontalDistance < 0.001f) {
        // If camera is directly above/below, use default orientation
        return getModelMatrix();
    }
    
    // Normalize horizontal direction
    horizontalDir.x /= horizontalDistance;
    horizontalDir.z /= horizontalDistance;
    
    // Create dual-axis billboard matrix using look-at approach
    Mat4 billboardMatrix;
    
    // Calculate the right vector (perpendicular to toCamera and world up)
    Vec3 worldUp = Vec3(0.0f, 1.0f, 0.0f);
    Vec3 right = toCamera.cross(worldUp).normalize();
    
    // Calculate the up vector (perpendicular to toCamera and right)
    Vec3 up = right.cross(toCamera).normalize();
    
    // Create billboard matrix directly (much more efficient and correct)
    // Set rotation part - direct assignment
    billboardMatrix.m[0] = right.x;   billboardMatrix.m[1] = right.y;   billboardMatrix.m[2] = right.z;   billboardMatrix.m[3] = 0.0f;
    billboardMatrix.m[4] = up.x;      billboardMatrix.m[5] = up.y;      billboardMatrix.m[6] = up.z;      billboardMatrix.m[7] = 0.0f;
    billboardMatrix.m[8] = -toCamera.x; billboardMatrix.m[9] = -toCamera.y; billboardMatrix.m[10] = -toCamera.z; billboardMatrix.m[11] = 0.0f;
    
    // Set translation part - direct assignment
    billboardMatrix.m[12] = healthBarPos.x; billboardMatrix.m[13] = healthBarPos.y; billboardMatrix.m[14] = healthBarPos.z; billboardMatrix.m[15] = 1.0f;
    
    // Debug output every 600 frames (about every 10 seconds)
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 600 == 0) {
        std::cout << "=== HEALTH BAR DUAL-AXIS BILLBOARD DEBUG ===" << std::endl;
        std::cout << "HealthBar: " << getName() << std::endl;
        std::cout << "Camera pos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
        std::cout << "HealthBar pos: (" << healthBarPos.x << ", " << healthBarPos.y << ", " << healthBarPos.z << ")" << std::endl;
        std::cout << "ToCamera direction (3D): (" << toCamera.x << ", " << toCamera.y << ", " << toCamera.z << ")" << std::endl;
        std::cout << "Horizontal direction: (" << horizontalDir.x << ", " << horizontalDir.y << ", " << horizontalDir.z << ")" << std::endl;
        std::cout << "Distance: " << distance << ", Horizontal distance: " << horizontalDistance << std::endl;
        std::cout << "Right vector: (" << right.x << ", " << right.y << ", " << right.z << ")" << std::endl;
        std::cout << "Up vector: (" << up.x << ", " << up.y << ", " << up.z << ")" << std::endl;
        std::cout << "Dual-axis billboard matrix (look-at approach): " << std::endl;
        std::cout << "  [" << billboardMatrix.m[0] << ", " << billboardMatrix.m[1] << ", " << billboardMatrix.m[2] << ", " << billboardMatrix.m[3] << "]" << std::endl;
        std::cout << "  [" << billboardMatrix.m[4] << ", " << billboardMatrix.m[5] << ", " << billboardMatrix.m[6] << ", " << billboardMatrix.m[7] << "]" << std::endl;
        std::cout << "  [" << billboardMatrix.m[8] << ", " << billboardMatrix.m[9] << ", " << billboardMatrix.m[10] << ", " << billboardMatrix.m[11] << "]" << std::endl;
        std::cout << "  [" << billboardMatrix.m[12] << ", " << billboardMatrix.m[13] << ", " << billboardMatrix.m[14] << ", " << billboardMatrix.m[15] << "]" << std::endl;
        std::cout << "===========================================" << std::endl;
    }
    
    return billboardMatrix;
}

void HealthBar::updatePositionFromMonster() {
    if (!targetMonster) return;
    
    Vec3 monsterPos = targetMonster->getPosition();
    setPosition(Vec3(monsterPos.x, monsterPos.y + offsetY, monsterPos.z));
}

void HealthBar::onHealthChanged(float oldHealth, float newHealth) {
    // Override point for custom behavior
}

void HealthBar::onHealthPercentageChanged(float oldPercentage, float newPercentage) {
    // Override point for custom behavior
}

void HealthBar::setupMeshes() {
    // Create background mesh (full bar) - position only for simplified health bar shader
    std::vector<float> backgroundVertices = {
        // Position (x, y, z) only - 3 floats per vertex
        -barWidth/2, -barHeight/2, 0.0f,  // Bottom-left
         barWidth/2, -barHeight/2, 0.0f,  // Bottom-right
         barWidth/2,  barHeight/2, 0.0f,  // Top-right
        -barWidth/2,  barHeight/2, 0.0f   // Top-left
    };
    
    std::vector<unsigned int> backgroundIndices = {
        0, 1, 2,
        0, 2, 3
    };
    
    backgroundMesh = std::make_unique<Mesh>();
    backgroundMesh->createMesh(backgroundVertices, backgroundIndices);
    
    // Create health mesh (will be updated based on health percentage)
    healthMesh = std::make_unique<Mesh>();
    updateHealthMesh();
    
    // Create border mesh (slightly larger than background)
    float borderWidth = barWidth + borderThickness * 2;
    float borderHeight = barHeight + borderThickness * 2;
    
    std::vector<float> borderVertices = {
        // Position (x, y, z) only - 3 floats per vertex
        -borderWidth/2, -borderHeight/2, 0.0f,  // Bottom-left
         borderWidth/2, -borderHeight/2, 0.0f,  // Bottom-right
         borderWidth/2,  borderHeight/2, 0.0f,  // Top-right
        -borderWidth/2,  borderHeight/2, 0.0f   // Top-left
    };
    
    std::vector<unsigned int> borderIndices = {
        0, 1, 2,
        0, 2, 3
    };
    
    borderMesh = std::make_unique<Mesh>();
    borderMesh->createMesh(borderVertices, borderIndices);
}

void HealthBar::setupShader() {
    // Use dedicated health bar shaders for proper 3D positioning
    healthBarShader = std::make_unique<Shader>();
    healthBarShader->loadFromFiles("Resources/Shaders/healthbar_vertex.glsl", "Resources/Shaders/healthbar_fragment.glsl");
}

void HealthBar::updateHealthTransition(float deltaTime) {
    if (std::abs(targetHealth - currentHealth) > 0.01f) {
        float diff = targetHealth - currentHealth;
        float step = healthTransitionSpeed * deltaTime;
        
        if (std::abs(diff) <= step) {
            currentHealth = targetHealth;
        } else {
            currentHealth += (diff > 0 ? step : -step);
        }
        
        // Update health mesh
        updateHealthMesh();
        
        // Update health color
        float percentage = getHealthPercentage();
        healthColor = getHealthColorForPercentage(percentage);
    }
}

void HealthBar::updatePulseAnimation(float deltaTime) {
    if (isPulsing) {
        pulseTimer += deltaTime * pulseSpeed;
        if (pulseTimer > 2.0f * 3.14159f) {
            pulseTimer -= 2.0f * 3.14159f;
        }
    }
}

void HealthBar::updateHealthMesh() {
    if (!healthMesh) return;
    
    float healthPercentage = getHealthPercentage();
    float healthWidth = barWidth * healthPercentage;
    
    // Debug output to see health bar fullness
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 300 == 0) { // Print every 5 seconds
        std::cout << "=== HEALTH BAR FULLNESS DEBUG ===" << std::endl;
        std::cout << "HealthBar: " << getName() << std::endl;
        std::cout << "Health: " << currentHealth << "/" << maxHealth << std::endl;
        std::cout << "Health percentage: " << (healthPercentage * 100.0f) << "%" << std::endl;
        std::cout << "Bar width: " << barWidth << std::endl;
        std::cout << "Health width: " << healthWidth << std::endl;
        std::cout << "Health vertices: " << std::endl;
        std::cout << "  Bottom-left: (" << (-barWidth/2) << ", " << (-barHeight/2) << ", 0)" << std::endl;
        std::cout << "  Bottom-right: (" << (-barWidth/2 + healthWidth) << ", " << (-barHeight/2) << ", 0)" << std::endl;
        std::cout << "  Top-right: (" << (-barWidth/2 + healthWidth) << ", " << (barHeight/2) << ", 0)" << std::endl;
        std::cout << "  Top-left: (" << (-barWidth/2) << ", " << (barHeight/2) << ", 0)" << std::endl;
        std::cout << "=================================" << std::endl;
    }
    
    std::vector<float> healthVertices = {
        // Position (x, y, z) only - 3 floats per vertex for simplified shader
        -barWidth/2, -barHeight/2, 0.0f,  // Bottom-left
         -barWidth/2 + healthWidth, -barHeight/2, 0.0f,  // Bottom-right
         -barWidth/2 + healthWidth,  barHeight/2, 0.0f,  // Top-right
        -barWidth/2,  barHeight/2, 0.0f   // Top-left
    };
    
    std::vector<unsigned int> healthIndices = {
        0, 1, 2,
        0, 2, 3
    };
    
    healthMesh->createMesh(healthVertices, healthIndices);
}

} // namespace Engine
