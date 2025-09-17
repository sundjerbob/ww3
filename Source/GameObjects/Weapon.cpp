/**
 * Weapon.cpp - Implementation of 3D Weapon GameObject
 * 
 * OVERVIEW:
 * Implements weapon loading, positioning, and rendering for FPS-style weapon display.
 * Loads weapon models from OBJ files and renders them in a fixed screen position.
 */

#include "Weapon.h"
#include "../Engine/Core/ShootingSystem.h"
#include "../Engine/Utils/OBJLoader.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/WeaponRenderer.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Shader.h"
#include "../Engine/Math/Math.h"
#include <algorithm>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Engine {

Weapon::Weapon(const std::string& name, const std::string& modelPath, const Vec3& color)
    : GameObject(name),
      weaponModelPath(modelPath),
      weaponColor(color),
      weaponScale(0.5f),  // Smaller scale for better FPS weapon size
      screenPosition(0.0f, -0.1f, 0.0f),  // Legacy screen position (kept for compatibility)
      weaponOffset(0.0f, 0.0f, 0.0f),
      worldSpaceOffset(0.3f, -0.2f, 0.4f),  // 3D offset from camera in world space - closer and lower
      barrelTipOffset(0.0f, 0.0f, 0.6f),    // Offset to barrel tip from weapon origin - shorter
      isLoaded(true),  // Force weapon to be loaded by default
      isVisible(true),
      recoilOffset(0.0f, 0.0f, 0.0f),
      recoilVelocity(0.0f, 0.0f, 0.0f),
      recoilRecoveryRate(3.0f),
      recoilRotation(0.0f, 0.0f, 0.0f),
      recoilRotationVelocity(0.0f, 0.0f, 0.0f),
      rotationRecoveryRate(2.0f),
      maxPositionRecoil(0.3f),  // Maximum upward position recoil
      playerCamera(nullptr),
      aimSensitivity(1.0f),
      defaultRotation(0.0f, 0.0f, 0.0f),
      currentWeaponIndex(0),
      shootingEnabled(false) {
    
    
    // Set weapon to render in 2D screen space (not as 3D entity)
    setEntity(false);
    
    // Set initial position and scale
    setPosition(screenPosition);
    setScale(Vec3(weaponScale, weaponScale, weaponScale));
    setRotation(defaultRotation);
    
    // Don't create placeholder mesh here - let initialize() handle proper model loading
    
    // Initialize weapon inventory
    initializeWeaponInventory();
    
}

bool Weapon::initialize() {
    
    if (!GameObject::initialize()) {
        return false;
    }
    
    // Load weapon model if path is provided
    if (!weaponModelPath.empty()) {
        if (!loadWeaponModel(weaponModelPath)) {
            // Fall back to placeholder mesh instead of failing
            setupMesh();
        } else {
        }
    } else {
        setupMesh();
    }
    
    // Setup default weapon mesh if no model loaded
    if (!mesh || !mesh->isValid()) {
        setupMesh();
    }
    
    // Initialize shooting system
    initializeShootingSystem();
    
    return true;
}

bool Weapon::loadWeaponModel(const std::string& modelPath) {
    
    // Check if file exists
    std::ifstream file(modelPath);
    if (!file.good()) {
        return false;
    }
    file.close();
    
    // Check file size to detect empty files
    std::ifstream sizeCheck(modelPath, std::ios::ate | std::ios::binary);
    if (sizeCheck.is_open()) {
        std::streamsize fileSize = sizeCheck.tellg();
        sizeCheck.close();
        if (fileSize < 100) { // Very small file, likely empty or corrupted
            return false;
        }
    }
    
    // Load OBJ data
    OBJMeshData objData = OBJLoader::loadOBJ(modelPath, weaponScale);
    
    if (!objData.isValid()) {
        return false;
    }
    
    
    // Create mesh from OBJ data
    mesh = std::make_unique<Mesh>();
    
    // Convert OBJ data to interleaved format for texture coordinates
    // OBJ data has interleaved format: [pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, tex.u, tex.v]
    // We need to create interleaved format: [pos.x, pos.y, pos.z, tex.u, tex.v] (5 floats per vertex)
    std::vector<float> interleavedData;
    
    for (size_t i = 0; i < objData.vertices.size(); i += 8) {
        // Position (first 3 floats)
        interleavedData.push_back(objData.vertices[i]);     // pos.x
        interleavedData.push_back(objData.vertices[i + 1]); // pos.y
        interleavedData.push_back(objData.vertices[i + 2]); // pos.z
        
        // Texture coordinates (last 2 floats)
        interleavedData.push_back(objData.vertices[i + 6]); // tex.u
        interleavedData.push_back(objData.vertices[i + 7]); // tex.v
    }
    
    
    // Create mesh with interleaved position and texture coordinate data
    if (!mesh->createMeshWithTexCoords(interleavedData, objData.indices)) {
        return false;
    }
    
    // Store materials for multi-color rendering
    weaponMaterials = objData.materials;
    
    // Print all available materials with their colors
    for (const auto& materialName : weaponMaterials.getMaterialNames()) {
        const Material* mat = weaponMaterials.getMaterial(materialName);
        if (mat) {
        }
    }
    
    // Create material groups for multi-material rendering
            createMaterialGroups(objData);
    
    // Calculate and apply weapon-specific transformations
    // Center the weapon and apply proper orientation
    Vec3 weaponCenter = objData.center;
    Vec3 weaponSize = objData.boundingBoxMax - objData.boundingBoxMin;
    
    // Adjust weapon position to center it
    Vec3 adjustedPosition = screenPosition - weaponCenter * weaponScale;
    setPosition(adjustedPosition + weaponOffset);
    
    // Apply base orientation from configuration
    setRotation(defaultRotation);
    
    isLoaded = true;
    weaponModelPath = modelPath;
    
    
    return true;
}

