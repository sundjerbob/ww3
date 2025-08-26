/**
 * Monster.cpp - Implementation of Simplified Monster/Enemy System
 */

#include "Monster.h"
#include "../Engine/Core/Projectile.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Utils/OBJLoader.h"
#include "../Engine/Rendering/MaterialLoader.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/MonsterRenderer.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>

namespace Engine {

Monster::Monster(const std::string& name, MonsterType monsterType)
    : GameObject(name),
      type(monsterType),
      state(MonsterState::Idle),
      health(100.0f),
      maxHealth(100.0f),
      moveSpeed(2.0f),
      attackRange(2.0f),
      attackDamage(25.0f),
      detectionRange(10.0f),
      targetPosition(0.0f, 0.0f, 0.0f),
      moveTimer(0.0f),
      stateTimer(0.0f),
      patrolRadius(15.0f),
      lastAttackTime(0.0f),
      attackCooldown(2.0f),
      damageFlashTimer(0.0f),
      originalColor(0.5f, 0.14f, 0.58f),  // Xenomorph purple color
      damageColor(1.0f, 0.0f, 0.0f),      // Red for damage
      isFlashing(false),
      playerTarget(nullptr) {
    
    // Set entity flag to true for monsters
    setEntity(true);
    
    // Configure monster based on type
    configureMonster(monsterType);
}

bool Monster::initialize() {
    if (isInitialized) return true;
    
    std::cout << "=== INITIALIZING MONSTER: " << getName() << " ===" << std::endl;
    
    // Setup monster mesh
    std::cout << "Setting up monster mesh..." << std::endl;
    setupMonsterMesh();
    
    // Setup monster material
    std::cout << "Setting up monster material..." << std::endl;
    setupMonsterMaterial();
    
    // Set initial target position
    targetPosition = getPosition();
    
    isInitialized = true;
    std::cout << "Monster initialized successfully: " << getName() << std::endl;
    std::cout << "=== END INITIALIZATION ===" << std::endl;
    return true;
}

void Monster::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // Update AI behavior
    updateAI(deltaTime);
    
    // Update movement
    updateMovement(deltaTime);
    
    // Update visual effects
    updateVisualEffects(deltaTime);
    
    // Call base class update
    GameObject::update(deltaTime);
}

void Monster::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized || isDead()) return;
    
    // Try to use MonsterRenderer for multi-material rendering
    const MonsterRenderer* monsterRenderer = dynamic_cast<const MonsterRenderer*>(&renderer);
    if (monsterRenderer && !materialGroups.empty()) {
        // Use monster renderer for multi-material rendering
        Mat4 monsterMatrix = getModelMatrix();
        
        // Render each material group with its own color
        for (const auto& materialGroup : materialGroups) {
            Vec3 renderColor = materialGroup.color;
            
            // Apply damage flash effect
            if (isFlashing) {
                renderColor = damageColor;
            }
            
            monsterRenderer->renderMonsterTriangles(*mesh, monsterMatrix, camera, renderColor, materialGroup.indices, true);
        }
    } else {
        // Fallback to basic renderer with dominant material color
        if (!materialGroups.empty()) {
            // Use the first material group's color as the dominant color
            Vec3 materialColor = materialGroups[0].color;
            
            // Apply damage flash effect
            if (isFlashing) {
                materialColor = damageColor;
            }
            
            setColor(materialColor);
        } else {
            // Fallback to original color system
            setColor(getCurrentColor());
        }
        
        // Render the monster using the basic renderer
        GameObject::render(renderer, camera);
    }
}

void Monster::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up Monster: " << getName() << std::endl;
    
    playerTarget = nullptr;
    
    GameObject::cleanup();
}

