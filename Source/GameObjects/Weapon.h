/**
 * Weapon.h - 3D Weapon GameObject for FPS-style weapon rendering
 * 
 * OVERVIEW:
 * A weapon object that loads 3D weapon models from OBJ files and renders them
 * in a fixed position on screen, pointing towards the crosshair.
 * Similar to Counter-Strike weapon rendering system.
 * 
 * FEATURES:
 * - OBJ model loading with automatic scaling and positioning
 * - Fixed screen position (bottom-right corner)
 * - Automatic rotation to point towards crosshair
 * - Support for different weapon types
 * - Proper weapon orientation and scaling
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Core/ShootingSystem.h"
#include "../Engine/Math/Camera.h"
#include "../Engine/Rendering/Material.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Engine {

// Forward declarations
class WeaponShootingComponent;
struct WeaponStats;
struct OBJMeshData;

/**
 * Weapon - 3D Weapon GameObject for FPS-style rendering
 * 
 * Features:
 * - Loads weapon models from OBJ files
 * - Renders in fixed screen position
 * - Points towards crosshair direction
 * - Automatic scaling and positioning
 * - Support for weapon switching
 * - Integrated shooting system
 */
class Weapon : public GameObject {
private:
    // Weapon properties
    std::string weaponModelPath;
    Vec3 weaponColor;
    float weaponScale;
    Vec3 screenPosition;  // Legacy screen position (kept for compatibility)
    Vec3 weaponOffset;    // Legacy offset (kept for compatibility)
    
    // 3D World-space positioning
    Vec3 worldSpaceOffset;  // 3D offset from camera position in world space
    Vec3 barrelTipOffset;   // Offset from weapon origin to barrel tip
    
    // Weapon state
    bool isLoaded;
    bool isVisible;
    
    // Recoil system
    Vec3 recoilOffset;           // Position-based recoil (limited)
    Vec3 recoilVelocity;         // Position recoil velocity
    float recoilRecoveryRate;    // Recovery rate for position recoil
    
    // Rotation-based recoil (unlimited)
    Vec3 recoilRotation;         // Rotation recoil (unlimited upward tilt)
    Vec3 recoilRotationVelocity; // Rotation recoil velocity
    float rotationRecoveryRate;  // Recovery rate for rotation recoil
    
    // Recoil limits
    float maxPositionRecoil;     // Maximum upward position recoil
    
    // Recoil callback
    std::function<void(const Vec3&)> onRecoilApplied;
    
    // Camera reference for aiming
    Camera* playerCamera;
    
    // Weapon-specific properties
    float aimSensitivity;  // How much the weapon rotates with mouse movement
    Vec3 defaultRotation;  // Default weapon orientation
    
    // Material system
    MaterialLibrary weaponMaterials; // Materials loaded from .mtl file
    
    // Material groups for multi-material rendering
    struct MaterialGroup {
        std::string materialName;
        std::vector<unsigned int> indices; // Triangle indices for this material
        Vec3 color; // Material color
    };
    std::vector<MaterialGroup> materialGroups;
    
    // Weapon switching system
    struct WeaponData {
        std::string name;
        std::string modelPath;
        Vec3 color = Vec3(0.8f, 0.8f, 0.8f);
        float scale = 1.0f;
        Vec3 offset = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 defaultRotation = Vec3(0.0f, 0.0f, 0.0f);
        float aimSensitivity = 1.0f;
        WeaponStats shootingStats; // Added shooting stats
    };
    std::vector<WeaponData> weaponInventory;
    int currentWeaponIndex;
    
    // Shooting system integration
    WeaponShootingComponent shootingComponent;
    bool shootingEnabled;

public:
    // Constructor/Destructor
    Weapon(const std::string& name = "Weapon", 
           const std::string& modelPath = "",
           const Vec3& color = Vec3(0.8f, 0.8f, 0.8f));
    virtual ~Weapon() = default;
    
    // Weapon-specific methods
    bool loadWeaponModel(const std::string& modelPath);
    void setWeaponColor(const Vec3& color) { weaponColor = color; }
    Vec3 getWeaponColor() const { return weaponColor; }
    
