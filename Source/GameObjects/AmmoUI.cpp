/**
 * AmmoUI.cpp - Implementation of Ammunition UI GameObject
 */

#include "AmmoUI.h"
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ShootingSystem.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Math/Math.h"
#include "../GameObjects/Weapon.h"
#include "../Engine/Rendering/SimpleTextRenderer.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Engine {

AmmoUI::AmmoUI(const std::string& name)
    : GameObject(name), 
      screenPosition(0.8f, -0.8f),    // Bottom-right corner (NDC coordinates)
      size(0.15f, 0.1f),              // Smaller size for text area
      textColor(1.0f, 1.0f, 1.0f),    // White text
      backgroundColor(0.0f, 0.0f, 0.0f), // Transparent background (will be handled by alpha)
      lowAmmoColor(1.0f, 0.3f, 0.3f), // Red for low ammo
      reloadColor(1.0f, 1.0f, 0.0f),  // Yellow for reloading
      currentAmmo(0),
      maxAmmo(30),
      reserveAmmo(0),
      maxReserveAmmo(90),
      isReloading(false),
      reloadProgress(0.0f),
      isVisible(true),
      lowAmmoThreshold(0.25f),        // 25% threshold for low ammo
      pulseTimer(0.0f),
      pulseSpeed(3.0f),               // 3 pulses per second
      weapon(nullptr),
      shootingComponent(nullptr),
      ammoText(""),
      reserveText(""),
      statusText("") {
    
    // Set as non-entity (UI object)
    setEntity(false);
    
    // Position and scale will be set in the render method using proper NDC coordinates
    setPosition(Vec3(0.0f, 0.0f, 0.0f));  // Will be overridden in render
    setScale(Vec3(1.0f, 1.0f, 1.0f));     // Will be overridden in render
}

bool AmmoUI::initialize() {
    if (!GameObject::initialize()) {
        return false;
    }
    
    std::cout << "Initializing AmmoUI..." << std::endl;
    
    // Initialize text strings
    updateTextStrings();
    
    std::cout << "AmmoUI initialized successfully" << std::endl;
    return true;
}

void AmmoUI::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update ammunition data from weapon
    updateAmmunitionData();
    
    // Update text strings
    updateTextStrings();
    
    // Update visual state (pulsing, colors, etc.)
    updateVisualState(deltaTime);
    
    // Call base update
    GameObject::update(deltaTime);
}

void AmmoUI::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || !isVisible) return;
    
    // Try to cast to SimpleTextRenderer
    const SimpleTextRenderer* textRenderer = dynamic_cast<const SimpleTextRenderer*>(&renderer);
    if (!textRenderer) {
        // Fallback to console output if text renderer not available
        std::cout << "\rAMMO: " << currentAmmo << "/" << maxAmmo << " | RESERVE: " << reserveAmmo << "/" << maxReserveAmmo;
        if (isReloading) {
            std::cout << " | RELOADING: " << (reloadProgress * 100.0f) << "%";
        }
        std::cout << "     ";
        std::cout.flush();
        return;
    }
    
    // Calculate screen position (bottom-right corner)
    // We'll position text relative to screen dimensions
    float screenWidth = 800.0f;  // TODO: Get actual screen width
    float screenHeight = 600.0f; // TODO: Get actual screen height
    
    float x = screenWidth - 200.0f;  // 200 pixels from right edge
    float y = 80.0f;                 // 80 pixels from bottom
    float scale = 1.5f;              // Larger scale for better visibility
    
    // Get current color based on state
    Vec3 color = getCurrentTextColor();
    
    // Render main ammo text
    std::string mainText = "AMMO: " + std::to_string(currentAmmo) + "/" + std::to_string(maxAmmo);
    textRenderer->renderText(mainText, x, y, scale, color);
    
    // Render reserve ammo text below
    std::string reserveText = "RESERVE: " + std::to_string(reserveAmmo);
    textRenderer->renderText(reserveText, x, y - 30.0f, scale * 0.8f, color);
    
    // Render status text if needed
    if (!statusText.empty()) {
        Vec3 statusColor = isReloading ? reloadColor : color;
        textRenderer->renderText(statusText, x, y - 50.0f, scale * 0.9f, statusColor);
    }
}

void AmmoUI::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up AmmoUI..." << std::endl;
    
    weapon = nullptr;
    shootingComponent = nullptr;
    
    GameObject::cleanup();
}

