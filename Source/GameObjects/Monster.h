/**
 * Monster.h - Simplified Monster/Enemy System for Combat
 * 
 * OVERVIEW:
 * Implements basic monster enemies that can move around the world and be shot at.
 * Uses the Xenomorph model for visual representation.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Math/Math.h"
#include "../Engine/Rendering/Material.h"
#include "../Engine/Utils/OBJLoader.h"
#include <memory>
#include <vector>

namespace Engine {

// Forward declarations
class Projectile;
class Scene;

// Material group structure for multi-material rendering
struct MaterialGroup {
    std::string materialName;
    std::vector<unsigned int> indices; // Triangle indices for this material
    Vec3 color; // Material color
};

/**
 * MonsterType - Different types of monsters
 */
enum class MonsterType {
    Xenomorph,      // Standard alien monster
    Runner,         // Fast moving monster
    Tank            // Slow but tough monster
};

/**
 * MonsterState - Current state of the monster
 */
enum class MonsterState {
    Idle,           // Standing still
    Patrolling,     // Moving around
    Chasing,        // Moving towards player
    Attacking,      // Performing attack
    Dead            // Dead monster
};

/**
 * Monster - Main monster class extending GameObject
 */
class Monster : public GameObject {
private:
    // Monster properties
    MonsterType type;
    MonsterState state;
    float health;
    float maxHealth;
    float moveSpeed;
    float attackRange;
    float attackDamage;
    float detectionRange;
    
    // Movement
    Vec3 targetPosition;
    float moveTimer;
    float stateTimer;
    float patrolRadius;
    
    // AI behavior
    float lastAttackTime;
    float attackCooldown;
    
    // Visual effects
    float damageFlashTimer;
    Vec3 originalColor;
    Vec3 damageColor;
    bool isFlashing;
    
    // References
    GameObject* playerTarget;
    
    // Material system
    MaterialLibrary monsterMaterials;
    std::vector<MaterialGroup> materialGroups;

public:
    // Constructor/Destructor
    Monster(const std::string& name, MonsterType monsterType = MonsterType::Xenomorph);
    virtual ~Monster() = default;
    
    // Core functionality
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;
    
    // Monster control
    void spawn(const Vec3& position);
    void takeDamage(float damage, GameObject* attacker = nullptr);
    void die();
    
    // AI behavior
    void updateAI(float deltaTime);
    void updateMovement(float deltaTime);
    void updateState(float deltaTime);
    void findNewTarget();
    void moveTowardsTarget(float deltaTime);
    void patrol(float deltaTime);
    void attack();
    
    // State management
    void setState(MonsterState newState);
    MonsterState getState() const { return state; }
    bool isDead() const { return state == MonsterState::Dead; }
    bool isAlive() const { return health > 0.0f; }
    
    // Health and combat
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercentage() const { return health / maxHealth; }
    void setHealth(float newHealth);
    
    // Movement and positioning
    void setTargetPosition(const Vec3& position);
    Vec3 getTargetPosition() const { return targetPosition; }
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    float getMoveSpeed() const { return health > 0.0f ? moveSpeed : 0.0f; }
    
    // Targeting
    void setPlayerTarget(GameObject* player) { playerTarget = player; }
    GameObject* getPlayerTarget() const { return playerTarget; }
    
    // Visual effects
    void flashDamage();
    void updateVisualEffects(float deltaTime);
    Vec3 getCurrentColor() const;
    
    // Configuration
    void configureMonster(MonsterType monsterType);
    
    // Utility
    float getDistanceToPlayer() const;
    bool canSeePlayer() const;
    bool isInAttackRange() const;
    Vec3 getRandomPatrolPosition() const;
    void resetTimers();

protected:
    // Override points for custom behavior
    virtual void onDamage(float damage, GameObject* attacker);
    virtual void onDeath();
    virtual void onStateChange(MonsterState oldState, MonsterState newState);
    
    // Internal helpers
    void setupMonsterMesh();
    void setupMonsterMaterial();
    void createMaterialGroups(const OBJMeshData& objData);
    void updateDamageFlash(float deltaTime);
    bool shouldChangeState() const;
    MonsterState determineNextState();
};

/**
 * MonsterSpawner - Manages monster spawning and population
 */
class MonsterSpawner {
private:
    std::vector<Monster*> activeMonsters; // Raw pointers since scene owns the monsters
    std::vector<Vec3> spawnPoints;
    std::vector<MonsterType> monsterTypes;
    
    // Spawning configuration
    int maxMonsters;
    float spawnInterval;
    float lastSpawnTime;
    float spawnRadius;
    Vec3 spawnCenter;
    
    // References
    GameObject* playerTarget;
    Scene* gameScene;

public:
    // Constructor/Destructor
    MonsterSpawner();
    ~MonsterSpawner() = default;
    
    // Core functionality
    void initialize(Scene* scene, GameObject* player);
    void update(float deltaTime);
    void cleanup();
    
    // Spawning control
    void spawnMonster(MonsterType type = MonsterType::Xenomorph);
    void spawnMonsterAt(const Vec3& position, MonsterType type = MonsterType::Xenomorph);
    void spawnRandomMonster();
    void clearAllMonsters();
    
    // Configuration
    void setMaxMonsters(int max) { maxMonsters = max; }
    void setSpawnInterval(float interval) { spawnInterval = interval; }
    void setSpawnRadius(float radius) { spawnRadius = radius; }
    void setSpawnCenter(const Vec3& center) { spawnCenter = center; }
    void addSpawnPoint(const Vec3& point);
    void addMonsterType(MonsterType type);
    
    // Management
    const std::vector<Monster*>& getActiveMonsters() const { return activeMonsters; }
    size_t getActiveMonsterCount() const { return activeMonsters.size(); }
    void removeDeadMonsters();
    
    // Utility
    Vec3 getRandomSpawnPosition() const;
    MonsterType getRandomMonsterType() const;
    bool shouldSpawnMonster() const;
};

} // namespace Engine
