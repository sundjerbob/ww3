/**
 * ShootingSystem.h - Modular Shooting System Integration
 * 
 * OVERVIEW:
 * Integrates the projectile system with the weapon system to provide
 * a complete shooting mechanics solution within the GameObject-Renderer structure.
 * 
 * FEATURES:
 * - Weapon-specific projectile configurations
 * - Automatic projectile spawning and management
 * - Recoil and accuracy systems
 * - Ammunition management
 * - Fire rate control
 * - Spread and accuracy calculations
 * - Multi-shot capabilities (burst, auto, etc.)
 */

#pragma once
#include "Projectile.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Engine {

// Forward declarations
class Camera;
class Input;
class Weapon;
class Projectile;

/**
 * FireMode - Different firing modes for weapons
 */
enum class FireMode {
    Single,     // Single shot per trigger pull
    Burst,      // Multiple shots in quick succession
    Auto,       // Continuous firing while trigger held
    SemiAuto    // Semi-automatic with fire rate limit
};

/**
 * AmmoType - Different ammunition types
 */
enum class AmmoType {
    Bullet,     // Standard bullets
    Rocket,     // Rockets/missiles
    Energy,     // Energy cells
    Arrow,      // Arrows/bolts
    Grenade,    // Grenades
    Custom      // Custom ammo type
};

/**
 * WeaponStats - Weapon-specific statistics
 */
struct WeaponStats {
    // Firing properties
    FireMode fireMode = FireMode::Single;
    float fireRate = 1.0f;           // Shots per second
    float burstCount = 3;            // Shots per burst
    float spread = 0.0f;             // Base spread in degrees
    float recoil = 0.0f;             // Recoil force
    float accuracy = 1.0f;           // Base accuracy (0.0-1.0)
    
    // Ammunition
    AmmoType ammoType = AmmoType::Bullet;
    int maxAmmo = 30;
    int currentAmmo = 30;
    int maxReserveAmmo = 90;
    int currentReserveAmmo = 90;
    bool infiniteAmmo = false;
    
    // Projectile configuration
    ProjectileType projectileType = ProjectileType::Bullet;
    ProjectileConfig projectileConfig;
    
    // Reload properties
    float reloadTime = 2.0f;
    bool reloadInProgress = false;
    float reloadTimer = 0.0f;
    
    // Cooldown properties
    float lastFireTime = 0.0f;
    float cooldownTime = 0.0f;
    
    // Custom callbacks - using void* to avoid circular dependencies
    std::function<void(void*, Projectile*)> onProjectileFired = nullptr;
    std::function<void(void*)> onReloadStart = nullptr;
    std::function<void(void*)> onReloadComplete = nullptr;
    std::function<void(void*)> onAmmoEmpty = nullptr;
};

/**
 * ShootingSystem - Main shooting system class
 */
class ShootingSystem {
private:
    // Core systems
    ProjectileManager* projectileManager;
    Camera* playerCamera;
    Input* input;
    
    // Weapon integration
    Weapon* currentWeapon;
    WeaponStats weaponStats;
    
    // Shooting state
    bool isFiring;
    bool triggerPressed;
    float fireTimer;
    int burstShotsFired;
    
    // Recoil system
    Vec3 currentRecoil;
    Vec3 recoilRecovery;
    Vec3 recoilPattern;      // Current recoil pattern (x, y, z offsets)
    Vec3 recoilVelocity;     // Current recoil velocity
    float recoilTimer;
    float recoilRecoveryRate;
    float maxRecoil;
    
    // Recoil callbacks for visual feedback
    std::function<void(const Vec3&)> onRecoilApplied;
    
    // Spread system
    float currentSpread;
    float spreadRecovery;
    float spreadTimer;

public:
    // Constructor/Destructor
    ShootingSystem();
    ~ShootingSystem() = default;
    
    // Core functionality
    void initialize(ProjectileManager* projectiles, Camera* camera, Input* input);
    void update(float deltaTime);
    void cleanup();
    void attachToWeapon(Weapon* weapon);
    
    // Weapon management
    void setWeapon(Weapon* weapon);
    Weapon* getWeapon() const { return currentWeapon; }
    void configureWeapon(const WeaponStats& stats);
    const WeaponStats& getWeaponStats() const { return weaponStats; }
    void setProjectileManager(ProjectileManager* manager);
    