void AmmoUI::setupMesh() {
    // Create a simple quad mesh for the UI background
    // This would be a 2D rectangle positioned in screen space
    
    // Create vertex data with positions and texture coordinates
    // Format: x, y, z, u, v (5 floats per vertex)
    std::vector<float> vertexData = {
        // Position (x, y, z)    // TexCoord (u, v)
        -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f, 0.0f,     1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f   // Top-left
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    
    mesh = std::make_unique<Mesh>();
    mesh->createMeshWithTexCoords(vertexData, indices);
}

void AmmoUI::updateAmmunitionData() {
    if (!shootingComponent) {
        // Try to get shooting component from weapon if available
        if (weapon) {
            shootingComponent = &weapon->getShootingComponent();
        }
    }
    
    if (shootingComponent) {
        // Get current ammunition data
        currentAmmo = shootingComponent->getCurrentAmmo();
        reserveAmmo = shootingComponent->getReserveAmmo();
        isReloading = shootingComponent->isReloading();
        
        // Get max values from weapon stats
        const WeaponStats& stats = shootingComponent->getShootingSystem()->getWeaponStats();
        maxAmmo = stats.maxAmmo;
        maxReserveAmmo = stats.maxReserveAmmo;
        
        // Calculate reload progress if reloading
        if (isReloading) {
            reloadProgress = 1.0f - (stats.reloadTimer / stats.reloadTime);
            reloadProgress = std::max(0.0f, std::min(1.0f, reloadProgress));
        } else {
            reloadProgress = 0.0f;
        }
    } else {
        // Default values if no weapon is connected
        currentAmmo = 0;
        maxAmmo = 30;
        reserveAmmo = 0;
        maxReserveAmmo = 90;
        isReloading = false;
        reloadProgress = 0.0f;
    }
}

void AmmoUI::updateTextStrings() {
    // Update ammo text
    std::ostringstream ammoStream;
    ammoStream << currentAmmo << "/" << maxAmmo;
    ammoText = ammoStream.str();
    
    // Update reserve text
    std::ostringstream reserveStream;
    reserveStream << "Reserve: " << reserveAmmo;
    reserveText = reserveStream.str();
    
    // Update status text
    if (isReloading) {
        std::ostringstream statusStream;
        statusStream << "RELOADING " << std::fixed << std::setprecision(0) 
                    << (reloadProgress * 100.0f) << "%";
        statusText = statusStream.str();
    } else if (currentAmmo == 0 && reserveAmmo > 0) {
        statusText = "PRESS R TO RELOAD";
    } else if (currentAmmo == 0 && reserveAmmo == 0) {
        statusText = "OUT OF AMMO";
    } else {
        statusText = "";
    }
}

void AmmoUI::updateVisualState(float deltaTime) {
    // Update pulse timer
    pulseTimer += deltaTime * pulseSpeed;
    if (pulseTimer > 2.0f * 3.14159f) {
        pulseTimer -= 2.0f * 3.14159f;
    }
}

bool AmmoUI::isLowAmmo() const {
    if (maxAmmo <= 0) return false;
    float ammoPercentage = static_cast<float>(currentAmmo) / static_cast<float>(maxAmmo);
    return ammoPercentage <= lowAmmoThreshold;
}

bool AmmoUI::shouldPulse() const {
    return isLowAmmo() || isReloading;
}

float AmmoUI::getPulseAlpha() const {
    if (!shouldPulse()) return 1.0f;
    
    // Create a pulsing effect using sine wave
    float pulse = (std::sin(pulseTimer) + 1.0f) * 0.5f;
    return 0.5f + pulse * 0.5f; // Pulse between 0.5 and 1.0 alpha
}

Vec3 AmmoUI::getCurrentTextColor() const {
    if (isReloading) {
        return reloadColor;
    } else if (isLowAmmo()) {
        return lowAmmoColor;
    } else {
        return textColor;
    }
}

Vec3 AmmoUI::getCurrentBackgroundColor() const {
    Vec3 bgColor = backgroundColor;
    
    // Apply pulsing effect if needed
    if (shouldPulse()) {
        float alpha = getPulseAlpha();
        bgColor.x *= alpha;
        bgColor.y *= alpha;
        bgColor.z *= alpha;
    }
    
    return bgColor;
}

void AmmoUI::setWeapon(Weapon* weaponRef) {
    weapon = weaponRef;
    if (weapon) {
        shootingComponent = &weapon->getShootingComponent();
    }
}

void AmmoUI::setShootingComponent(WeaponShootingComponent* component) {
    shootingComponent = component;
}

Mat4 AmmoUI::getModelMatrix() const {
    // Create transformation matrix for NDC positioning
    Mat4 model = Mat4(); // Identity matrix
    
    // For NDC positioning, we need to account for the fact that our quad is centered at (0,0)
    // and spans from (-0.5, -0.5) to (0.5, 0.5). We need to position it so that the
    // bottom-right corner of our UI element is at the desired screen position.
    
    // Calculate the position that will place the bottom-right corner at screenPosition
    float x = screenPosition.x - size.x * 0.5f;
    float y = screenPosition.y - size.y * 0.5f;
    
    // Apply translation
    model = Engine::translate(model, Vec3(x, y, 0.0f));
    
    // Apply scale for UI size
    model = model * Engine::scale(Vec3(size.x, size.y, 1.0f));
    
    return model;
}

} // namespace Engine