void Monster::spawn(const Vec3& position) {
    setPosition(position);
    targetPosition = position;
    setHealth(maxHealth);
    setState(MonsterState::Patrolling);
    resetTimers();
    setActive(true);
    
    std::cout << "Monster spawned at: " << position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Monster::takeDamage(float damage, GameObject* attacker) {
    if (isDead()) return;
    
    health = std::max(0.0f, health - damage);
    flashDamage();
    
    std::cout << "Monster " << getName() << " took " << damage << " damage. Health: " << health << "/" << maxHealth << std::endl;
    
    // If health reaches 0, die
    if (health <= 0.0f) {
        die();
    }
    
    // Call damage callback
    onDamage(damage, attacker);
}

void Monster::die() {
    if (isDead()) return;
    
    setState(MonsterState::Dead);
    setActive(false);
    
    std::cout << "Monster " << getName() << " died!" << std::endl;
    
    // Call death callback
    onDeath();
}

void Monster::updateAI(float deltaTime) {
    if (isDead()) return;
    
    // Update state timer
    stateTimer += deltaTime;
    
    // Update state based on conditions
    updateState(deltaTime);
}

void Monster::updateMovement(float deltaTime) {
    if (isDead()) return;
    
    Vec3 currentPos = getPosition();
    Vec3 newPos = currentPos;
    
    switch (state) {
        case MonsterState::Idle:
            // Do nothing, just stand still
            break;
            
        case MonsterState::Patrolling:
            patrol(deltaTime);
            break;
            
        case MonsterState::Chasing:
            moveTowardsTarget(deltaTime);
            break;
            
        case MonsterState::Attacking:
            // Stop moving when attacking
            break;
            
        case MonsterState::Dead:
            // No movement when dead
            break;
    }
    
    // Update position
    setPosition(newPos);
}

void Monster::updateState(float deltaTime) {
    if (isDead()) return;
    
    MonsterState oldState = state;
    MonsterState newState = determineNextState();
    
    if (newState != oldState) {
        setState(newState);
    }
}

void Monster::findNewTarget() {
    if (!playerTarget) return;
    
    // For now, just target the player
    targetPosition = playerTarget->getPosition();
}

void Monster::moveTowardsTarget(float deltaTime) {
    if (!playerTarget) return;
    
    Vec3 currentPos = getPosition();
    Vec3 targetPos = playerTarget->getPosition();
    
    // Calculate direction to target
    Vec3 direction = targetPos - currentPos;
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    if (distance > 0.1f) {
        // Normalize direction
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
        
        // Move towards target
        float moveDistance = moveSpeed * deltaTime;
        if (distance < moveDistance) {
            setPosition(targetPos);
        } else {
            Vec3 newPos = currentPos + direction * moveDistance;
            setPosition(newPos);
        }
    }
}

void Monster::patrol(float deltaTime) {
    moveTimer += deltaTime;
    
    // Change patrol direction every few seconds
    if (moveTimer > 3.0f) {
        targetPosition = getRandomPatrolPosition();
        moveTimer = 0.0f;
    }
    
    // Move towards current patrol target
    Vec3 currentPos = getPosition();
    Vec3 direction = targetPosition - currentPos;
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    if (distance > 0.1f) {
        // Normalize direction
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
        
        // Move towards target
        float moveDistance = moveSpeed * deltaTime;
        if (distance < moveDistance) {
            setPosition(targetPosition);
        } else {
            Vec3 newPos = currentPos + direction * moveDistance;
            setPosition(newPos);
        }
    }
}

void Monster::attack() {
    if (!playerTarget) return;
    
    float currentTime = 0.0f; // TODO: Get actual game time
    if (currentTime - lastAttackTime >= attackCooldown) {
        // TODO: Implement actual attack logic
        std::cout << "Monster " << getName() << " attacks!" << std::endl;
        lastAttackTime = currentTime;
    }
}

void Monster::setState(MonsterState newState) {
    if (state == newState) return;
    
    MonsterState oldState = state;
    state = newState;
    
    // Reset timers on state change
    resetTimers();
    
    // Call state change callback
    onStateChange(oldState, newState);
}

void Monster::setHealth(float newHealth) {
    health = std::max(0.0f, std::min(maxHealth, newHealth));
    
    if (health <= 0.0f && !isDead()) {
        die();
    }
}

void Monster::flashDamage() {
    isFlashing = true;
    damageFlashTimer = 0.3f; // Flash for 0.3 seconds
}

void Monster::updateVisualEffects(float deltaTime) {
    updateDamageFlash(deltaTime);
}

Vec3 Monster::getCurrentColor() const {
    if (isFlashing) {
        return damageColor;
    }
    return originalColor;
}

void Monster::configureMonster(MonsterType monsterType) {
    type = monsterType;
    
    switch (type) {
        case MonsterType::Xenomorph:
            maxHealth = 100.0f;
            moveSpeed = 2.0f;
            attackRange = 2.0f;
            attackDamage = 25.0f;
            detectionRange = 10.0f;
            originalColor = Vec3(0.5f, 0.14f, 0.58f); // Purple
            break;
            
        case MonsterType::Runner:
            maxHealth = 50.0f;
            moveSpeed = 4.0f;
            attackRange = 1.5f;
            attackDamage = 15.0f;
            detectionRange = 15.0f;
            originalColor = Vec3(0.2f, 0.8f, 0.2f); // Green
            break;
            
        case MonsterType::Tank:
            maxHealth = 200.0f;
            moveSpeed = 1.0f;
            attackRange = 3.0f;
            attackDamage = 40.0f;
            detectionRange = 8.0f;
            originalColor = Vec3(0.8f, 0.2f, 0.2f); // Red
            break;
    }
    
    health = maxHealth;
}

float Monster::getDistanceToPlayer() const {
    if (!playerTarget) return 999999.0f;
    
    Vec3 monsterPos = getPosition();
    Vec3 playerPos = playerTarget->getPosition();
    
    Vec3 diff = playerPos - monsterPos;
    return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

bool Monster::canSeePlayer() const {
    if (!playerTarget) return false;
    
    float distance = getDistanceToPlayer();
    return distance <= detectionRange;
}

bool Monster::isInAttackRange() const {
    if (!playerTarget) return false;
    
    float distance = getDistanceToPlayer();
    return distance <= attackRange;
}

Vec3 Monster::getRandomPatrolPosition() const {
    Vec3 currentPos = getPosition();
    
    // Generate random offset within patrol radius
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    float distance = (float)(rand() % (int)patrolRadius);
    
    Vec3 offset;
    offset.x = cos(angle) * distance;
    offset.z = sin(angle) * distance;
    offset.y = 0.0f; // Keep on same height
    
    return currentPos + offset;
}

void Monster::resetTimers() {
    moveTimer = 0.0f;
    stateTimer = 0.0f;
}

void Monster::onDamage(float damage, GameObject* attacker) {
    // Override point for custom damage behavior
}

void Monster::onDeath() {
    // Override point for custom death behavior
}

void Monster::onStateChange(MonsterState oldState, MonsterState newState) {
    // Override point for custom state change behavior
}

void Monster::setupMonsterMesh() {
    // Load the actual Xenomorph model
    std::string modelPath = "Resources/Objects/Xenomorph/model.obj";
    
    std::cout << "Loading Xenomorph model from: " << modelPath << std::endl;
    
    // Load the OBJ model data
    OBJMeshData meshData = OBJLoader::loadOBJ(modelPath, 1.0f); // Scale of 1.0
    
    // Load materials from MTL file
    std::string mtlPath = MaterialLoader::getMTLPathFromOBJ(modelPath);
    std::cout << "Trying to load MTL file from: " << mtlPath << std::endl;
    
    if (MaterialLoader::isValidMTLFile(mtlPath)) {
        monsterMaterials = MaterialLoader::loadMTL(mtlPath);
        std::cout << "Loaded " << monsterMaterials.getMaterialCount() << " materials for monster" << std::endl;
        
        // Create material groups for multi-material rendering
        createMaterialGroups(meshData);
    } else {
        std::cout << "Warning: MTL file not found at " << mtlPath << std::endl;
        std::cout << "Trying alternative path: Resources/Objects/Xenomorph/materials.mtl" << std::endl;
        
        // Try alternative path
        std::string altMtlPath = "Resources/Objects/Xenomorph/materials.mtl";
        if (MaterialLoader::isValidMTLFile(altMtlPath)) {
            monsterMaterials = MaterialLoader::loadMTL(altMtlPath);
            std::cout << "Loaded " << monsterMaterials.getMaterialCount() << " materials from alternative path" << std::endl;
            createMaterialGroups(meshData);
        } else {
            std::cout << "Failed to load materials from both paths" << std::endl;
        }
    }
    
    if (!meshData.isValid()) {
        std::cerr << "Failed to load Xenomorph model for '" << getName() << "', falling back to cube" << std::endl;
        
        // Fallback to simple cube if model loading fails
        std::vector<float> vertices = {
            // Front face
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            
            // Back face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f
        };
        
        std::vector<unsigned int> indices = {
            // Front face
            0, 1, 2,  2, 3, 0,
            // Back face
            5, 4, 7,  7, 6, 5,
            // Left face
            4, 0, 3,  3, 7, 4,
            // Right face
            1, 5, 6,  6, 2, 1,
            // Top face
            3, 2, 6,  6, 7, 3,
            // Bottom face
            4, 5, 1,  1, 0, 4
        };
        
        mesh = std::make_unique<Mesh>();
        if (!mesh->createMesh(vertices, indices)) {
            std::cerr << "Failed to create fallback cube mesh for '" << getName() << "'" << std::endl;
        } else {
            std::cout << "Created fallback cube mesh for '" << getName() << "'" << std::endl;
        }
        return;
    }
    
    // Create mesh from loaded data
    mesh = std::make_unique<Mesh>();
    
    // Extract position and texture coordinates from OBJ data for basic renderer
    // OBJ format: [pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, texCoord.u, texCoord.v]
    // We need: [pos.x, pos.y, pos.z, texCoord.u, texCoord.v] for createMeshWithTexCoords
    std::vector<float> basicVertexData;
    basicVertexData.reserve((meshData.vertices.size() / 8) * 5); // Convert 8 floats per vertex to 5
    
    for (size_t i = 0; i < meshData.vertices.size(); i += 8) {
        // Position (3 floats)
        basicVertexData.push_back(meshData.vertices[i]);     // pos.x
        basicVertexData.push_back(meshData.vertices[i + 1]); // pos.y
        basicVertexData.push_back(meshData.vertices[i + 2]); // pos.z
        
        // Texture coordinates (2 floats) - skip normals
        basicVertexData.push_back(meshData.vertices[i + 6]); // texCoord.u
        basicVertexData.push_back(meshData.vertices[i + 7]); // texCoord.v
    }
    
    std::cout << "=== XENOMORPH MESH CONVERSION ===" << std::endl;
    std::cout << "Original OBJ data: " << meshData.vertices.size() << " floats" << std::endl;
    std::cout << "Converted data: " << basicVertexData.size() << " floats" << std::endl;
    std::cout << "Vertices: " << (meshData.vertices.size() / 8) << ", Indices: " << meshData.indices.size() << std::endl;
    std::cout << "First few vertices: ";
    for (int i = 0; i < std::min(10, (int)basicVertexData.size()); i++) {
        std::cout << basicVertexData[i] << " ";
    }
    std::cout << "..." << std::endl;
    
    if (!mesh->createMeshWithTexCoords(basicVertexData, meshData.indices)) {
        std::cerr << "Failed to create Xenomorph mesh for '" << getName() << "'" << std::endl;
    } else {
        std::cout << "Successfully loaded Xenomorph model for '" << getName() << "'" << std::endl;
        std::cout << "  Vertices: " << meshData.vertexCount << std::endl;
        std::cout << "  Triangles: " << meshData.triangleCount << std::endl;
        std::cout << "  Bounds: " << meshData.boundingBoxMin.x << "," << meshData.boundingBoxMin.y << "," << meshData.boundingBoxMin.z 
                  << " to " << meshData.boundingBoxMax.x << "," << meshData.boundingBoxMax.y << "," << meshData.boundingBoxMax.z << std::endl;
    }
}

void Monster::setupMonsterMaterial() {
    // Materials are now loaded in setupMonsterMesh() from the MTL file
    // Set default color for fallback rendering
    setColor(originalColor);
    
    std::cout << "Monster material setup complete for '" << getName() << "'" << std::endl;
    if (monsterMaterials.getMaterialCount() > 0) {
        std::cout << "  Using " << monsterMaterials.getMaterialCount() << " materials" << std::endl;
        std::cout << "  Material groups: " << materialGroups.size() << std::endl;
    } else {
        std::cout << "  Using fallback color: " << originalColor.x << ", " << originalColor.y << ", " << originalColor.z << std::endl;
    }
}

void Monster::createMaterialGroups(const OBJMeshData& objData) {
    // Implementation for creating material groups from OBJ data
    // Parse the OBJ data and create material groups based on material assignments
    std::cout << "Creating material groups from OBJ data..." << std::endl;
    
    // Clear existing material groups
    materialGroups.clear();
    
    // Create material groups based on loaded materials
    if (monsterMaterials.getMaterialCount() > 0 && !objData.faceMaterials.empty()) {
        auto materialNames = monsterMaterials.getMaterialNames();
        
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
                // Each face has 3 vertices, so we need to add 3 indices
                size_t baseIndex = faceIndex * 3;
                if (baseIndex + 2 < objData.indices.size()) {
                    it->second.push_back(objData.indices[baseIndex]);
                    it->second.push_back(objData.indices[baseIndex + 1]);
                    it->second.push_back(objData.indices[baseIndex + 2]);
                }
            }
        }
        
        // Create material groups from the parsed data
        for (const auto& materialName : materialNames) {
            const Material* mat = monsterMaterials.getMaterial(materialName);
            if (mat && !materialIndexMap[materialName].empty()) {
                MaterialGroup group;
                group.materialName = materialName;
                group.indices = materialIndexMap[materialName];
                group.color = mat->diffuse;
                materialGroups.push_back(group);
                
                std::cout << "  Material group '" << materialName << "': " 
                          << group.indices.size() << " indices, color(" 
                          << group.color.x << ", " << group.color.y << ", " << group.color.z << ")" << std::endl;
            }
        }
    }
    
    std::cout << "Created " << materialGroups.size() << " material groups for monster" << std::endl;
}