void Weapon::update(float deltaTime) {
    GameObject::update(deltaTime);
    
    // Debug output to check weapon state
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 300 == 0) { // Print every 5 seconds at 60fps
    }
    
    if (!isVisible || !isLoaded) return;
    
    // Force weapon model loading if not already loaded
    if (!mesh || !mesh->isValid() || weaponModelPath.empty()) {
        static bool modelLoadAttempted = false;
        if (!modelLoadAttempted) {
            if (!weaponModelPath.empty()) {
                if (loadWeaponModel(weaponModelPath)) {
                } else {
                    setupMesh();
                }
            } else {
                setupMesh();
            }
            modelLoadAttempted = true;
        }
    }
    
    // Update weapon position and rotation based on camera
    updateWeaponPosition();
    updateWeaponRotation();
    
    // Update recoil system
    updateRecoil(deltaTime);
    
    // Update shooting system
    if (shootingEnabled) {
        updateShootingSystem(deltaTime);
    }
}

void Weapon::cleanup() {
    // Clean up shooting system
    if (shootingEnabled) {
        shootingComponent.cleanup();
    }
    
    // Call base class cleanup
    GameObject::cleanup();
}

void Weapon::render(const Renderer& renderer, const Camera& camera) {
    // Debug output to see if render is being called
    static int renderCount = 0;
    renderCount++;
    if (renderCount % 300 == 0) { // Print every 5 seconds at 60fps
    }
    
    if (!isVisible || !mesh || !mesh->isValid()) {
        return;
    }
    
    // Use the specialized WeaponRenderer if available
    const WeaponRenderer* weaponRenderer = dynamic_cast<const WeaponRenderer*>(&renderer);
    if (weaponRenderer) {
        // Use weapon-specific rendering with multi-material support
        Mat4 weaponMatrix = createWeaponTransformMatrix();
        
        // Multi-material rendering: Render each material group with its own color
        // This successfully renders the weapon with true multi-material colors (black, brown, gray)
        // as defined in the MTL file, giving realistic weapon appearance
        if (!materialGroups.empty()) {
            for (const auto& materialGroup : materialGroups) {
                weaponRenderer->renderWeaponTriangles(*mesh, weaponMatrix, camera, materialGroup.color, materialGroup.indices, true);
            }
        } else {
            // Fallback to blended material color if no material groups
            Vec3 renderColor = weaponColor; // Default fallback color
            if (weaponMaterials.getMaterialCount() > 0) {
                // Use the first material's color
                auto materialNames = weaponMaterials.getMaterialNames();
                if (!materialNames.empty()) {
                    const Material* firstMaterial = weaponMaterials.getMaterial(materialNames[0]);
                    if (firstMaterial) {
                        renderColor = firstMaterial->diffuse;
                    }
                }
            }
            weaponRenderer->renderWeaponMesh(*mesh, weaponMatrix, camera, renderColor, true);
        }
    } else {
        // Fallback to basic rendering (backward compatibility)
        GameObject::render(renderer, camera);
    }
}

void Weapon::setupMesh() {
    // Create a simple placeholder weapon mesh (cube) if no model is loaded
    std::vector<float> vertices = {
        // Front face
        -0.1f, -0.1f,  0.1f,  // 0
         0.1f, -0.1f,  0.1f,  // 1
         0.1f,  0.1f,  0.1f,  // 2
        -0.1f,  0.1f,  0.1f,  // 3
        // Back face
        -0.1f, -0.1f, -0.1f,  // 4
         0.1f, -0.1f, -0.1f,  // 5
         0.1f,  0.1f, -0.1f,  // 6
        -0.1f,  0.1f, -0.1f   // 7
    };
    
    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        5, 4, 7, 7, 6, 5,
        // Left face
        4, 0, 3, 3, 7, 4,
        // Right face
        1, 5, 6, 6, 2, 1,
        // Top face
        3, 2, 6, 6, 7, 3,
        // Bottom face
        4, 5, 1, 1, 0, 4
    };
    
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMesh(vertices, indices)) {
    }
    
}

