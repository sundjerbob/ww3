/**
 * Player.cpp - Implementation of Player GameObject with health system
 */

#include "Player.h"
#include "HealthBar.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Rendering/Renderer.h"
#include <iostream>
#include <algorithm>

namespace Engine {

Player::Player(const std::string& name)
    : GameObject(name),
      health(100.0f),
      maxHealth(100.0f),
      armor(0.0f),
      maxArmor(50.0f),
      showHealthBar(true),
      damageFlashTimer(0.0f),
      originalColor(0.2f, 0.6f, 1.0f),  // Blue color for player
      damageColor(1.0f, 0.0f, 0.0f),    // Red for damage
      isFlashing(false),
      healthRegenRate(5.0f),    // 5 health per second
      armorRegenRate(2.0f),     // 2 armor per second
      lastDamageTime(0.0f),
      regenDelay(3.0f) {        // 3 seconds before regeneration starts
    
    // Set entity flag to true for player
    setEntity(true);
}

bool Player::initialize() {
    if (isInitialized) return true;
    
    std::cout << "=== INITIALIZING PLAYER: " << getName() << " ===" << std::endl;
    
    // Create health bar
    if (showHealthBar) {
        createHealthBar();
        std::cout << "Health bar created for player: " << getName() << std::endl;
    }
    
    isInitialized = true;
    std::cout << "Player initialized successfully: " << getName() << std::endl;
    std::cout << "=== END INITIALIZATION ===" << std::endl;
    return true;
}

void Player::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update visual effects
    updateVisualEffects(deltaTime);
    
    // Update regeneration
    updateRegeneration(deltaTime);
    
    // Update health bar
    updateHealthBar();
    
    // Call base class update
    GameObject::update(deltaTime);
}

void Player::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized) return;
    
    // Skip rendering if player is dead
    if (isDead()) return;
    
    // Set color based on damage flash
    Vec3 finalColor = getCurrentColor();
    setColor(finalColor);
    
    // Render the player using the basic renderer
    GameObject::render(renderer, camera);
    
    // Render health bar if available
    if (healthBar && showHealthBar && healthBar->isVisible()) {
        healthBar->render(renderer, camera);
    }
}

void Player::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up Player: " << getName() << std::endl;
    
    GameObject::cleanup();
}

