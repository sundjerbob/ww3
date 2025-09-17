/**
 * Player.h - Player GameObject with health and damage system
 * 
 * OVERVIEW:
 * Represents the player character with health, damage handling, and interaction with monsters.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Math/Math.h"
#include <memory>

namespace Engine {

// Forward declarations
// class HealthBar;  // REMOVED: Using new texture-based health bar system
class TextureHealthBar;

/**
 * Player - Main player character with health system
 */
class Player : public GameObject {
private:
    // Player properties
    float health;
    float maxHealth;
    float armor;
    float maxArmor;
    
    // Health bar - NEW: Using texture-based system
    std::unique_ptr<TextureHealthBar> textureHealthBar;
    bool showHealthBar;
    
    // Damage effects
    float damageFlashTimer;
    Vec3 originalColor;
    Vec3 damageColor;
    bool isFlashing;
    
    // Regeneration
    float healthRegenRate;
    float armorRegenRate;
    float lastDamageTime;
    float regenDelay; // Time before regeneration starts after taking damage

public:
    // Constructor/Destructor
    Player(const std::string& name = "Player");
    virtual ~Player() = default;
    
    // Core functionality
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;
    
    // Health and damage
    void takeDamage(float damage, GameObject* attacker = nullptr);
    void heal(float amount);
    void addArmor(float amount);
    void die();
    
    // Health management
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercentage() const { return health / maxHealth; }
    void setHealth(float newHealth);
    
    // Armor management
    float getArmor() const { return armor; }
    float getMaxArmor() const { return maxArmor; }
    float getArmorPercentage() const { return armor / maxArmor; }
    void setArmor(float newArmor);
    
    // Health bar management - NEW: Using texture-based system
    void setShowHealthBar(bool show) { showHealthBar = show; }
    bool getShowHealthBar() const { return showHealthBar; }
    TextureHealthBar* getHealthBar() const { return textureHealthBar.get(); }
    void createHealthBar();
    void updateHealthBar();
    
    // Status checks
    bool isAlive() const { return health > 0.0f; }
    bool isDead() const { return health <= 0.0f; }
    
    // Visual effects
    void flashDamage();
    void updateVisualEffects(float deltaTime);
    Vec3 getCurrentColor() const;

protected:
    // Override points for custom behavior
    virtual void onDamage(float damage, GameObject* attacker);
    virtual void onDeath();
    virtual void onHeal(float amount);
    
    // Internal helpers
    void updateDamageFlash(float deltaTime);
    void updateRegeneration(float deltaTime);
};

} // namespace Engine
