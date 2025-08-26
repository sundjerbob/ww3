/**
 * ShootingSystem.cpp - Implementation of Modular Shooting System
 */

#include "ShootingSystem.h"
#include "../GameObjects/Weapon.h"
#include "../Math/Camera.h"
#include <iostream>
#include <algorithm>

namespace Engine {

// ShootingSystem implementation
ShootingSystem::ShootingSystem()
    : projectileManager(nullptr),
      playerCamera(nullptr),
      input(nullptr),
      currentWeapon(nullptr),
      isFiring(false),
      triggerPressed(false),
      fireTimer(0.0f),
      burstShotsFired(0),
      currentRecoil(0.0f, 0.0f, 0.0f),
      recoilRecovery(0.0f, 0.0f, 0.0f),
      recoilPattern(0.0f, 0.0f, 0.0f),
      recoilVelocity(0.0f, 0.0f, 0.0f),
      recoilTimer(0.0f),
      recoilRecoveryRate(2.0f),
      maxRecoil(0.5f),
      currentSpread(0.0f),
      spreadRecovery(0.0f),
      spreadTimer(0.0f) {
}

void ShootingSystem::initialize(ProjectileManager* projectiles, Camera* camera, Input* input) {
    projectileManager = projectiles;
    playerCamera = camera;
    this->input = input;
}

void ShootingSystem::update(float deltaTime) {
    // Update fire timer
    updateFireTimer(deltaTime);
    
    // Update reload timer
    updateReloadTimer(deltaTime);
    
    // Update recoil
    updateRecoil(deltaTime);
    
    // Update spread
    updateSpread(deltaTime);
    
    // Handle firing logic
    if (isFiring) {
        std::cout << "=== FIRING LOGIC DEBUG ===" << std::endl;
        std::cout << "isFiring: " << (isFiring ? "true" : "false") << std::endl;
        std::cout << "canFire(): " << (canFire() ? "true" : "false") << std::endl;
        std::cout << "hasAmmo(): " << (hasAmmo() ? "true" : "false") << std::endl;
        std::cout << "reloadInProgress: " << (weaponStats.reloadInProgress ? "true" : "false") << std::endl;
        std::cout << "checkFireCooldown(): " << (checkFireCooldown() ? "true" : "false") << std::endl;
        std::cout << "fireTimer: " << fireTimer << std::endl;
        std::cout << "fireRate: " << weaponStats.fireRate << std::endl;
        
        if (canFire()) {
            // Fire and reset timer for next shot
            fireSingleShot();
            resetFireTimer();
        }
    }
}

void ShootingSystem::cleanup() {
    // Cleanup implementation
}

void ShootingSystem::setWeapon(Weapon* weapon) {
    currentWeapon = weapon;
}

void ShootingSystem::configureWeapon(const WeaponStats& stats) {
    weaponStats = stats;
    std::cout << "=== WEAPON CONFIGURED ===" << std::endl;
    std::cout << "Current Ammo: " << weaponStats.currentAmmo << "/" << weaponStats.maxAmmo << std::endl;
    std::cout << "Reserve Ammo: " << weaponStats.currentReserveAmmo << "/" << weaponStats.maxReserveAmmo << std::endl;
    std::cout << "Recoil: " << weaponStats.recoil << std::endl;
    std::cout << "Fire Rate: " << weaponStats.fireRate << std::endl;
}

void ShootingSystem::setProjectileManager(ProjectileManager* manager) {
    projectileManager = manager;
}

void ShootingSystem::startFiring() {
    isFiring = true;
    triggerPressed = true;
    // Set fire timer to cooldown threshold so first shot can fire immediately
    fireTimer = 1.0f / weaponStats.fireRate;
}

void ShootingSystem::stopFiring() {
    isFiring = false;
    triggerPressed = false;
}

void ShootingSystem::fireSingleShot() {
    if (!canFire()) return;
    
    // Fire logic implementation
    std::cout << "Firing single shot" << std::endl;
    
    // Consume ammo
    consumeAmmo();
    
    // Apply recoil
    applyRecoil();
    
    // Spawn and fire projectile
    std::cout << "=== PROJECTILE DEBUG ===" << std::endl;
    std::cout << "projectileManager: " << (projectileManager ? "valid" : "null") << std::endl;
    std::cout << "currentWeapon: " << (currentWeapon ? "valid" : "null") << std::endl;
    
    if (projectileManager && currentWeapon) {
        Vec3 firePos = getFirePosition();
        Vec3 fireDir = getFireDirection();
        fireProjectile(firePos, fireDir);
    } else {
        std::cout << "Cannot fire projectile - missing manager or weapon" << std::endl;
    }
}

void ShootingSystem::fireBurst() {
    // Burst fire implementation
}

void ShootingSystem::fireAuto(float deltaTime) {
    // Auto fire implementation
}

bool ShootingSystem::hasAmmo() const {
    return weaponStats.infiniteAmmo || weaponStats.currentAmmo > 0;
}

bool ShootingSystem::canFire() const {
    if (!hasAmmo()) return false;
    if (weaponStats.reloadInProgress) return false;
    if (!checkFireCooldown()) return false;
    return true;
}

void ShootingSystem::consumeAmmo(int amount) {
    if (weaponStats.infiniteAmmo) return;
    weaponStats.currentAmmo = std::max(0, weaponStats.currentAmmo - amount);
    
    if (weaponStats.currentAmmo == 0) {
        handleAmmoEmpty();
    }
}

void ShootingSystem::addAmmo(int amount) {
    weaponStats.currentReserveAmmo = std::min(
        weaponStats.maxReserveAmmo,
        weaponStats.currentReserveAmmo + amount
    );
}

void ShootingSystem::reload() {
    if (weaponStats.reloadInProgress) return;
    if (weaponStats.currentAmmo == weaponStats.maxAmmo) return;
    if (weaponStats.currentReserveAmmo == 0) return;
    
    weaponStats.reloadInProgress = true;
    weaponStats.reloadTimer = 0.0f;
    
    if (weaponStats.onReloadStart) {
        weaponStats.onReloadStart(currentWeapon);
    }
}

void ShootingSystem::cancelReload() {
    weaponStats.reloadInProgress = false;
    weaponStats.reloadTimer = 0.0f;
}

void ShootingSystem::applyRecoil() {
    // Calculate recoil pattern (upward and slightly random)
    float recoilForce = weaponStats.recoil;
    std::cout << "=== APPLYING RECOIL ===" << std::endl;
    std::cout << "Recoil Force: " << recoilForce << std::endl;
    std::cout << "Current Recoil Pattern: (" << recoilPattern.x << ", " << recoilPattern.y << ", " << recoilPattern.z << ")" << std::endl;
    
    float randomX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.3f; // Random horizontal recoil
    float randomZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f; // Random forward/backward recoil
    
    // Apply recoil to pattern
    recoilPattern.y += recoilForce * 0.8f;  // Upward recoil (80% of force)
    recoilPattern.x += recoilForce * randomX; // Random horizontal recoil
    recoilPattern.z += recoilForce * randomZ; // Random forward/backward recoil
    
    // Clamp recoil to maximum
    recoilPattern.y = std::min(recoilPattern.y, maxRecoil);
    recoilPattern.x = std::clamp(recoilPattern.x, -maxRecoil * 0.5f, maxRecoil * 0.5f);
    recoilPattern.z = std::clamp(recoilPattern.z, -maxRecoil * 0.3f, maxRecoil * 0.3f);
    
    // Set recoil velocity for smooth movement
    recoilVelocity = recoilPattern * 10.0f; // Initial velocity
    
    // Apply to spread
    currentSpread += weaponStats.spread;
    
    // Notify visual systems of recoil
    if (onRecoilApplied) {
        std::cout << "=== RECOIL CALLBACK TRIGGERED ===" << std::endl;
        onRecoilApplied(recoilPattern);
    } else {
        std::cout << "=== RECOIL CALLBACK NOT SET ===" << std::endl;
    }
    
    std::cout << "=== RECOIL APPLIED ===" << std::endl;
    std::cout << "Recoil Pattern: (" << recoilPattern.x << ", " << recoilPattern.y << ", " << recoilPattern.z << ")" << std::endl;
}

void ShootingSystem::updateRecoil(float deltaTime) {
    // Update recoil timer
    recoilTimer += deltaTime;
    
    // Apply recoil recovery over time with much faster recovery
    float recoveryRate = recoilRecoveryRate * 6.0f; // 6x faster recovery rate
    
    if (recoilPattern.y > 0.0f) {
        recoilPattern.y -= recoveryRate * deltaTime;
        recoilPattern.y = std::max(0.0f, recoilPattern.y);
    }
    
    if (recoilPattern.x != 0.0f) {
        recoilPattern.x -= recoilPattern.x * recoveryRate * deltaTime;
        if (std::abs(recoilPattern.x) < 0.001f) recoilPattern.x = 0.0f;
    }
    
    if (recoilPattern.z != 0.0f) {
        recoilPattern.z -= recoilPattern.z * recoveryRate * deltaTime;
        if (std::abs(recoilPattern.z) < 0.001f) recoilPattern.z = 0.0f;
    }
    
    // Update recoil velocity for smooth movement - much stronger damping
    recoilVelocity *= (1.0f - deltaTime * 15.0f); // Much stronger damping for faster return
    
    // Notify visual systems of recoil changes
    if (onRecoilApplied) {
        onRecoilApplied(recoilPattern);
    }
}

Vec3 ShootingSystem::calculateSpread(const Vec3& baseDirection) {
    // Spread calculation implementation
    return baseDirection; // Simplified for now
}

void ShootingSystem::updateSpread(float deltaTime) {
    // Spread recovery implementation
    if (currentSpread > 0.0f) {
        currentSpread -= spreadRecovery * deltaTime;
        currentSpread = std::max(0.0f, currentSpread);
    }
}

Vec3 ShootingSystem::getRecoilOffset() const {
    return recoilPattern;
}

void ShootingSystem::setRecoilCallback(std::function<void(const Vec3&)> callback) {
    onRecoilApplied = callback;
}

Projectile* ShootingSystem::spawnProjectile(const Vec3& position, const Vec3& direction) {
    if (!projectileManager) return nullptr;
    
    std::cout << "=== SPAWNING PROJECTILE ===" << std::endl;
    Projectile* projectile = projectileManager->createProjectile(weaponStats.projectileConfig, "Projectile");
    if (projectile) {
        std::cout << "Projectile created successfully" << std::endl;
    } else {
        std::cout << "Failed to create projectile" << std::endl;
    }
    return projectile;
}

void ShootingSystem::fireProjectile(const Vec3& position, const Vec3& direction) {
    std::cout << "=== FIRING PROJECTILE ===" << std::endl;
    std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    
    Projectile* projectile = spawnProjectile(position, direction);
    if (projectile) {
        projectile->fire(position, direction, currentWeapon);
        std::cout << "Projectile fired successfully" << std::endl;
    } else {
        std::cout << "Failed to fire projectile" << std::endl;
    }
}

Vec3 ShootingSystem::getFireDirection() const {
    if (playerCamera) {
        return playerCamera->getForward();
    }
    return Vec3(0.0f, 0.0f, -1.0f); // Default forward direction
}

Vec3 ShootingSystem::getFirePosition() const {
    if (currentWeapon) {
        return currentWeapon->getPosition();
    }
    return Vec3(0.0f, 0.0f, 0.0f);
}

void ShootingSystem::updateFireTimer(float deltaTime) {
    fireTimer += deltaTime;
}

void ShootingSystem::updateReloadTimer(float deltaTime) {
    if (!weaponStats.reloadInProgress) return;
    
    weaponStats.reloadTimer += deltaTime;
    if (weaponStats.reloadTimer >= weaponStats.reloadTime) {
        handleReloadComplete();
    }
}

void ShootingSystem::resetFireTimer() {
    fireTimer = 0.0f;
}

bool ShootingSystem::checkFireCooldown() const {
    return fireTimer >= (1.0f / weaponStats.fireRate);
}

void ShootingSystem::handleAmmoEmpty() {
    if (weaponStats.onAmmoEmpty) {
        weaponStats.onAmmoEmpty(currentWeapon);
    }
}

void ShootingSystem::handleReloadComplete() {
    weaponStats.reloadInProgress = false;
    weaponStats.reloadTimer = 0.0f;
    
    // Transfer ammo from reserve to current
    int ammoNeeded = weaponStats.maxAmmo - weaponStats.currentAmmo;
    int ammoToTransfer = std::min(ammoNeeded, weaponStats.currentReserveAmmo);
    
    weaponStats.currentAmmo += ammoToTransfer;
    weaponStats.currentReserveAmmo -= ammoToTransfer;
    
    if (weaponStats.onReloadComplete) {
        weaponStats.onReloadComplete(currentWeapon);
    }
}

// WeaponShootingComponent implementation
WeaponShootingComponent::WeaponShootingComponent()
    : isEnabled(false) {
}

void WeaponShootingComponent::initialize(ProjectileManager* projectiles, Camera* camera, Input* input) {
    shootingSystem.initialize(projectiles, camera, input);
    isEnabled = true;
}

void WeaponShootingComponent::attachToWeapon(Weapon* weapon) {
    shootingSystem.setWeapon(weapon);
}

void WeaponShootingComponent::update(float deltaTime) {
    if (isEnabled) {
        shootingSystem.update(deltaTime);
    }
}

void WeaponShootingComponent::cleanup() {
    shootingSystem.cleanup();
}

void WeaponShootingComponent::configureWeapon(const WeaponStats& stats) {
    shootingSystem.configureWeapon(stats);
}

void WeaponShootingComponent::startFiring() {
    if (isEnabled) {
        shootingSystem.startFiring();
    }
}

void WeaponShootingComponent::stopFiring() {
    if (isEnabled) {
        shootingSystem.stopFiring();
    }
}

void WeaponShootingComponent::fireSingleShot() {
    if (isEnabled) {
        shootingSystem.fireSingleShot();
    }
}

bool WeaponShootingComponent::hasAmmo() const {
    return isEnabled && shootingSystem.hasAmmo();
}

bool WeaponShootingComponent::canFire() const {
    return isEnabled && shootingSystem.canFire();
}

void WeaponShootingComponent::reload() {
    if (isEnabled) {
        shootingSystem.reload();
    }
}

int WeaponShootingComponent::getCurrentAmmo() const {
    return isEnabled ? shootingSystem.getCurrentAmmo() : 0;
}

int WeaponShootingComponent::getReserveAmmo() const {
    return isEnabled ? shootingSystem.getReserveAmmo() : 0;
}

bool WeaponShootingComponent::isReloading() const {
    return isEnabled && shootingSystem.isReloading();
}

bool WeaponShootingComponent::isFiring() const {
    return isEnabled && shootingSystem.getIsFiring();
}

} // namespace Engine