void Weapon::updateWeaponPosition() {
    // Apply weapon position with recoil offset
    Vec3 finalPosition = screenPosition + weaponOffset + recoilOffset;
    setPosition(finalPosition);
    
    // Debug output
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 300 == 0) { // Print every 5 seconds at 60fps
    }
}

void Weapon::updateWeaponRotation() {
    if (!playerCamera) return;
    
    // Base orientation to point forward into -Z view space
    Vec3 weaponRotation = defaultRotation;
    
    // Enhanced aim-follow effect to show more handle when looking down
    Vec3 cameraRotation = playerCamera->getRotation(); // degrees
    float pitchInfluence = std::max(-5.0f, std::min(5.0f, cameraRotation.x * 0.1f)); // Increased influence
    float yawInfluence   = std::max(-1.0f, std::min(1.0f, cameraRotation.y * 0.005f)); // GREATLY reduced yaw influence to minimize bullet path shifting
    
    // Add a slight downward tilt to show more of the handle
    float handleTilt = -3.0f; // Tilt down to show handle
    weaponRotation.x = handleTilt + pitchInfluence;
    weaponRotation.y += yawInfluence;
    
    // Apply weapon-specific rotation corrections
    if (currentWeaponIndex == 4) { // Shotgun - special correction
        weaponRotation.z = 0.0f; // Ensure no Z rotation
        weaponRotation.x = std::max(-8.0f, std::min(2.0f, weaponRotation.x)); // Allow more downward tilt
        // Keep the 90-degree Y rotation for proper shotgun orientation
        weaponRotation.y = defaultRotation.y + yawInfluence;
    } else {
        // For other weapons, allow more downward tilt to show handle
        weaponRotation.x = std::max(-8.0f, std::min(2.0f, weaponRotation.x));
    }
    
    // ADD RECOIL ROTATION (unlimited upward tilt)
    weaponRotation.x += recoilRotation.x;  // Add recoil rotation to pitch
    
    setRotation(weaponRotation);
    
    // Debug output
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 300 == 0) { // Print every 5 seconds at 60fps
    }
}

Vec3 Weapon::calculateAimDirection() const {
    if (!playerCamera) {
        return Vec3(0.0f, 0.0f, -1.0f);  // Default forward direction
    }
    
    // ALWAYS use camera's forward direction for bullet path
    // This ensures bullets always go where the crosshair is pointing
    // regardless of weapon visual rotation
    return playerCamera->getForward();
}

Mat4 Weapon::createWeaponTransformMatrix() const {
    // Create transformation matrix for weapon positioning in view space
    // Map NDC-like screenPosition [-1,1] to a fixed view-space position at depth -zDepth
    const float zDepth = 0.8f; // Original depth
    
    // Use the weapon's actual position (which includes recoil) instead of just screenPosition
    Vec3 weaponPos = getPosition(); // This includes screenPosition + weaponOffset + recoilOffset
    Vec3 viewSpacePos(weaponPos.x, weaponPos.y, -zDepth);
    
    Mat4 modelMatrix = Mat4();
    Mat4 translationMatrix = Engine::translate(Mat4(), viewSpacePos);
    modelMatrix = Engine::multiply(modelMatrix, translationMatrix);
    
    // Apply rotation
    Vec3 rotation = getRotation();
    Mat4 rotationMatrix = Mat4();
    
    if (rotation.x != 0.0f) {
        Mat4 rotX = Engine::rotateX(rotation.x * 3.14159f / 180.0f);
        rotationMatrix = Engine::multiply(rotationMatrix, rotX);
    }
    
    if (rotation.y != 0.0f) {
        Mat4 rotY = Engine::rotateY(rotation.y * 3.14159f / 180.0f);
        rotationMatrix = Engine::multiply(rotationMatrix, rotY);
    }
    
    if (rotation.z != 0.0f) {
        Mat4 rotZ = Engine::rotateZ(rotation.z * 3.14159f / 180.0f);
        rotationMatrix = Engine::multiply(rotationMatrix, rotZ);
    }
    
    modelMatrix = Engine::multiply(modelMatrix, rotationMatrix);
    
    // Apply weapon scale
    Vec3 scale(weaponScale, weaponScale, weaponScale);
    Mat4 scaleMatrix = Engine::scale(scale);
    modelMatrix = Engine::multiply(modelMatrix, scaleMatrix);
    
    // Debug output
    static int debugCount = 0;
    debugCount++;
    if (debugCount % 300 == 0) { // Print every 5 seconds at 60fps
    }
    
    return modelMatrix;
}

