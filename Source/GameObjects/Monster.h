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
#include "HealthBar.h"
#include <memory>
#include <vector>

namespace Engine {

// Forward declarations
class Projectile;
class Scene;
class Player;

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
    Alert,          // Aware of player but not yet chasing
    Chasing,        // Moving towards player
    Attacking,      // Performing attack
    Stunned,        // Temporarily disabled (e.g., after taking damage)
    Retreating,     // Moving away from player when low health
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
    float dangerRange;  // Range at which monster becomes aggressive and charges
    
    // Movement
    Vec3 targetPosition;
    float moveTimer;
    float stateTimer;
    float patrolRadius;
    float chargeSpeed;      // Speed when charging at player
    float baseSpeed;        // Normal patrol speed
    bool isCharging;        // Whether monster is in charging mode
    
    // AI behavior
    float lastAttackTime;
    float attackCooldown;
    float alertTimer;           // Time spent in alert state
    float stunTimer;            // Time remaining stunned
    float retreatTimer;         // Time spent retreating
    float lastPlayerSeenTime;   // When player was last seen
    Vec3 lastKnownPlayerPos;    // Last known player position
    bool hasLineOfSight;        // Whether monster can see player
    float aggressionLevel;      // How aggressive the monster is (0.0-1.0)
    float fearLevel;            // How scared the monster is (0.0-1.0)
    
    // Pathfinding
    Vec3 currentPathDirection;  // Current movement direction
    float pathUpdateTimer;      // Timer for pathfinding updates
    float pathUpdateInterval;   // How often to update pathfinding
    bool isStuck;              // Whether monster is stuck
    float stuckTimer;          // How long monster has been stuck
    Vec3 lastPosition;         // Last position for stuck detection
    
    // Group behavior
    bool inGroup;              // Whether monster is part of a group
    float groupAlertRadius;    // Radius to alert nearby monsters
    float groupCoordinationTimer; // Timer for group coordination updates
    bool hasAlertedGroup;      // Whether this monster has alerted the group
    Vec3 groupTarget;          // Shared target for group coordination
    
    // Visual effects
    float damageFlashTimer;
    Vec3 originalColor;
    Vec3 damageColor;
    bool isFlashing;
    
    // Enhanced visual effects
    float stateChangeFlashTimer;
    bool isStateFlashing;
    Vec3 alertColor;           // Color when alert
    Vec3 chaseColor;           // Color when chasing
    Vec3 attackColor;          // Color when attacking
    float pulseTimer;          // Timer for pulsing effects
    bool isPulsing;            // Whether monster is pulsing
    float pulseSpeed;          // Speed of pulsing effect
    
    // Loot system
    int experienceReward;      // Experience points awarded when killed
    int scoreReward;           // Score points awarded when killed
    bool hasDroppedLoot;       // Whether loot has been dropped
    
    // Death animation
    float deathAnimationTimer; // Timer for death animation
    float deathAnimationDuration; // Duration of death animation
    bool isDeathAnimating;     // Whether death animation is playing
    Vec3 deathScale;           // Scale during death animation
    Vec3 originalScale;        // Original scale before death
    
    // Deletion management
    bool markedForDeletion;
    float deletionTimer;
    static constexpr float DELETION_DELAY = 0.5f; // 0.5 seconds delay
    
    // References
    GameObject* playerTarget;
    
    // Health bar (simplified - no separate object)
    bool showHealthBar;
    float healthBarWidth;
    float healthBarHeight;
    float healthBarOffsetY;
    
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
    void setRotationFromDirection(const Vec3& direction);
    
    // Enhanced AI methods
    void updateLineOfSight();
    void updateAggression(float deltaTime);
    void updateFear(float deltaTime);
    void retreat(float deltaTime);
    void becomeAlert();
    void becomeStunned(float duration);
    bool canSeePlayer() const;
    bool hasPlayerInSight() const;
    float getDistanceToLastKnownPlayerPos() const;
    
    // Pathfinding methods
    void updatePathfinding(float deltaTime);
    Vec3 findPathToTarget(const Vec3& target);
    Vec3 avoidObstacles(const Vec3& direction);
    bool isPathBlocked(const Vec3& from, const Vec3& to) const;
    Vec3 getRandomDirection() const;
    
    // Group behavior methods
    void updateGroupBehavior(float deltaTime);
    void communicateWithNearbyMonsters();
    void respondToGroupAlert();
    void coordinateAttack();
    std::vector<Monster*> getNearbyMonsters(float radius) const;
    void alertNearbyMonsters();
    bool isInGroup() const;
    void joinGroup();
    void leaveGroup();
    
