/**
 * Projectile.h - Modular Projectile System for Shooting Mechanics
 * 
 * OVERVIEW:
 * Implements a flexible projectile system that supports different projectile types,
 * collision detection, and configurable behavior within the GameObject-Renderer structure.
 * 
 * FEATURES:
 * - Multiple projectile types (bullet, rocket, laser, etc.)
 * - Configurable physics properties (speed, gravity, bounce, etc.)
 * - Collision detection with different object types
 * - Damage system with different damage types
 * - Particle effects and trails
 * - Sound effects integration
 * - Network synchronization support
 */

#pragma once
#include "GameObject.h"
#include "../Math/Math.h"
#include "../Rendering/Material.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <iostream>

namespace Engine {

// Forward declarations
class CollisionSystem;
class ParticleSystem;
class AudioSystem;
class GameObject;

/**
 * ProjectileType - Enumeration of different projectile types
 */
enum class ProjectileType {
    Bullet,         // Standard bullet (fast, straight)
    Rocket,         // Rocket with explosion
    Laser,          // Instant hit laser
    Grenade,        // Bouncing grenade
    Plasma,         // Energy projectile
    Arrow,          // Bow/crossbow projectile
    Custom          // Custom projectile type
};

/**
 * DamageType - Different types of damage
 */
enum class DamageType {
    Physical,       // Standard physical damage
    Explosive,      // Explosion damage
    Energy,         // Energy/plasma damage
    Fire,           // Fire damage
    Ice,            // Ice damage
    Poison,         // Poison damage
    Custom          // Custom damage type
};

/**
 * CollisionLayer - Defines what objects the projectile can collide with
 */
enum class CollisionLayer {
    None = 0,
    Terrain = 1 << 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
    Vehicle = 1 << 3,
    Destructible = 1 << 4,
    Water = 1 << 5,
    All = 0x3F  // Use 6 bits instead of 32 bits
};

/**
 * ProjectileConfig - Configuration structure for projectile behavior
 */
struct ProjectileConfig {
    // Basic properties
    ProjectileType type = ProjectileType::Bullet;
    float speed = 50.0f;
    float maxDistance = 100.0f;
    float lifetime = 5.0f;
    float size = 0.1f;
    
    // Physics properties
    bool affectedByGravity = false;
    float gravity = 9.81f;
    bool bounces = false;
    int maxBounces = 0;
    float bounceEnergy = 0.5f;
    bool ricochets = false;
    float ricochetChance = 0.0f;
    
    // Damage properties
    DamageType damageType = DamageType::Physical;
    float damage = 25.0f;
    float armorPenetration = 0.0f;
    bool explosive = false;
    float explosionRadius = 0.0f;
    float explosionForce = 0.0f;
    
    // Visual properties
    Vec3 color = Vec3(1.0f, 1.0f, 0.0f);
    bool hasTrail = false;
    float trailLength = 2.0f;
    bool hasGlow = false;
    float glowIntensity = 1.0f;
    
    // Audio properties
    std::string fireSound = "";
    std::string impactSound = "";
    std::string flybySound = "";
    
    // Collision properties
    CollisionLayer collisionLayers = CollisionLayer::All;
    bool destroyOnCollision = true;
    bool penetrateTargets = false;
    int maxPenetrations = 0;
    
    // Custom behavior - using void* to avoid circular dependencies
    std::function<void(void*, GameObject*)> onHitCallback = nullptr;
    std::function<void(void*)> onDestroyCallback = nullptr;
    std::function<void(void*, float)> onUpdateCallback = nullptr;
};

/**
 * Projectile - Main projectile class extending GameObject
 */
class Projectile : public GameObject {
private:
    // Projectile state
    ProjectileConfig config;
    Vec3 velocity;
    Vec3 startPosition;
    float distanceTraveled;
    float currentLifetime;
    int bounceCount;
    int penetrationCount;
    bool isDestroyed;
    
    // Trail system
    struct TrailPoint {
        Vec3 position;
        float time;
    };
    std::vector<TrailPoint> trailPoints;
    
    // Collision detection
    CollisionSystem* collisionSystem;
    
    // Effects systems
    ParticleSystem* particleSystem;
    AudioSystem* audioSystem;
    
    // Owner information
    GameObject* owner;
    std::string ownerTag;

public:
    // Constructor/Destructor
    Projectile(const std::string& name, const ProjectileConfig& config);
    virtual ~Projectile() = default;
    