void Weapon::createMaterialGroups(const OBJMeshData& objData) {
    // Implementation for creating material groups from OBJ data
    // Parse the OBJ data and create material groups based on material assignments
    
    // Clear existing material groups
    materialGroups.clear();
    
    // Create material groups based on loaded materials
    if (weaponMaterials.getMaterialCount() > 0 && !objData.faceMaterials.empty()) {
        auto materialNames = weaponMaterials.getMaterialNames();
        
        // Create a map to store indices for each material
        std::map<std::string, std::vector<unsigned int>> materialIndexMap;
        
        // Initialize empty index vectors for each material
        for (const auto& materialName : materialNames) {
            materialIndexMap[materialName] = std::vector<unsigned int>();
        }
        
        // Parse face-material assignments and group indices by material
        for (size_t faceIndex = 0; faceIndex < objData.faceMaterials.size(); faceIndex++) {
            const std::string& faceMaterial = objData.faceMaterials[faceIndex];
            
            // Find the corresponding material group
            auto it = materialIndexMap.find(faceMaterial);
            if (it != materialIndexMap.end()) {
                // Add the 3 indices for this triangle (face) to the material group
                size_t baseIndex = faceIndex * 3; // Each face has 3 vertices
                if (baseIndex + 2 < objData.indices.size()) {
                    it->second.push_back(objData.indices[baseIndex]);
                    it->second.push_back(objData.indices[baseIndex + 1]);
                    it->second.push_back(objData.indices[baseIndex + 2]);
                }
            }
        }
        
        // Create material groups from the parsed data
        for (const auto& materialName : materialNames) {
            const Material* mat = weaponMaterials.getMaterial(materialName);
            if (mat && !materialIndexMap[materialName].empty()) {
                MaterialGroup group;
                group.materialName = materialName;
                group.color = mat->diffuse;
                group.indices = materialIndexMap[materialName];
                
                materialGroups.push_back(group);
                // std::cout << "Created material group '" << materialName << "' with color ("
                //           << mat->diffuse.x << ", " << mat->diffuse.y << ", " << mat->diffuse.z 
                //           << ") and " << group.indices.size() / 3 << " triangles" << std::endl;
            }
        }
    } else {
        // Fallback: create a default material group with weapon color
        MaterialGroup defaultGroup;
        defaultGroup.materialName = "default";
        defaultGroup.color = weaponColor;
        
        // Assign all indices to the default group
        for (size_t i = 0; i < objData.indices.size(); i++) {
            defaultGroup.indices.push_back(objData.indices[i]);
        }
        
        materialGroups.push_back(defaultGroup);
    }
    
}