    // State management
    void setState(MonsterState newState);
    MonsterState getState() const { return state; }
    std::string getStateName(MonsterState state) const;
    bool isDead() const { 
        try {
            // CRASH PREVENTION: Multiple safety checks
            if (!getActive()) return true; // If inactive, consider dead
            if (health <= 0.0f) return true; // If no health, consider dead
            return state == MonsterState::Dead; 
        } catch (...) {
            return true; // If we can't access anything, assume dead to prevent crashes
        }
    }
    bool isAlive() const { return health > 0.0f; }
    
    // Deletion management
    bool shouldBeDeleted() const { return markedForDeletion && deletionTimer >= DELETION_DELAY; }
    void markForDeletion();
    
    // Health and combat
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercentage() const { return health / maxHealth; }
    void setHealth(float newHealth);
    
    // Health bar management
    void setShowHealthBar(bool show) { showHealthBar = show; }
    bool getShowHealthBar() const { return showHealthBar; }
    void updateHealthBar();
    void renderHealthBar(const Renderer& renderer, const Camera& camera);
    
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
    
    // Enhanced visual effects
    void flashStateChange();
    void updateStateVisualEffects(float deltaTime);
    void updatePulsingEffect(float deltaTime);
    Vec3 getStateColor() const;
    void setPulsing(bool pulsing, float speed = 2.0f);
    
    // Configuration
    void configureMonster(MonsterType monsterType);
    
    // Utility
    float getDistanceToPlayer() const;
    bool isInAttackRange() const;
    bool isPlayerInDangerZone() const;  // New: Check if player is in danger zone
    Vec3 getRandomPatrolPosition() const;
    void resetTimers();
    
    // Enhanced utility methods
    float getAggressionLevel() const { return aggressionLevel; }
    float getFearLevel() const { return fearLevel; }
    void setAggressionLevel(float level) { aggressionLevel = std::max(0.0f, std::min(1.0f, level)); }
    void setFearLevel(float level) { fearLevel = std::max(0.0f, std::min(1.0f, level)); }
    bool isStunned() const { return stunTimer > 0.0f; }
    bool shouldRetreat() const { return getHealthPercentage() < 0.3f && fearLevel > 0.5f; }
    
    // Loot and rewards
    void dropLoot();
    int getExperienceReward() const;
    int getScoreReward() const;
    
    // Death animation
    void startDeathAnimation();
    void updateDeathAnimation(float deltaTime);
    
    // Collision system
    float getCollisionRadius() const;
    Vec3 getCollisionCenter() const;
    
    // Renderer selection
    virtual RendererType getPreferredRendererType() const override;
    


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
    
    // Enhanced internal helpers
    void updateTimers(float deltaTime);
    void updateBehaviorModifiers(float deltaTime);
    bool checkLineOfSightToPlayer() const;
    void updateLastKnownPlayerPosition();
    float calculateThreatLevel() const;
    void updateStuckDetection(const Vec3& oldPos, const Vec3& newPos, float deltaTime);
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
    
    // Wave system
    int currentWave;
    int monstersInCurrentWave;
    int monstersSpawnedInWave;
    float waveStartTime;
    float waveDuration;
    bool waveInProgress;
    float timeBetweenWaves;
    float lastWaveEndTime;
    
    // Difficulty scaling
    float difficultyLevel;
    float difficultyIncreaseRate;
    float lastDifficultyIncrease;
    float difficultyIncreaseInterval;
    
    // References
    GameObject* playerTarget;
    Scene* gameScene;

public:
    // Constructor/Destructor
    MonsterSpawner(Scene* scene, GameObject* player);
    ~MonsterSpawner() = default;
    void update(float deltaTime);
    void cleanup();
    
    // Spawning control
    void spawnMonster(MonsterType type = MonsterType::Xenomorph);
    void spawnMonsterAt(const Vec3& position, MonsterType type = MonsterType::Xenomorph);
    void spawnRandomMonster();
    void clearAllMonsters();
    
    // Configuration
    void setMaxMonsters(int max);
    void setSpawnInterval(float interval);
    void setSpawnRadius(float radius);
    void setSpawnCenter(const Vec3& center);
    void addSpawnPoint(const Vec3& point);
    void addMonsterType(MonsterType type);
    
    // Management
    const std::vector<Monster*>& getActiveMonsters() const;
    size_t getActiveMonsterCount() const;
    void removeDeadMonsters();
    void removeMonster(Monster* monster);
    
    // Wave system
    void startNewWave();
    void endCurrentWave();
    bool isWaveInProgress() const;
    int getCurrentWave() const;
    int getMonstersInCurrentWave() const;
    int getMonstersSpawnedInWave() const;
    float getWaveProgress() const;
    
    // Difficulty scaling
    void updateDifficulty(float deltaTime);
    float getDifficultyLevel() const;
    void setDifficultyLevel(float level);
    void increaseDifficulty(float amount = 0.1f);
    
    // Utility
    Vec3 getRandomSpawnPosition() const;
    MonsterType getRandomMonsterType() const;
    bool shouldSpawnMonster() const;
    bool shouldStartNewWave() const;
};

} // namespace Engine