    void setWeaponScale(float scale) { weaponScale = scale; }
    float getWeaponScale() const { return weaponScale; }
    
    void setScreenPosition(const Vec3& position) { screenPosition = position; }
    Vec3 getScreenPosition() const { return screenPosition; }
    
    void setWeaponOffset(const Vec3& offset) { weaponOffset = offset; }
    Vec3 getWeaponOffset() const { return weaponOffset; }
    
    // World position calculation (legacy method)
    Vec3 getWorldPosition() const;
    
    // 3D World-space positioning methods
    void updateWorldPosition();           // Update weapon position based on camera
    Vec3 getBarrelTipPosition() const;    // Get precise barrel tip world position
    void setWorldSpaceOffset(const Vec3& offset) { worldSpaceOffset = offset; }
    Vec3 getWorldSpaceOffset() const { return worldSpaceOffset; }
    void setBarrelTipOffset(const Vec3& offset) { barrelTipOffset = offset; }
    Vec3 getBarrelTipOffset() const { return barrelTipOffset; }
    
    void setAimSensitivity(float sensitivity) { aimSensitivity = sensitivity; }
    float getAimSensitivity() const { return aimSensitivity; }
    
    void setDefaultRotation(const Vec3& rotation) { defaultRotation = rotation; }
    Vec3 getDefaultRotation() const { return defaultRotation; }
    
    void setPlayerCamera(Camera* camera) { playerCamera = camera; }
    Camera* getPlayerCamera() const { return playerCamera; }
    
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }
    
    bool isModelLoaded() const { return isLoaded; }
    
    // Weapon switching methods
    void initializeWeaponInventory();
    bool switchToWeapon(int weaponIndex);
    bool switchToWeapon(const std::string& weaponName);
    int getCurrentWeaponIndex() const { return currentWeaponIndex; }
    const std::string& getCurrentWeaponName() const;
    int getWeaponCount() const { return static_cast<int>(weaponInventory.size()); }
    void cycleToNextWeapon();
    void cycleToPreviousWeapon();
    
    // Material access
    const MaterialLibrary& getWeaponMaterials() const { return weaponMaterials; }
    const std::vector<MaterialGroup>& getMaterialGroups() const { return materialGroups; }
    
    // Shooting system integration
    void enableShooting(bool enabled) { shootingEnabled = enabled; }
    bool isShootingEnabled() const { return shootingEnabled; }
    const WeaponShootingComponent& getShootingComponent() const { return shootingComponent; }
    WeaponShootingComponent& getShootingComponent() { return shootingComponent; }
    
    // Shooting interface
    void startFiring();
    void stopFiring();
    void fireSingleShot();
    void fireMonsterHunterShot();
    bool canFire() const;
    bool hasAmmo() const;
    void reload();
    int getCurrentAmmo() const;
    int getReserveAmmo() const;
    bool isReloading() const;
    bool isFiring() const;
    
    // Weapon configuration
    void configureShooting(const WeaponStats& stats);
    const WeaponStats& getShootingStats() const;
    void setProjectileManager(ProjectileManager* manager);
    
    // Recoil methods
    void applyRecoil(const Vec3& recoil);
    void updateRecoil(float deltaTime);
    Vec3 getRecoilOffset() const { return recoilOffset; }
    Vec3 getRecoilRotation() const { return recoilRotation; }
    
    // Recoil callback system
    void setRecoilCallback(std::function<void(const Vec3&)> callback) { onRecoilApplied = callback; }
    
    // Override GameObject methods
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;

private:
    // Internal methods
    void setupMesh();
    void updateWeaponPosition();
    void updateWeaponRotation();
    Vec3 calculateAimDirection() const;
    Mat4 createWeaponTransformMatrix() const;
    void createMaterialGroups(const OBJMeshData& objData);
    
    // Shooting system helpers
    void initializeShootingSystem();
    void updateShootingSystem(float deltaTime);
    Vec3 getFirePosition() const;
    Vec3 getFireDirection() const;
};

} // namespace Engine