// Weapon switching implementation
void Weapon::initializeWeaponInventory() {
    weaponInventory.clear();
    
    // Weapon 1: Assault Rifle (Slot 1)
    WeaponData assaultRifle;
    assaultRifle.name = "Assault Rifle";
    assaultRifle.modelPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/AssaultRifle_01.obj";
    assaultRifle.color = Vec3(0.8f, 0.8f, 0.8f);
    assaultRifle.scale = 0.35f;
    assaultRifle.offset = Vec3(0.0f, 0.0f, 0.0f);  // Consistent position for all weapons
    assaultRifle.defaultRotation = Vec3(0.0f, 0.0f, 0.0f);
    assaultRifle.aimSensitivity = 1.0f;
    
    // Configure assault rifle shooting stats
    assaultRifle.shootingStats.fireMode = FireMode::Auto;
    assaultRifle.shootingStats.fireRate = 8.0f; // 8 shots per second
    assaultRifle.shootingStats.spread = 2.0f;
    assaultRifle.shootingStats.recoil = 0.3f;
    assaultRifle.shootingStats.accuracy = 0.8f;
    assaultRifle.shootingStats.ammoType = AmmoType::Bullet;
    assaultRifle.shootingStats.maxAmmo = 30;
    assaultRifle.shootingStats.currentAmmo = 30;
    assaultRifle.shootingStats.maxReserveAmmo = 90;
    assaultRifle.shootingStats.currentReserveAmmo = 90;
    assaultRifle.shootingStats.projectileType = ProjectileType::Bullet;
    assaultRifle.shootingStats.projectileConfig = ProjectileFactory::createBulletConfig();
    assaultRifle.shootingStats.reloadTime = 2.5f;
    
    weaponInventory.push_back(assaultRifle);
    
    // Weapon 2: Sniper Rifle (Slot 2)
    WeaponData sniperRifle;
    sniperRifle.name = "Sniper Rifle";
    sniperRifle.modelPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/SniperRifle_01.obj";
    sniperRifle.color = Vec3(0.8f, 0.8f, 0.8f);
    sniperRifle.scale = 0.35f;
    sniperRifle.offset = Vec3(0.0f, 0.0f, 0.0f);  // Consistent position for all weapons
    sniperRifle.defaultRotation = Vec3(0.0f, 0.0f, 0.0f);
    sniperRifle.aimSensitivity = 1.0f;
    
    // Configure sniper rifle shooting stats
    sniperRifle.shootingStats.fireMode = FireMode::Single;
    sniperRifle.shootingStats.fireRate = 1.5f;
    sniperRifle.shootingStats.spread = 0.1f; // Very accurate
    sniperRifle.shootingStats.recoil = 0.8f; // High recoil
    sniperRifle.shootingStats.accuracy = 0.95f;
    sniperRifle.shootingStats.ammoType = AmmoType::Bullet;
    sniperRifle.shootingStats.maxAmmo = 5;
    sniperRifle.shootingStats.currentAmmo = 5;
    sniperRifle.shootingStats.maxReserveAmmo = 15;
    sniperRifle.shootingStats.currentReserveAmmo = 15;
    sniperRifle.shootingStats.projectileType = ProjectileType::Bullet;
    sniperRifle.shootingStats.projectileConfig = ProjectileFactory::createBulletConfig();
    sniperRifle.shootingStats.projectileConfig.damage = 100.0f; // High damage
    sniperRifle.shootingStats.projectileConfig.speed = 150.0f; // Fast bullets
    sniperRifle.shootingStats.reloadTime = 3.0f;
    
    weaponInventory.push_back(sniperRifle);
    
    // Weapon 3: Submachine Gun (Slot 3)
    WeaponData smg;
    smg.name = "Submachine Gun";
    smg.modelPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/SubmachineGun_01.obj";
    smg.color = Vec3(0.8f, 0.8f, 0.8f);
    smg.scale = 0.25f;
    smg.offset = Vec3(0.0f, 0.0f, 0.0f);  // Consistent position for all weapons
    smg.defaultRotation = Vec3(0.0f, 0.0f, 0.0f);
    smg.aimSensitivity = 1.0f;
    
    // Configure SMG shooting stats
    smg.shootingStats.fireMode = FireMode::Auto;
    smg.shootingStats.fireRate = 12.0f; // Very fast
    smg.shootingStats.spread = 3.0f; // More spread
    smg.shootingStats.recoil = 0.2f; // Low recoil
    smg.shootingStats.accuracy = 0.7f;
    smg.shootingStats.ammoType = AmmoType::Bullet;
    smg.shootingStats.maxAmmo = 25;
    smg.shootingStats.currentAmmo = 25;
    smg.shootingStats.maxReserveAmmo = 75;
    smg.shootingStats.currentReserveAmmo = 75;
    smg.shootingStats.projectileType = ProjectileType::Bullet;
    smg.shootingStats.projectileConfig = ProjectileFactory::createBulletConfig();
    smg.shootingStats.projectileConfig.damage = 15.0f; // Lower damage
    smg.shootingStats.reloadTime = 2.0f;
    
    weaponInventory.push_back(smg);
    
    // Weapon 4: Pistol (Slot 4)
    WeaponData pistol;
    pistol.name = "Pistol";
    pistol.modelPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/Pistol_01.obj";
    pistol.color = Vec3(0.8f, 0.8f, 0.8f);
    pistol.scale = 0.28f;
    pistol.offset = Vec3(0.0f, 0.0f, 0.0f);  // Consistent position for all weapons
    pistol.defaultRotation = Vec3(0.0f, 0.0f, 0.0f);
    pistol.aimSensitivity = 1.0f;
    
    // Configure pistol shooting stats
    pistol.shootingStats.fireMode = FireMode::SemiAuto;
    pistol.shootingStats.fireRate = 3.0f;
    pistol.shootingStats.spread = 1.5f;
    pistol.shootingStats.recoil = 0.4f;
    pistol.shootingStats.accuracy = 0.85f;
    pistol.shootingStats.ammoType = AmmoType::Bullet;
    pistol.shootingStats.maxAmmo = 12;
    pistol.shootingStats.currentAmmo = 12;
    pistol.shootingStats.maxReserveAmmo = 36;
    pistol.shootingStats.currentReserveAmmo = 36;
    pistol.shootingStats.projectileType = ProjectileType::Bullet;
    pistol.shootingStats.projectileConfig = ProjectileFactory::createBulletConfig();
    pistol.shootingStats.projectileConfig.damage = 30.0f;
    pistol.shootingStats.reloadTime = 1.5f;
    
    weaponInventory.push_back(pistol);
    
    // Weapon 5: Shotgun (Slot 5)
    WeaponData shotgun;
    shotgun.name = "Shotgun";
    shotgun.modelPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/Shotgun_01.obj";
    shotgun.color = Vec3(0.8f, 0.8f, 0.8f);
    shotgun.scale = 0.30f;
    shotgun.offset = Vec3(0.0f, 0.0f, 0.0f);  // Consistent position for all weapons
    shotgun.defaultRotation = Vec3(0.0f, 90.0f, 0.0f);
    shotgun.aimSensitivity = 1.0f;
    
    // Configure shotgun shooting stats
    shotgun.shootingStats.fireMode = FireMode::Single;
    shotgun.shootingStats.fireRate = 1.0f; // Slow fire rate
    shotgun.shootingStats.spread = 8.0f; // High spread
    shotgun.shootingStats.recoil = 0.9f; // Very high recoil
    shotgun.shootingStats.accuracy = 0.6f;
    shotgun.shootingStats.ammoType = AmmoType::Bullet;
    shotgun.shootingStats.maxAmmo = 8;
    shotgun.shootingStats.currentAmmo = 8;
    shotgun.shootingStats.maxReserveAmmo = 24;
    shotgun.shootingStats.currentReserveAmmo = 24;
    shotgun.shootingStats.projectileType = ProjectileType::Bullet;
    shotgun.shootingStats.projectileConfig = ProjectileFactory::createBulletConfig();
    shotgun.shootingStats.projectileConfig.damage = 80.0f; // High damage
    shotgun.shootingStats.projectileConfig.size = 0.08f; // Larger projectiles
    shotgun.shootingStats.reloadTime = 4.0f; // Slow reload
    
    weaponInventory.push_back(shotgun);
    
    // Set initial weapon
    currentWeaponIndex = 0;
    
    for (size_t i = 0; i < weaponInventory.size(); i++) {
    }
}