void Monster::updateDamageFlash(float deltaTime) {
    if (isFlashing) {
        damageFlashTimer -= deltaTime;
        if (damageFlashTimer <= 0.0f) {
            isFlashing = false;
        }
    }
}

bool Monster::shouldChangeState() const {
    if (isDead()) return false;
    
    // Check if player is in range
    if (canSeePlayer()) {
        if (isInAttackRange()) {
            return state != MonsterState::Attacking;
        } else {
            return state != MonsterState::Chasing;
        }
    }
    
    // If no player in sight, patrol
    return state != MonsterState::Patrolling;
}

MonsterState Monster::determineNextState() {
    if (isDead()) return MonsterState::Dead;
    
    if (canSeePlayer()) {
        if (isInAttackRange()) {
            return MonsterState::Attacking;
        } else {
            return MonsterState::Chasing;
        }
    }
    
    return MonsterState::Patrolling;
}

// MonsterSpawner implementation
MonsterSpawner::MonsterSpawner()
    : maxMonsters(5),
      spawnInterval(3.0f),
      lastSpawnTime(0.0f),
      spawnRadius(20.0f),
      spawnCenter(0.0f, 0.0f, 0.0f),
      playerTarget(nullptr),
      gameScene(nullptr) {
    
    // Add default spawn points
    spawnPoints.push_back(Vec3(-10.0f, 0.0f, -10.0f));
    spawnPoints.push_back(Vec3(10.0f, 0.0f, -10.0f));
    spawnPoints.push_back(Vec3(-10.0f, 0.0f, 10.0f));
    spawnPoints.push_back(Vec3(10.0f, 0.0f, 10.0f));
    spawnPoints.push_back(Vec3(0.0f, 0.0f, -15.0f));
    spawnPoints.push_back(Vec3(0.0f, 0.0f, 15.0f));
    
    // Add default monster types
    monsterTypes.push_back(MonsterType::Xenomorph);
    monsterTypes.push_back(MonsterType::Runner);
    monsterTypes.push_back(MonsterType::Tank);
}

