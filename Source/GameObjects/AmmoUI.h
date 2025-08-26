/**
 * AmmoUI.h - Fixed UI Element for Displaying Current Ammunition
 * 
 * OVERVIEW:
 * A UI GameObject that displays current ammunition information in a fixed
 * position on screen. Shows current ammo, reserve ammo, and reload status.
 * 
 * FEATURES:
 * - Fixed screen position (bottom-right corner)
 * - Real-time ammunition updates from weapon system
 * - Visual feedback for low ammo and reloading states
 * - Clean, modern UI design
 * - Integration with existing weapon system
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ShootingSystem.h"
#include <string>

namespace Engine {

// Forward declarations
class Weapon;
class WeaponShootingComponent;

/**
 * AmmoUI - Fixed UI Element for Ammunition Display
 * 
 * Features:
 * - Displays current and reserve ammunition
 * - Shows reload status and progress
 * - Visual indicators for low ammo
 * - Fixed screen positioning
 * - Real-time updates from weapon system
 */
class AmmoUI : public GameObject {
private:
    // UI positioning and styling
    Vec2 screenPosition;      // Position on screen (normalized coordinates)
    Vec2 size;                // Size of the UI element
    Vec3 textColor;           // Color of the text
    Vec3 backgroundColor;     // Color of the background
    Vec3 lowAmmoColor;        // Color when ammo is low
    Vec3 reloadColor;         // Color when reloading
    
    // Ammunition data
    int currentAmmo;
    int maxAmmo;
    int reserveAmmo;
    int maxReserveAmmo;
    bool isReloading;
    float reloadProgress;
    
    // Visual state
    bool isVisible;
    float lowAmmoThreshold;   // Percentage threshold for low ammo warning
    float pulseTimer;         // For pulsing effects
    float pulseSpeed;         // Speed of pulse animation
    
    // Weapon reference
    Weapon* weapon;
    WeaponShootingComponent* shootingComponent;
    
    // UI text strings
    std::string ammoText;
    std::string reserveText;
    std::string statusText;
    
public:
    // Constructor/Destructor
    AmmoUI(const std::string& name = "AmmoUI");
    virtual ~AmmoUI() = default;
    
    // UI configuration
    void setScreenPosition(const Vec2& position) { screenPosition = position; }
    Vec2 getScreenPosition() const { return screenPosition; }
    
    void setSize(const Vec2& uiSize) { size = uiSize; }
    Vec2 getSize() const { return size; }
    
    void setTextColor(const Vec3& color) { textColor = color; }
    Vec3 getTextColor() const { return textColor; }
    
    void setBackgroundColor(const Vec3& color) { backgroundColor = color; }
    Vec3 getBackgroundColor() const { return backgroundColor; }
    
    void setLowAmmoColor(const Vec3& color) { lowAmmoColor = color; }
    Vec3 getLowAmmoColor() const { return lowAmmoColor; }
    
    void setReloadColor(const Vec3& color) { reloadColor = color; }
    Vec3 getReloadColor() const { return reloadColor; }
    
    void setLowAmmoThreshold(float threshold) { lowAmmoThreshold = threshold; }
    float getLowAmmoThreshold() const { return lowAmmoThreshold; }
    
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }
    
    // Weapon integration
    void setWeapon(Weapon* weaponRef);
    Weapon* getWeapon() const { return weapon; }
    
    void setShootingComponent(WeaponShootingComponent* component);
    WeaponShootingComponent* getShootingComponent() const { return shootingComponent; }
    
    // Ammunition data access
    int getCurrentAmmo() const { return currentAmmo; }
    int getMaxAmmo() const { return maxAmmo; }
    int getReserveAmmo() const { return reserveAmmo; }
    int getMaxReserveAmmo() const { return maxReserveAmmo; }
    bool getIsReloading() const { return isReloading; }
    float getReloadProgress() const { return reloadProgress; }
    
    // Text access
    const std::string& getAmmoText() const { return ammoText; }
    const std::string& getReserveText() const { return reserveText; }
    const std::string& getStatusText() const { return statusText; }
    
    // Visual state queries
    bool isLowAmmo() const;
    bool shouldPulse() const;
    float getPulseAlpha() const;
    
    // Override GameObject methods
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;
    virtual RendererType getPreferredRendererType() const override { return RendererType::Text; }
    
    // Custom model matrix calculation for proper NDC positioning
    Mat4 getModelMatrix() const;

private:
    // Internal methods
    void setupMesh() override;
    void updateAmmunitionData();
    void updateTextStrings();
    void updateVisualState(float deltaTime);
    Vec3 getCurrentTextColor() const;
    Vec3 getCurrentBackgroundColor() const;
};

} // namespace Engine