bool Weapon::switchToWeapon(int weaponIndex) {
    if (weaponIndex < 0 || weaponIndex >= static_cast<int>(weaponInventory.size())) {
        return false;
    }
    
    if (weaponIndex == currentWeaponIndex) {
        return true; // Already using this weapon
    }
    
    
    // Update weapon properties
    const WeaponData& newWeapon = weaponInventory[weaponIndex];
    weaponModelPath = newWeapon.modelPath;
    weaponColor = newWeapon.color;
    weaponScale = newWeapon.scale;
    weaponOffset = newWeapon.offset;
    defaultRotation = newWeapon.defaultRotation;
    aimSensitivity = newWeapon.aimSensitivity;
    
    // Configure shooting stats for the new weapon
    if (shootingEnabled) {
        configureShooting(newWeapon.shootingStats);
    }
    
    // Load the new weapon model
    bool modelLoaded = false;
    
    if (!loadWeaponModel(newWeapon.modelPath)) {
        
        // Special fallback for shotgun (weapon 5)
        if (weaponIndex == 4 && newWeapon.name == "Shotgun") {
            std::string fallbackPath = "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/Shotgun_03.obj";
            if (loadWeaponModel(fallbackPath)) {
                modelLoaded = true;
            } else {
            }
        }
        
        if (!modelLoaded) {
            setupMesh(); // Fall back to placeholder mesh
        }
    } else {
        modelLoaded = true;
    }
    
    currentWeaponIndex = weaponIndex;
    
    // Update weapon position and rotation
    updateWeaponPosition();
    updateWeaponRotation();
    
    return true;
}

bool Weapon::switchToWeapon(const std::string& weaponName) {
    for (size_t i = 0; i < weaponInventory.size(); i++) {
        if (weaponInventory[i].name == weaponName) {
            return switchToWeapon(static_cast<int>(i));
        }
    }
    
    return false;
}

const std::string& Weapon::getCurrentWeaponName() const {
    if (currentWeaponIndex >= 0 && currentWeaponIndex < static_cast<int>(weaponInventory.size())) {
        return weaponInventory[currentWeaponIndex].name;
    }
    static const std::string defaultName = "Unknown";
    return defaultName;
}

void Weapon::cycleToNextWeapon() {
    if (weaponInventory.empty()) return;
    
    int nextIndex = (currentWeaponIndex + 1) % static_cast<int>(weaponInventory.size());
    switchToWeapon(nextIndex);
}

void Weapon::cycleToPreviousWeapon() {
    if (weaponInventory.empty()) return;
    
    int prevIndex = currentWeaponIndex - 1;
    if (prevIndex < 0) {
        prevIndex = static_cast<int>(weaponInventory.size()) - 1;
    }
    switchToWeapon(prevIndex);
}

// Shooting system integration methods
void Weapon::initializeShootingSystem() {
    // Initialize shooting component with default systems
    // Note: These systems will need to be passed from the Game class
    // For now, we'll use nullptr placeholders
    shootingComponent.initialize(nullptr, playerCamera, nullptr);
    
    // Attach this weapon to the shooting system
    shootingComponent.attachToWeapon(this);
    
    // Set up recoil callback to connect shooting system recoil to weapon visual recoil
    ShootingSystem* shootingSystem = shootingComponent.getShootingSystem();
    if (shootingSystem) {
        shootingSystem->setRecoilCallback([this](const Vec3& recoil) {
            this->applyRecoil(recoil);
        });
    }
    
    shootingEnabled = true;
    
}

void Weapon::setProjectileManager(ProjectileManager* manager) {
    // Update the shooting component with the projectile manager
    if (shootingEnabled) {
        shootingComponent.getShootingSystem()->setProjectileManager(manager);
    }
}

void Weapon::updateShootingSystem(float deltaTime) {
    if (shootingEnabled) {
        shootingComponent.update(deltaTime);
    }
}

void Weapon::startFiring() {
    if (shootingEnabled) {
        shootingComponent.startFiring();
    }
}

void Weapon::stopFiring() {
    if (shootingEnabled) {
        shootingComponent.stopFiring();
    }
}

void Weapon::fireSingleShot() {
    if (shootingEnabled) {
        shootingComponent.fireSingleShot();
    }
}

void Weapon::fireMonsterHunterShot() {
    if (shootingEnabled) {
        shootingComponent.fireMonsterHunterShot();
    }
}