void MonsterSpawner::initialize(Scene* scene, GameObject* player) {
    gameScene = scene;
    playerTarget = player;
    
    std::cout << "MonsterSpawner initialized" << std::endl;
}

void MonsterSpawner::update(float deltaTime) {
    if (!gameScene || !playerTarget) return;
    
    // Update spawn timer
    lastSpawnTime += deltaTime;
    
    // Check if we should spawn a new monster
    if (shouldSpawnMonster()) {
        spawnRandomMonster();
        lastSpawnTime = 0.0f;
    }
    
    // Update all active monsters
    for (auto& monster : activeMonsters) {
        if (monster && monster->isAlive()) {
            monster->update(deltaTime);
        }
    }
    
    // Remove dead monsters
    removeDeadMonsters();
    
    // Debug: Print monster status every few seconds
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 2.0f) {
        std::cout << "=== MONSTER DEBUG ===" << std::endl;
        std::cout << "Active monsters: " << activeMonsters.size() << std::endl;
        for (size_t i = 0; i < activeMonsters.size(); ++i) {
            if (activeMonsters[i]) {
                Vec3 pos = activeMonsters[i]->getPosition();
                std::cout << "  Monster_" << i << ": pos(" << pos.x << ", " << pos.y << ", " << pos.z 
                          << ") alive=" << activeMonsters[i]->isAlive() 
                          << " active=" << activeMonsters[i]->getActive() << std::endl;
            }
        }
        debugTimer = 0.0f;
    }
}