    // Shooting control
    void startFiring();
    void stopFiring();
    void fireSingleShot();
    void fireBurst();
    void fireAuto(float deltaTime);
    
    // Ammunition management
    bool hasAmmo() const;
    bool canFire() const;
    void consumeAmmo(int amount = 1);
    void addAmmo(int amount);
    void reload();
    void cancelReload();
    
    // Recoil and accuracy
    void applyRecoil();
    void updateRecoil(float deltaTime);
    Vec3 getRecoilOffset() const;
    void setRecoilCallback(std::function<void(const Vec3&)> callback);
    Vec3 calculateSpread(const Vec3& baseDirection);
    void updateSpread(float deltaTime);
    
    // Projectile spawning
    Projectile* spawnProjectile(const Vec3& position, const Vec3& direction);
    void fireProjectile(const Vec3& position, const Vec3& direction);
    
    // Utility
    Vec3 getFireDirection() const;
    Vec3 getFirePosition() const;
    float getFireRate() const { return weaponStats.fireRate; }
    float getSpread() const { return currentSpread; }
    Vec3 getRecoil() const { return currentRecoil; }
    
    // State queries
    bool isReloading() const { return weaponStats.reloadInProgress; }
    bool getIsFiring() const { return isFiring; }
    int getCurrentAmmo() const { return weaponStats.currentAmmo; }
    int getReserveAmmo() const { return weaponStats.currentReserveAmmo; }
    
protected:
    // Internal helpers
    void updateFireTimer(float deltaTime);
    void updateReloadTimer(float deltaTime);
    void resetFireTimer();
    bool checkFireCooldown() const;
    void handleAmmoEmpty();
    void handleReloadComplete();
};

/**
 * WeaponShootingComponent - Component that can be added to weapons
 */
class WeaponShootingComponent {
private:
    ShootingSystem shootingSystem;
    bool isEnabled;

public:
    WeaponShootingComponent();
    ~WeaponShootingComponent() = default;
    
    // Component interface
    void initialize(ProjectileManager* projectiles, Camera* camera, Input* input);
    void update(float deltaTime);
    void cleanup();
    
    // Configuration
    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }
    
    // Weapon integration
    void attachToWeapon(Weapon* weapon);
    void configureWeapon(const WeaponStats& stats);
    ShootingSystem* getShootingSystem() { return &shootingSystem; }
    const ShootingSystem* getShootingSystem() const { return &shootingSystem; }
    
    // Shooting interface
    void startFiring();
    void stopFiring();
    void fireSingleShot();
    
    // Ammunition interface
    bool hasAmmo() const;
    bool canFire() const;
    void reload();
    int getCurrentAmmo() const;
    int getReserveAmmo() const;
    
    // State queries
    bool isReloading() const;
    bool isFiring() const;
};

/**
 * WeaponPreset - Predefined weapon configurations
 */
struct WeaponPreset {
    std::string name;
    WeaponStats stats;
    
    // Constructor for easy preset creation
    WeaponPreset(const std::string& presetName, const WeaponStats& weaponStats)
        : name(presetName), stats(weaponStats) {}
};

/**
 * WeaponPresetFactory - Factory for creating weapon presets
 */
class WeaponPresetFactory {
public:
    // Assault rifle presets
    static WeaponPreset createAssaultRiflePreset();
    static WeaponPreset createSniperRiflePreset();
    static WeaponPreset createSubmachineGunPreset();
    static WeaponPreset createPistolPreset();
    static WeaponPreset createShotgunPreset();
    
    // Special weapon presets
    static WeaponPreset createRocketLauncherPreset();
    static WeaponPreset createLaserRiflePreset();
    static WeaponPreset createPlasmaRiflePreset();
    static WeaponPreset createBowPreset();
    static WeaponPreset createCrossbowPreset();
    
    // Custom preset creation
    static WeaponPreset createCustomPreset(const std::string& name, 
                                          ProjectileType projectileType,
                                          FireMode fireMode,
                                          float fireRate,
                                          int maxAmmo);
};

} // namespace Engine