bool Weapon::canFire() const {
    return shootingEnabled && shootingComponent.canFire();
}

bool Weapon::hasAmmo() const {
    return shootingEnabled && shootingComponent.hasAmmo();
}

void Weapon::reload() {
    if (shootingEnabled) {
        shootingComponent.reload();
    }
}

int Weapon::getCurrentAmmo() const {
    return shootingEnabled ? shootingComponent.getCurrentAmmo() : 0;
}

int Weapon::getReserveAmmo() const {
    return shootingEnabled ? shootingComponent.getReserveAmmo() : 0;
}

bool Weapon::isReloading() const {
    return shootingEnabled && shootingComponent.isReloading();
}

bool Weapon::isFiring() const {
    return shootingEnabled && shootingComponent.isFiring();
}

void Weapon::configureShooting(const WeaponStats& stats) {
    if (shootingEnabled) {
        shootingComponent.configureWeapon(stats);
    }
}

const WeaponStats& Weapon::getShootingStats() const {
    static WeaponStats defaultStats;
    return shootingEnabled ? shootingComponent.getShootingSystem()->getWeaponStats() : defaultStats;
}

Vec3 Weapon::getFirePosition() const {
    // Calculate fire position based on weapon position and camera
    Vec3 weaponPos = getPosition();
    
    if (playerCamera) {
        // Offset from weapon position in camera forward direction
        Vec3 cameraForward = playerCamera->getForward();
        return weaponPos + cameraForward * 0.5f; // Small offset from weapon
    }
    
    return weaponPos;
}

Vec3 Weapon::getFireDirection() const {
    if (playerCamera) {
        return playerCamera->getForward();
    }
    
    // Fallback to weapon's forward direction
    return calculateAimDirection();
}

// Recoil methods
void Weapon::applyRecoil(const Vec3& recoil) {
    // 1. POSITION-BASED RECOIL (limited upward movement)
    float positionRecoil = recoil.y * 0.2f;  // Scale for screen space
    recoilOffset.y += positionRecoil;
    
    // Apply maximum limit to position recoil
    recoilOffset.y = std::min(recoilOffset.y, maxPositionRecoil);
    
    // Set position recoil velocity for smooth movement
    recoilVelocity.y = recoil.y * 0.5f;
    
    // 2. ROTATION-BASED RECOIL (unlimited upward tilt)
    float rotationRecoil = recoil.y * 0.8f;  // Scale for rotation
    recoilRotation.x -= rotationRecoil;  // NEGATIVE X-axis rotation = upward tilt
    
    // Set rotation recoil velocity for smooth movement
    recoilRotationVelocity.x = -recoil.y * 1.0f; // Negative for upward tilt
    
    // 3. TRIGGER CAMERA RECOIL CALLBACK
    if (onRecoilApplied) {
        onRecoilApplied(recoil);
    }
    
}

void Weapon::updateRecoil(float deltaTime) {
    // ALWAYS recover recoil to return to original position - FAST RECOVERY
    // 1. POSITION RECOIL RECOVERY (limited) - 5x faster
    if (recoilOffset.y > 0.0f) {
        recoilOffset.y -= recoilRecoveryRate * 5.0f * deltaTime;  // 5x faster recovery
        recoilOffset.y = std::max(0.0f, recoilOffset.y);
    }
    
    // 2. ROTATION RECOIL RECOVERY (unlimited) - 5x faster
    if (recoilRotation.x < 0.0f) {  // Negative values = upward tilt
        recoilRotation.x += rotationRecoveryRate * 5.0f * deltaTime;  // 5x faster recovery
        recoilRotation.x = std::min(0.0f, recoilRotation.x);   // Don't go past zero
    }
    
    // Update velocities for smooth movement - stronger damping for faster return
    recoilVelocity.y *= (1.0f - deltaTime * 10.0f); // Much stronger position velocity damping
    recoilRotationVelocity.x *= (1.0f - deltaTime * 8.0f); // Much stronger rotation velocity damping
}