void MonsterSpawner::cleanup() {
    clearAllMonsters();
    gameScene = nullptr;
    playerTarget = nullptr;
}

void MonsterSpawner::spawnMonster(MonsterType type) {
    if (!gameScene || activeMonsters.size() >= maxMonsters) return;
    
    Vec3 spawnPos = getRandomSpawnPosition();
    spawnMonsterAt(spawnPos, type);
}

void MonsterSpawner::spawnMonsterAt(const Vec3& position, MonsterType type) {
    if (!gameScene || activeMonsters.size() >= maxMonsters) return;
    
    std::string monsterName = "Monster_" + std::to_string(activeMonsters.size());
    auto monster = std::make_unique<Monster>(monsterName, type);
    
    // Set player target
    monster->setPlayerTarget(playerTarget);
    
    // Initialize and spawn
    if (monster->initialize()) {
        monster->spawn(position);
        
        // Add monster to the scene so it's included in visibility system
        Monster* monsterPtr = monster.get();
        gameScene->addGameObject(std::move(monster));
        
        // Store reference in our active monsters list
        activeMonsters.push_back(monsterPtr);
        
        std::cout << "=== MONSTER SPAWNED ===" << std::endl;
        std::cout << "Spawned " << monsterName << " at " << position.x << ", " << position.y << ", " << position.z << std::endl;
        std::cout << "Monster entity flag: " << (monsterPtr->getEntity() ? "true" : "false") << std::endl;
        std::cout << "Monster active: " << (monsterPtr->getActive() ? "true" : "false") << std::endl;
        std::cout << "Total active monsters: " << activeMonsters.size() << std::endl;
    }
}

