/**
 * ShootingSystem.cpp - Implementation of Modular Shooting System
 */

#include "ShootingSystem.h"
#include "../GameObjects/Weapon.h"
#include "../Math/Camera.h"
#include <iostream>
#include <cmath>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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
    
    // Consume ammo
    consumeAmmo();
    
    // Apply recoil
    applyRecoil();
    
    // Spawn and fire projectile
    
    if (projectileManager && currentWeapon) {
        Vec3 firePos = getFirePosition();
        Vec3 fireDir = getFireDirection();
        fireProjectile(firePos, fireDir);
    } else {
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
        onRecoilApplied(recoilPattern);
    } else {
    }
    
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
    
    
    // Use monster hunter projectile for better monster damage
    Projectile* projectile = projectileManager->createMonsterHunterProjectile("MonsterHunterProjectile");
    
    if (projectile) {
    } else {
    }
    return projectile;
}

Projectile* ShootingSystem::spawnMonsterHunterProjectile(const Vec3& position, const Vec3& direction) {
    if (!projectileManager) return nullptr;
    
    
    // Use monster hunter projectile for better monster damage
    Projectile* projectile = projectileManager->createMonsterHunterProjectile("MonsterHunterProjectile");
    
    if (projectile) {
    } else {
    }
    return projectile;
}

void ShootingSystem::fireProjectile(const Vec3& position, const Vec3& direction) {
    // Debug output (disabled for now)
    // std::cout << "=== FIRING PROJECTILE ===" << std::endl;
    // std::cout << "Fire position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    // std::cout << "Fire direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    
    Projectile* projectile = spawnProjectile(position, direction);
    if (projectile) {
        projectile->fire(position, direction, currentWeapon);
        // std::cout << "Projectile fired successfully" << std::endl;
    } else {
        // std::cout << "Failed to spawn projectile!" << std::endl;
    }
    // std::cout << "=========================" << std::endl;
}

void ShootingSystem::fireMonsterHunterProjectile(const Vec3& position, const Vec3& direction) {
    
    Projectile* projectile = spawnMonsterHunterProjectile(position, direction);
    if (projectile) {
        projectile->fire(position, direction, currentWeapon);
    } else {
    }
}

Vec3 ShootingSystem::getFireDirection() const {
    if (playerCamera && currentWeapon) {
        // Calculate the crosshair target position in world space
        // The crosshair is at the center of the screen, so we need to calculate
        // where a ray from the camera through the center of the screen would go
        
        // Get camera position and forward direction
        Vec3 cameraPos = playerCamera->getPosition();
        Vec3 cameraForward = playerCamera->getForward();
        
        // Calculate a point far ahead in the camera's forward direction
        // This represents where the crosshair is pointing in world space
        const float crosshairDistance = 100.0f; // Distance to project the crosshair target
        Vec3 crosshairTarget = cameraPos + cameraForward * crosshairDistance;
        
        // Get the gun barrel position
        Vec3 gunBarrelPos = currentWeapon->getWorldPosition();
        
        // Calculate direction from gun barrel to crosshair target
        Vec3 fireDirection = crosshairTarget - gunBarrelPos;
        fireDirection = fireDirection.normalize();
        
        // Debug output
        std::cout << "=== GUN TO CROSSHAIR DIRECTION ===" << std::endl;
        std::cout << "Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
        std::cout << "Camera forward: (" << cameraForward.x << ", " << cameraForward.y << ", " << cameraForward.z << ")" << std::endl;
        std::cout << "Crosshair target: (" << crosshairTarget.x << ", " << crosshairTarget.y << ", " << crosshairTarget.z << ")" << std::endl;
        std::cout << "Gun barrel position: (" << gunBarrelPos.x << ", " << gunBarrelPos.y << ", " << gunBarrelPos.z << ")" << std::endl;
        std::cout << "Fire direction (gun to crosshair): (" << fireDirection.x << ", " << fireDirection.y << ", " << fireDirection.z << ")" << std::endl;
        std::cout << "===================================" << std::endl;
        
        return fireDirection;
    }
    return Vec3(0.0f, 0.0f, -1.0f); // Default forward direction
}

Vec3 ShootingSystem::getFirePosition() const {
    if (currentWeapon && playerCamera) {
        // Use the weapon's world position calculation (returns barrel tip position)
        Vec3 barrelTipPos = currentWeapon->getWorldPosition();
        
        // Debug output
        std::cout << "=== GUN BARREL FIRE POSITION ===" << std::endl;
        std::cout << "Gun barrel position: (" << barrelTipPos.x << ", " << barrelTipPos.y << ", " << barrelTipPos.z << ")" << std::endl;
        std::cout << "=================================" << std::endl;
        
        return barrelTipPos;
    }
    std::cout << "WARNING: No weapon or camera available for fire position!" << std::endl;
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

void WeaponShootingComponent::fireMonsterHunterShot() {
    if (isEnabled) {
        // Get fire position and direction from shooting system
        Vec3 firePos = shootingSystem.getFirePosition();
        Vec3 fireDir = shootingSystem.getFireDirection();
        
        // Fire monster hunter projectile
        shootingSystem.fireMonsterHunterProjectile(firePos, fireDir);
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