void Player::takeDamage(float damage, GameObject* attacker) {
    if (isDead()) {
        std::cout << "Player " << getName() << " is already dead, ignoring damage" << std::endl;
        return;
    }
    
    if (!getActive()) {
        std::cout << "Player " << getName() << " is inactive, ignoring damage" << std::endl;
        return;
    }
    
    // Apply armor reduction
    float actualDamage = damage;
    if (armor > 0.0f) {
        float armorReduction = armor * 0.5f; // Each point of armor reduces damage by 0.5
        actualDamage = std::max(0.0f, damage - armorReduction);
        
        // Reduce armor
        armor = std::max(0.0f, armor - damage * 0.25f); // Armor degrades when hit
    }
    
    health = std::max(0.0f, health - actualDamage);
    flashDamage();
    lastDamageTime = 0.0f; // Reset regeneration timer
    
    // Update health bar immediately
    if (healthBar && showHealthBar) {
        try {
            healthBar->setHealth(health, maxHealth);
        } catch (const std::exception& e) {
            std::cout << "Error updating player health bar: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Player " << getName() << " took " << actualDamage << " damage (from " << damage << "). Health: " << health << "/" << maxHealth << std::endl;
    if (armor > 0.0f) {
        std::cout << "Armor: " << armor << "/" << maxArmor << std::endl;
    }
    
    // If health reaches 0, die
    if (health <= 0.0f) {
        std::cout << "Player " << getName() << " health reached 0, calling die()..." << std::endl;
        die();
    }
    
    // Call damage callback
    onDamage(actualDamage, attacker);
}

void Player::heal(float amount) {
    if (isDead()) return;
    
    float oldHealth = health;
    health = std::min(maxHealth, health + amount);
    float actualHeal = health - oldHealth;
    
    if (actualHeal > 0.0f) {
        std::cout << "Player " << getName() << " healed for " << actualHeal << " health. Health: " << health << "/" << maxHealth << std::endl;
        
        // Update health bar
        if (healthBar && showHealthBar) {
            try {
                healthBar->setHealth(health, maxHealth);
            } catch (const std::exception& e) {
                std::cout << "Error updating player health bar: " << e.what() << std::endl;
            }
        }
        
        // Call heal callback
        onHeal(actualHeal);
    }
}

void Player::addArmor(float amount) {
    armor = std::min(maxArmor, armor + amount);
    std::cout << "Player " << getName() << " gained " << amount << " armor. Armor: " << armor << "/" << maxArmor << std::endl;
}

void Player::die() {
    if (isDead()) return;
    
    std::cout << "=== PLAYER DEATH ===" << std::endl;
    std::cout << "Player: " << getName() << std::endl;
    
    // Set health to 0
    health = 0.0f;
    
    // Mark player as inactive
    setActive(false);
    
    // HEALTH BAR CLEANUP: Use safer approach
    if (healthBar) {
        std::cout << "Player " << getName() << " died! Cleaning up health bar..." << std::endl;
        
        try {
            // Mark health bar as inactive so it won't render
            healthBar->setActive(false);
            std::cout << "Health bar marked as inactive" << std::endl;
            
            // Clear the health bar reference to prevent further updates
            healthBar.reset();
            std::cout << "Health bar reference cleared" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error cleaning up health bar: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Player " << getName() << " died!" << std::endl;
    
    // Call death callback with error handling
    std::cout << "Calling onDeath callback..." << std::endl;
    try {
        onDeath();
        std::cout << "onDeath callback completed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error in onDeath callback: " << e.what() << std::endl;
    }
    
    std::cout << "=== PLAYER DEATH COMPLETED ===" << std::endl;
}

void Player::setHealth(float newHealth) {
    health = std::max(0.0f, std::min(maxHealth, newHealth));
    
    if (health <= 0.0f && !isDead()) {
        die();
    }
}

void Player::setArmor(float newArmor) {
    armor = std::max(0.0f, std::min(maxArmor, newArmor));
}

void Player::createHealthBar() {
    if (healthBar) return; // Already created
    
    std::cout << "Creating health bar for player: " << getName() << std::endl;
    
    std::string healthBarName = getName() + "_HealthBar";
    healthBar = std::make_unique<HealthBar>(healthBarName, this);
    
    // Configure health bar
    healthBar->setHealth(health, maxHealth);
    healthBar->setBarSize(6.0f, 1.0f); // Larger health bar for player
    healthBar->setOffsetY(5.0f); // Position higher above player
    healthBar->setBackgroundColor(Vec3(0.2f, 0.2f, 0.2f));
    healthBar->setBorderColor(Vec3(1.0f, 1.0f, 1.0f));
    healthBar->setHealthTransitionSpeed(10.0f);
    
    // Initialize the health bar
    if (!healthBar->initialize()) {
        std::cerr << "Failed to initialize health bar for player: " << getName() << std::endl;
        healthBar.reset();
        return;
    }
    
    std::cout << "Health bar created for player: " << getName() << std::endl;
}

void Player::updateHealthBar() {
    if (!healthBar || !showHealthBar) return;
    
    // Update health bar with current health
    healthBar->setHealth(health, maxHealth);
    
    // Make health bar pulse when health is low
    float healthPercentage = getHealthPercentage();
    if (healthPercentage < 0.3f) {
        healthBar->setPulsing(true);
    } else {
        healthBar->setPulsing(false);
    }
}

void Player::flashDamage() {
    isFlashing = true;
    damageFlashTimer = 0.5f; // Flash for 0.5 seconds
    std::cout << "Player " << getName() << " flashing damage for " << damageFlashTimer << " seconds" << std::endl;
}

void Player::updateVisualEffects(float deltaTime) {
    updateDamageFlash(deltaTime);
}

Vec3 Player::getCurrentColor() const {
    if (isFlashing) {
        return damageColor;
    }
    return originalColor;
}

void Player::updateDamageFlash(float deltaTime) {
    if (isFlashing) {
        damageFlashTimer -= deltaTime;
        if (damageFlashTimer <= 0.0f) {
            isFlashing = false;
            std::cout << "Player " << getName() << " damage flash ended" << std::endl;
        }
    }
}

void Player::updateRegeneration(float deltaTime) {
    if (isDead()) return;
    
    lastDamageTime += deltaTime;
    
    // Only regenerate if enough time has passed since last damage
    if (lastDamageTime >= regenDelay) {
        // Regenerate health
        if (health < maxHealth) {
            float healthRegen = healthRegenRate * deltaTime;
            heal(healthRegen);
        }
        
        // Regenerate armor
        if (armor < maxArmor) {
            float armorRegen = armorRegenRate * deltaTime;
            addArmor(armorRegen);
        }
    }
}

void Player::onDamage(float damage, GameObject* attacker) {
    // Override point for custom damage behavior
    std::cout << "Player took damage from: " << (attacker ? attacker->getName() : "Unknown") << std::endl;
}

void Player::onDeath() {
    // Override point for custom death behavior
    std::cout << "=== onDeath() CALLBACK STARTING ===" << std::endl;
    std::cout << "Player " << getName() << " died!" << std::endl;
    std::cout << "=== onDeath() CALLBACK COMPLETED ===" << std::endl;
}

void Player::onHeal(float amount) {
    // Override point for custom heal behavior
    std::cout << "Player healed for: " << amount << " health" << std::endl;
}

} // namespace Engine