void MonsterSpawner::spawnRandomMonster() {
    MonsterType type = getRandomMonsterType();
    spawnMonster(type);
}

void MonsterSpawner::clearAllMonsters() {
    // Clear our reference list - scene will handle cleanup
    activeMonsters.clear();
}

void MonsterSpawner::addSpawnPoint(const Vec3& point) {
    spawnPoints.push_back(point);
}

void MonsterSpawner::addMonsterType(MonsterType type) {
    monsterTypes.push_back(type);
}

void MonsterSpawner::removeDeadMonsters() {
    activeMonsters.erase(
        std::remove_if(activeMonsters.begin(), activeMonsters.end(),
            [](const Monster* monster) {
                return !monster || monster->isDead();
            }),
        activeMonsters.end()
    );
}

Vec3 MonsterSpawner::getRandomSpawnPosition() const {
    if (spawnPoints.empty()) {
        // Generate random position around spawn center
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float distance = (float)(rand() % (int)spawnRadius);
        
        Vec3 pos = spawnCenter;
        pos.x += cos(angle) * distance;
        pos.z += sin(angle) * distance;
        return pos;
    }
    
    // Pick random spawn point
    int index = rand() % spawnPoints.size();
    return spawnPoints[index];
}

MonsterType MonsterSpawner::getRandomMonsterType() const {
    if (monsterTypes.empty()) {
        return MonsterType::Xenomorph;
    }
    
    int index = rand() % monsterTypes.size();
    return monsterTypes[index];
}

bool MonsterSpawner::shouldSpawnMonster() const {
    return activeMonsters.size() < maxMonsters && lastSpawnTime >= spawnInterval;
}

} // namespace Engine