Vec3 Weapon::getWorldPosition() const {
    if (!playerCamera) {
        // std::cout << "WARNING: No player camera available for weapon world position!" << std::endl;
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    
    // Get camera position and direction vectors
    Vec3 cameraPos = playerCamera->getPosition();
    Vec3 cameraForward = playerCamera->getForward();
    Vec3 cameraRight = playerCamera->getRight();
    Vec3 cameraUp = playerCamera->getUpVector();
    
    // Get weapon's screen position (in NDC-like coordinates)
    Vec3 weaponScreenPos = getPosition(); // This includes screenPosition + weaponOffset + recoilOffset
    
    // Convert weapon screen position to world coordinates
    // The weapon is positioned in view space, so we need to transform it to world space
    // Weapon screen position is relative to camera view
    
    // Calculate weapon world position based on screen position
    // Scale factors to convert from screen space to world space
    const float horizontalScale = 0.3f;  // Even more reduced horizontal scale for centered weapon
    const float verticalScale = 0.3f;    // Even more reduced vertical scale for centered weapon
    const float forwardOffset = 0.15f;   // Even more reduced forward offset for centered weapon
    
    Vec3 weaponWorldPos = cameraPos + 
                         cameraRight * weaponScreenPos.x * horizontalScale +  // Horizontal offset
                         cameraUp * weaponScreenPos.y * verticalScale +       // Vertical offset  
                         cameraForward * (forwardOffset + weaponScreenPos.z);  // Forward offset
    
    // Adjust position to be more precisely at the gun barrel
    // Move forward along the weapon's   forward direction to get to the barrel tip
    // Use a more precise barrel length calculation based on weapon scale
    const float baseBarrelLength = 0.5f;  // Base barrel length for standard weapon
    float barrelLength = baseBarrelLength * weaponScale;  // Scale barrel length with weapon scale
    Vec3 weaponForward = calculateAimDirection();  // Get weapon's aim direction
    Vec3 barrelTipPos = weaponWorldPos + weaponForward * barrelLength;
    
    // Debug output (disabled for now)
    // std::cout << "=== WEAPON WORLD POSITION ===" << std::endl;
    // std::cout << "Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    // std::cout << "Weapon screen position: (" << weaponScreenPos.x << ", " << weaponScreenPos.y << ", " << weaponScreenPos.z << ")" << std::endl;
    // std::cout << "Weapon world position: (" << weaponWorldPos.x << ", " << weaponWorldPos.y << ", " << weaponWorldPos.z << ")" << std::endl;
    // std::cout << "Barrel tip position: (" << barrelTipPos.x << ", " << barrelTipPos.y << ", " << barrelTipPos.z << ")" << std::endl;
    // std::cout << "=============================" << std::endl;
    
    return barrelTipPos;
}

// ===== NEW 3D WORLD-SPACE POSITIONING METHODS =====

void Weapon::updateWorldPosition() {
    if (!playerCamera) {
        return;
    }
    
    // Get camera position and orientation
    Vec3 cameraPos = playerCamera->getPosition();
    Vec3 cameraForward = playerCamera->getForward();
    Vec3 cameraRight = playerCamera->getRight();
    Vec3 cameraUp = playerCamera->getUpVector();
    
    // Calculate weapon position in world space
    // Apply 3D offset from camera using world-space coordinates
    Vec3 weaponWorldPos = cameraPos + 
                         cameraRight * (worldSpaceOffset.x + recoilOffset.x) +    // Right/Left offset + recoil
                         cameraUp * (worldSpaceOffset.y + recoilOffset.y) +       // Up/Down offset + recoil
                         cameraForward * (worldSpaceOffset.z + recoilOffset.z);   // Forward/Back offset + recoil
    
    // Update weapon's position in world space
    setPosition(weaponWorldPos);
    
    // Calculate weapon rotation to always point forward with camera
    // Apply camera's yaw and pitch to weapon orientation
    Vec3 weaponRotation = playerCamera->getRotation();
    weaponRotation += defaultRotation; // Add weapon-specific rotation offset
    weaponRotation += recoilRotation;  // Add recoil-based rotation
    setRotation(weaponRotation);
    
    // Debug output (can be enabled for testing)
    /*
    std::cout << "=== 3D WEAPON POSITIONING ===" << std::endl;
    std::cout << "Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "World space offset: (" << worldSpaceOffset.x << ", " << worldSpaceOffset.y << ", " << worldSpaceOffset.z << ")" << std::endl;
    std::cout << "Weapon world position: (" << weaponWorldPos.x << ", " << weaponWorldPos.y << ", " << weaponWorldPos.z << ")" << std::endl;
    std::cout << "Weapon rotation: (" << weaponRotation.x << ", " << weaponRotation.y << ", " << weaponRotation.z << ")" << std::endl;
    std::cout << "=============================" << std::endl;
    */
}

Vec3 Weapon::getBarrelTipPosition() const {
    if (!playerCamera) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    
    // PURE FIXED WORLD OFFSET SOLUTION:
    // Gun positioned at constant world coordinates relative to camera
    // This creates consistent screen appearance without rotation dependency
    
    Vec3 cameraPos = playerCamera->getPosition();
    
    // ABSOLUTE FIXED WORLD OFFSET - never affected by camera rotation
    Vec3 fixedWorldOffset = Vec3(0.15f, -0.1f, 0.4f);
    Vec3 gunWorldPos = cameraPos + fixedWorldOffset;
    
    // Debug to verify position is truly constant
    static int shotCount = 0;
    shotCount++;
    std::cout << "=== SHOT " << shotCount << " PURE FIXED OFFSET ===" << std::endl;
    std::cout << "Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "Fixed world offset: (" << fixedWorldOffset.x << ", " << fixedWorldOffset.y << ", " << fixedWorldOffset.z << ")" << std::endl;
    std::cout << "Gun world position: (" << gunWorldPos.x << ", " << gunWorldPos.y << ", " << gunWorldPos.z << ")" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    return gunWorldPos;
}

} // namespace Engine