    // Core functionality
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    
    // Override model matrix to provide world space orientation
    Mat4 getModelMatrix() const override;
    
    // Projectile control
    void fire(const Vec3& position, const Vec3& direction, GameObject* owner = nullptr);
    void destroy();
    void bounce(const Vec3& normal, float energy = 1.0f);
    void ricochet(const Vec3& normal);
    
    // Configuration
    void setConfig(const ProjectileConfig& newConfig) { config = newConfig; }
    const ProjectileConfig& getConfig() const { return config; }
    
    // State queries
    bool isActive() const { return GameObject::getActive() && !isDestroyed; }
    float getDistanceTraveled() const { return distanceTraveled; }
    float getLifetime() const { return currentLifetime; }
    const Vec3& getVelocity() const { return velocity; }
    
    // Owner management
    void setOwner(GameObject* newOwner) { owner = newOwner; }
    GameObject* getOwner() const { return owner; }
    void setOwnerTag(const std::string& tag) { ownerTag = tag; }
    const std::string& getOwnerTag() const { return ownerTag; }
    
    // Trail management
    void addTrailPoint(const Vec3& position);
    void clearTrail();
    const std::vector<TrailPoint>& getTrail() const { return trailPoints; }
    
    // Collision detection
    bool checkCollision(GameObject* target);
    void handleCollision(GameObject* target);
    
    // Damage calculation
    float calculateDamage(GameObject* target);
    void applyDamage(GameObject* target, float damage);
    
    // Effects
    void spawnImpactEffect(const Vec3& position, const Vec3& normal);
    void spawnTrailEffect();
    void playSound(const std::string& soundName);
    
    // Physics
    void updatePhysics(float deltaTime);
    void applyGravity(float deltaTime);
    void checkDistanceLimit();
    void checkLifetime();
    void checkMonsterCollisions();
    
    // Utility
    Vec3 predictPosition(float timeAhead) const;
    bool isInRange(const Vec3& targetPosition) const;
    float getTimeToTarget(const Vec3& targetPosition) const;
    
protected:
    // Override points for custom behavior
    virtual void onHit(GameObject* target);
    virtual void onDestroy();
    virtual void onUpdate(float deltaTime);
    virtual void onBounce(const Vec3& normal);
    virtual void onRicochet(const Vec3& normal);
    
    // Internal helpers
    void updateTrail(float deltaTime);
    void cleanupTrail();
    bool shouldDestroy() const;
    void performExplosion(const Vec3& position);
    void renderTrail(const Renderer& renderer, const Camera& camera);
    void setupMesh() override;
};

/**
 * ProjectileFactory - Factory for creating different projectile types
 */
class ProjectileFactory {
public:
    static std::unique_ptr<Projectile> createProjectile(ProjectileType type, const std::string& name = "");
    static ProjectileConfig getDefaultConfig(ProjectileType type);
    
    // Predefined projectile configurations
    static ProjectileConfig createBulletConfig();
    static ProjectileConfig createRocketConfig();
    static ProjectileConfig createLaserConfig();
    static ProjectileConfig createGrenadeConfig();
    static ProjectileConfig createPlasmaConfig();
    static ProjectileConfig createArrowConfig();
};

/**
 * ProjectileManager - Manages all active projectiles
 */
class ProjectileManager {
private:
    std::vector<std::unique_ptr<Projectile>> activeProjectiles;
    CollisionSystem* collisionSystem;
    ParticleSystem* particleSystem;
    AudioSystem* audioSystem;
    
public:
    ProjectileManager();
    ~ProjectileManager();
    
    void initialize(CollisionSystem* collision, ParticleSystem* particles, AudioSystem* audio);
    void update(float deltaTime);
    void render(const Renderer& renderer, const Camera& camera);
    void cleanup();
    
    Projectile* createProjectile(const ProjectileConfig& config, const std::string& name = "");
    Projectile* createMonsterHunterProjectile(const std::string& name = "");
    void destroyProjectile(Projectile* projectile);
    void destroyAllProjectiles();
    
    const std::vector<std::unique_ptr<Projectile>>& getActiveProjectiles() const { return activeProjectiles; }
    size_t getActiveProjectileCount() const { return activeProjectiles.size(); }
    
    // Collision detection for all projectiles
    void checkAllCollisions(const std::vector<GameObject*>& gameObjects);
};

} // namespace Engine
