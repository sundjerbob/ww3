/**
 * Projectile.cpp - Implementation of Modular Projectile System
 */

#include "Projectile.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/Shader.h"
#include "../Math/Math.h"
#include "../../GameObjects/Monster.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Engine {

// Forward declarations for systems that will be implemented later
class CollisionSystem {};
class ParticleSystem {};
class AudioSystem {};

// Projectile implementation
Projectile::Projectile(const std::string& name, const ProjectileConfig& config)
    : GameObject(name),
      config(config),
      velocity(0.0f, 0.0f, 0.0f),
      startPosition(0.0f, 0.0f, 0.0f),
      distanceTraveled(0.0f),
      currentLifetime(0.0f),
      bounceCount(0),
      penetrationCount(0),
      isDestroyed(false),
      collisionSystem(nullptr),
      particleSystem(nullptr),
      audioSystem(nullptr),
      owner(nullptr) {
    
    // Set projectile to be a world entity so it renders in 3D space
    setEntity(true);
    
    // Set initial scale based on projectile size
    setScale(Vec3(config.size, config.size, config.size));
    
    // Set color
    setColor(config.color);
}

bool Projectile::initialize() {
    
    if (!GameObject::initialize()) {
        return false;
    }
    
    return true;
}

void Projectile::setupMesh() {
    // Create a bullet-shaped mesh for the projectile
    // This makes projectiles look more realistic and professional
    
    // Bullet shape: cylinder with pointed tip
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    const int segments = 12;  // Number of segments for the cylinder
    const float radius = 0.1f;  // Bullet radius
    const float length = 0.4f;  // Bullet length
    const float tipLength = 0.15f;  // Pointed tip length
    
    // Generate cylinder body vertices with texture coordinates
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        float u = static_cast<float>(i) / segments; // Texture coordinate U
        
        // Back of bullet (flat end) - position + texture coords
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-length/2);
        vertices.push_back(u);
        vertices.push_back(0.0f); // V coordinate for back
        
        // Front of bullet body (before tip) - position + texture coords
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(length/2 - tipLength);
        vertices.push_back(u);
        vertices.push_back(0.8f); // V coordinate for front body
    }
    
    // Generate tip vertices with texture coordinates
    for (int i = 0; i <= segments; i++) {
        float angle = (2.0f * static_cast<float>(M_PI) * i) / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        float u = static_cast<float>(i) / segments; // Texture coordinate U
        
        // Front of bullet body (before tip) - position + texture coords
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(length/2 - tipLength);
        vertices.push_back(u);
        vertices.push_back(0.8f); // V coordinate for front body
        
        // Tip of bullet (point) - position + texture coords
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(length/2);
        vertices.push_back(0.5f); // Center U coordinate for tip
        vertices.push_back(1.0f); // V coordinate for tip
    }
    
    // Generate indices for cylinder body
    for (int i = 0; i < segments; i++) {
        int base = i * 2;
        
        // Side face (quad as two triangles)
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        
        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }
    
    // Generate indices for tip
    int tipBase = (segments + 1) * 2;
    for (int i = 0; i < segments; i++) {
        int base = tipBase + i * 2;
        
        // Tip face (triangle)
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back((i + 1) % segments + tipBase);
    }
    
    // Add end caps
    for (int i = 0; i < segments - 2; i++) {
        // Back cap
        indices.push_back(0);
        indices.push_back((i + 1) * 2);
        indices.push_back((i + 2) * 2);
        
        // Front cap (before tip)
        indices.push_back(1);
        indices.push_back((i + 1) * 2 + 1);
        indices.push_back((i + 2) * 2 + 1);
    }
    
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMeshWithTexCoords(vertices, indices)) {
        std::cout << "ERROR: Failed to create bullet mesh for projectile " << getName() << std::endl;
    }
}

void Projectile::update(float deltaTime) {
    if (isDestroyed) return;
    
    // Debug output to see if projectiles are updating (reduced frequency)
    static int updateCount = 0;
    updateCount++;
    if (updateCount % 600 == 0) { // Print every 600 frames (about every 10 seconds)
    }
    
    // Update lifetime
    currentLifetime += deltaTime;
    
    // Debug lifetime update (reduced frequency)
    static int lifetimeDebugCount = 0;
    lifetimeDebugCount++;
    if (lifetimeDebugCount % 300 == 0) { // Print every 300 frames (about every 5 seconds)
    }
    
    // Check lifetime limit
    checkLifetime();
    
    // Update physics
    updatePhysics(deltaTime);
    
    // Update trail
    if (config.hasTrail) {
        updateTrail(deltaTime);
    }
    
    // Check distance limit
    checkDistanceLimit();
    
    // Check for collisions with monsters
    checkMonsterCollisions();
    
    // Call custom update callback
    if (config.onUpdateCallback) {
        config.onUpdateCallback(this, deltaTime);
    }
    
    // Call virtual update method
    onUpdate(deltaTime);
    
    // Call base class update
    GameObject::update(deltaTime);
}

void Projectile::render(const Renderer& renderer, const Camera& camera) {
    if (isDestroyed) return;
    
    // Debug output to see if projectiles are rendering
    static int renderCount = 0;
    renderCount++;
    if (renderCount % 60 == 0) { // Print every 60 frames (about every second)
    }
    
    // Render the projectile
    GameObject::render(renderer, camera);
    
    // Render trail if enabled
    if (config.hasTrail && !trailPoints.empty()) {
        renderTrail(renderer, camera);
    }
}

void Projectile::fire(const Vec3& position, const Vec3& direction, GameObject* owner) {
    // Debug output
    std::cout << "=== PROJECTILE FIRE ===" << std::endl;
    std::cout << "Projectile name: " << getName() << std::endl;
    std::cout << "Fire position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Fire direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    
    startPosition = position;
    setPosition(position);
    
    // std::cout << "After setPosition, projectile position: (" << getPosition().x << ", " << getPosition().y << ", " << getPosition().z << ")" << std::endl;
    
    // Normalize direction and set velocity
    Vec3 normalizedDir = Engine::normalize(direction);
    velocity = normalizedDir * config.speed;
    
    std::cout << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
    std::cout << "Normalized direction: (" << normalizedDir.x << ", " << normalizedDir.y << ", " << normalizedDir.z << ")" << std::endl;
    
    // Simple rotation: no rotation for now, just use default orientation
    setRotation(Vec3(0.0f, 0.0f, 0.0f));
    std::cout << "Rotation set to: (0, 0, 0)" << std::endl;
    
    // Set owner
    this->owner = owner;
    if (owner) {
        ownerTag = owner->getName();
    }
    
    // Reset state
    distanceTraveled = 0.0f;
    currentLifetime = 0.0f;
    bounceCount = 0;
    penetrationCount = 0;
    isDestroyed = false;
    
    // Make sure projectile is active
    setActive(true);
    
    // std::cout << "=========================" << std::endl;
    
    // Clear trail
    clearTrail();
    
    // Play fire sound
    if (!config.fireSound.empty()) {
        playSound(config.fireSound);
    }
    
    // Add initial trail point
    if (config.hasTrail) {
        addTrailPoint(position);
    }
    
}

void Projectile::destroy() {
    if (isDestroyed) return;
    
    
    isDestroyed = true;
    
    // Call custom destroy callback
    if (config.onDestroyCallback) {
        config.onDestroyCallback(this);
    }
    
    // Call virtual destroy method
    onDestroy();
    
    // Spawn destruction effects
    spawnImpactEffect(getPosition(), Vec3(0.0f, 1.0f, 0.0f));
}

void Projectile::bounce(const Vec3& normal, float energy) {
    if (bounceCount >= config.maxBounces) {
        destroy();
        return;
    }
    
    // Calculate bounce velocity
    Vec3 normalizedNormal = normal.normalize();
    float dotProduct = velocity.dot(normalizedNormal);
    velocity = velocity - normalizedNormal * (2.0f * dotProduct);
    
    // Apply bounce energy
    velocity = velocity * energy;
    
    bounceCount++;
    
    // Call virtual bounce method
    onBounce(normal);
    
    // Play bounce sound
    if (!config.impactSound.empty()) {
        playSound(config.impactSound);
    }
}

void Projectile::ricochet(const Vec3& normal) {
    // Similar to bounce but with ricochet-specific behavior
    bounce(normal, config.bounceEnergy);
    
    // Call virtual ricochet method
    onRicochet(normal);
}

void Projectile::updatePhysics(float deltaTime) {
    // Apply gravity if enabled
    if (config.affectedByGravity) {
        applyGravity(deltaTime);
    }
    
    // Store old position for distance calculation
    Vec3 oldPosition = getPosition();
    
    // Update position based on velocity
    Vec3 newPosition = oldPosition + velocity * deltaTime;
    setPosition(newPosition);
    
    // Debug output for first few updates (disabled for now)
    // static int physicsDebugCount = 0;
    // physicsDebugCount++;
    // if (physicsDebugCount <= 5) { // Only debug first 5 updates
    //     std::cout << "=== PROJECTILE PHYSICS UPDATE " << physicsDebugCount << " ===" << std::endl;
    //     std::cout << "Projectile: " << getName() << std::endl;
    //     std::cout << "Old position: (" << oldPosition.x << ", " << oldPosition.y << ", " << oldPosition.z << ")" << std::endl;
    //     std::cout << "Velocity: (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
    //     std::cout << "Delta time: " << deltaTime << std::endl;
    //     std::cout << "New position: (" << newPosition.x << ", " << newPosition.y << ", " << newPosition.z << ")" << std::endl;
    //     std::cout << "=====================================" << std::endl;
    // }
    
    // Update distance traveled (calculate movement from old to new position)
    Vec3 movement = newPosition - oldPosition;
    distanceTraveled += movement.length();
    
    // Add trail point
    if (config.hasTrail) {
        addTrailPoint(newPosition);
    }
}

void Projectile::applyGravity(float deltaTime) {
    velocity.y -= config.gravity * deltaTime;
}

void Projectile::checkDistanceLimit() {
    if (distanceTraveled > config.maxDistance) {
        destroy();
    }
}

void Projectile::checkMonsterCollisions() {
    // Get all game objects from the scene
    // This is a simplified collision check - in a real implementation,
    // you'd want a proper collision system
    if (!owner) return;  // Don't check if no owner (prevents self-damage)
    
    // For now, we'll check against all entities in the scene
    // This will be enhanced when we integrate with the monster system
    Vec3 projectilePos = getPosition();
    float collisionRadius = config.size * 0.5f;
    
    // Check if projectile is close to any monster
    // This is a placeholder - actual implementation will use scene's entity list
    if (distanceTraveled > 1.0f) {  // Only check after traveling some distance
        // TODO: Implement proper monster collision detection
        // Removed hardcoded distance limit - let projectiles use their configured lifetime and maxDistance
    }
}

void Projectile::checkLifetime() {
    if (currentLifetime > config.lifetime) {
        destroy();
    }
}

void Projectile::addTrailPoint(const Vec3& position) {
    TrailPoint point;
    point.position = position;
    point.time = currentLifetime;
    trailPoints.push_back(point);
    
    // Clean up old trail points
    cleanupTrail();
}

void Projectile::clearTrail() {
    trailPoints.clear();
}

void Projectile::updateTrail(float deltaTime) {
    // Remove trail points that are too old
    float cutoffTime = currentLifetime - config.trailLength;
    trailPoints.erase(
        std::remove_if(trailPoints.begin(), trailPoints.end(),
            [cutoffTime](const TrailPoint& point) {
                return point.time < cutoffTime;
            }),
        trailPoints.end()
    );
}

void Projectile::cleanupTrail() {
    // Remove trail points beyond the trail length
    float cutoffTime = currentLifetime - config.trailLength;
    trailPoints.erase(
        std::remove_if(trailPoints.begin(), trailPoints.end(),
            [cutoffTime](const TrailPoint& point) {
                return point.time < cutoffTime;
            }),
        trailPoints.end()
    );
}

bool Projectile::checkCollision(GameObject* target) {
    if (!target || target == owner) return false;
    
    // Ignore terrain collisions - we only want to hit monsters
    if (target->getName().find("SimpleChunkTerrain") != std::string::npos ||
        target->getName().find("Chunk_") != std::string::npos ||
        target->getName().find("WaterSurface") != std::string::npos) {
        return false;
    }
    
    // Only check collisions with monsters and other entities
    if (target->getName().find("Monster_") == std::string::npos &&
        target->getName().find("HealthBar") == std::string::npos) {
        return false;
    }
    
    // Simple sphere-sphere collision for now
    // This can be enhanced with more sophisticated collision detection
    Vec3 projectilePos = getPosition();
    Vec3 targetPos = target->getPosition();
    float distance = (projectilePos - targetPos).length();
    
    float projectileRadius = config.size;
    
    // Get proper collision radius from target
    float targetRadius = 1.0f; // Default fallback
    Vec3 targetCenter = targetPos;
    
    // Check if target is a monster and get its proper collision data
    if (target->getName().find("Monster_") != std::string::npos) {
        // Try to cast to Monster to get proper collision data
        try {
            Monster* monster = dynamic_cast<Monster*>(target);
            if (monster) {
                targetRadius = monster->getCollisionRadius();
                targetCenter = monster->getCollisionCenter();
                
                // Recalculate distance with proper collision center
                distance = (projectilePos - targetCenter).length();
            } else {
                // Fallback if cast fails
                targetRadius = 1.5f;
                targetCenter.y += 1.0f;
                distance = (projectilePos - targetCenter).length();
            }
        } catch (...) {
            // Fallback if any error occurs
            targetRadius = 1.5f;
            targetCenter.y += 1.0f;
            distance = (projectilePos - targetCenter).length();
        }
    }
    
    // Debug collision detection
    static int collisionDebugCount = 0;
    collisionDebugCount++;
    if (collisionDebugCount % 300 == 0) { // Print every 5 seconds
    }
    
    return distance < (projectileRadius + targetRadius);
}

void Projectile::handleCollision(GameObject* target) {
    if (!target) return;
    
    
    // Calculate and apply damage
    float damage = calculateDamage(target);
    applyDamage(target, damage);
    
    // Call custom hit callback
    if (config.onHitCallback) {
        config.onHitCallback(this, target);
    }
    
    // Call virtual hit method
    onHit(target);
    
    // Handle penetration
    if (config.penetrateTargets && penetrationCount < config.maxPenetrations) {
        penetrationCount++;
        // Continue through target
        return;
    }
    
    // Only destroy projectile if hitting monsters (not terrain)
    if (target->getName().find("Monster_") != std::string::npos) {
        destroy();
    } else {
    }
    
}

float Projectile::calculateDamage(GameObject* target) {
    // Base damage calculation
    float damage = config.damage;
    
    // Apply armor penetration
    // This is a simplified version - can be enhanced with armor systems
    if (config.armorPenetration > 0.0f) {
        // Reduce damage based on target armor (simplified)
        damage *= (1.0f - config.armorPenetration * 0.5f);
    }
    
    // Apply distance falloff
    float distance = (getPosition() - startPosition).length();
    float falloff = 1.0f - (distance / config.maxDistance);
    damage *= std::max(0.1f, falloff);
    
    return damage;
}

void Projectile::applyDamage(GameObject* target, float damage) {
    // This is a placeholder - damage application should be implemented
    // based on the target's damage system
}

void Projectile::spawnImpactEffect(const Vec3& position, const Vec3& normal) {
    // Placeholder for impact effects
    // This should spawn particles, play sounds, etc.
}

void Projectile::spawnTrailEffect() {
    // Placeholder for trail effects
    // This should spawn particles along the trail
}

void Projectile::playSound(const std::string& soundName) {
    // Placeholder for sound system
    // This should play the specified sound
}

Vec3 Projectile::predictPosition(float timeAhead) const {
    Vec3 predictedPos = getPosition() + velocity * timeAhead;
    
    // Apply gravity if enabled
    if (config.affectedByGravity) {
        predictedPos.y -= 0.5f * config.gravity * timeAhead * timeAhead;
    }
    
    return predictedPos;
}

bool Projectile::isInRange(const Vec3& targetPosition) const {
    float distance = (targetPosition - startPosition).length();
    return distance <= config.maxDistance;
}

float Projectile::getTimeToTarget(const Vec3& targetPosition) const {
    Vec3 direction = (targetPosition - getPosition()).normalize();
    float distance = (targetPosition - getPosition()).length();
    return distance / config.speed;
}

void Projectile::performExplosion(const Vec3& position) {
    if (!config.explosive) return;
    
    // Placeholder for explosion effects
    // This should create explosion particles, apply force to nearby objects, etc.
}

// Virtual method implementations
void Projectile::onHit(GameObject* target) {
    // Override in derived classes for custom hit behavior
}

void Projectile::onDestroy() {
    // Override in derived classes for custom destroy behavior
}

void Projectile::onUpdate(float deltaTime) {
    // Override in derived classes for custom update behavior
}

void Projectile::onBounce(const Vec3& normal) {
    // Override in derived classes for custom bounce behavior
}

void Projectile::onRicochet(const Vec3& normal) {
    // Override in derived classes for custom ricochet behavior
}

void Projectile::renderTrail(const Renderer& renderer, const Camera& camera) {
    // Placeholder for trail rendering
    // This should render a trail effect using the trail points
}

bool Projectile::shouldDestroy() const {
    return isDestroyed || currentLifetime > config.lifetime || distanceTraveled > config.maxDistance;
}

// ProjectileFactory implementation
std::unique_ptr<Projectile> ProjectileFactory::createProjectile(ProjectileType type, const std::string& name) {
    ProjectileConfig config = getDefaultConfig(type);
    return std::make_unique<Projectile>(name.empty() ? "Projectile" : name, config);
}

ProjectileConfig ProjectileFactory::getDefaultConfig(ProjectileType type) {
    switch (type) {
        case ProjectileType::Bullet:
            return createBulletConfig();
        case ProjectileType::Rocket:
            return createRocketConfig();
        case ProjectileType::Laser:
            return createLaserConfig();
        case ProjectileType::Grenade:
            return createGrenadeConfig();
        case ProjectileType::Plasma:
            return createPlasmaConfig();
        case ProjectileType::Arrow:
            return createArrowConfig();
        default:
            return ProjectileConfig{};
    }
}

ProjectileConfig ProjectileFactory::createBulletConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Bullet;
    config.speed = 100.0f;
    config.maxDistance = 200.0f;
    config.lifetime = 3.0f;
    config.size = 0.05f;
    config.damage = 25.0f;
    config.color = Vec3(1.0f, 1.0f, 0.0f);
    config.hasTrail = true;
    config.trailLength = 1.0f;
    config.fireSound = "bullet_fire.wav";
    config.impactSound = "bullet_impact.wav";
    return config;
}

ProjectileConfig ProjectileFactory::createRocketConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Rocket;
    config.speed = 30.0f;
    config.maxDistance = 150.0f;
    config.lifetime = 8.0f;
    config.size = 0.2f;
    config.affectedByGravity = true;
    config.damage = 100.0f;
    config.explosive = true;
    config.explosionRadius = 5.0f;
    config.explosionForce = 50.0f;
    config.color = Vec3(1.0f, 0.5f, 0.0f);
    config.hasTrail = true;
    config.trailLength = 3.0f;
    config.fireSound = "rocket_fire.wav";
    config.impactSound = "rocket_explosion.wav";
    return config;
}

ProjectileConfig ProjectileFactory::createLaserConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Laser;
    config.speed = 1000.0f; // Very fast
    config.maxDistance = 300.0f;
    config.lifetime = 1.0f;
    config.size = 0.02f;
    config.damage = 50.0f;
    config.damageType = DamageType::Energy;
    config.color = Vec3(0.0f, 1.0f, 1.0f);
    config.hasGlow = true;
    config.glowIntensity = 2.0f;
    config.fireSound = "laser_fire.wav";
    return config;
}

ProjectileConfig ProjectileFactory::createGrenadeConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Grenade;
    config.speed = 20.0f;
    config.maxDistance = 100.0f;
    config.lifetime = 10.0f;
    config.size = 0.15f;
    config.affectedByGravity = true;
    config.bounces = true;
    config.maxBounces = 3;
    config.bounceEnergy = 0.7f;
    config.damage = 75.0f;
    config.explosive = true;
    config.explosionRadius = 8.0f;
    config.explosionForce = 100.0f;
    config.color = Vec3(0.5f, 0.5f, 0.5f);
    config.fireSound = "grenade_throw.wav";
    config.impactSound = "grenade_bounce.wav";
    return config;
}

ProjectileConfig ProjectileFactory::createPlasmaConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Plasma;
    config.speed = 40.0f;
    config.maxDistance = 120.0f;
    config.lifetime = 6.0f;
    config.size = 0.1f;
    config.damage = 35.0f;
    config.damageType = DamageType::Energy;
    config.color = Vec3(0.0f, 1.0f, 0.5f);
    config.hasGlow = true;
    config.glowIntensity = 1.5f;
    config.hasTrail = true;
    config.trailLength = 2.0f;
    config.fireSound = "plasma_fire.wav";
    config.impactSound = "plasma_impact.wav";
    return config;
}

ProjectileConfig ProjectileFactory::createArrowConfig() {
    ProjectileConfig config;
    config.type = ProjectileType::Arrow;
    config.speed = 60.0f;
    config.maxDistance = 80.0f;
    config.lifetime = 4.0f;
    config.size = 0.03f;
    config.affectedByGravity = true;
    config.damage = 40.0f;
    config.penetrateTargets = true;
    config.maxPenetrations = 1;
    config.color = Vec3(0.8f, 0.6f, 0.4f);
    config.fireSound = "arrow_fire.wav";
    config.impactSound = "arrow_impact.wav";
    return config;
}

// ProjectileManager implementation
ProjectileManager::ProjectileManager()
    : collisionSystem(nullptr), particleSystem(nullptr), audioSystem(nullptr) {
}

ProjectileManager::~ProjectileManager() {
    cleanup();
}

void ProjectileManager::initialize(CollisionSystem* collision, ParticleSystem* particles, AudioSystem* audio) {
    collisionSystem = collision;
    particleSystem = particles;
    audioSystem = audio;
}

// MonsterProjectile - Specialized projectile for monster damage
class MonsterProjectile : public Projectile {
public:
    MonsterProjectile(const std::string& name, const ProjectileConfig& config) 
        : Projectile(name, config) {}
    
    virtual void onHit(GameObject* target) override {
        
        // Check if target is a monster
        Monster* monster = dynamic_cast<Monster*>(target);
        if (monster && monster->isAlive()) {
            // Apply damage to monster
            float damage = calculateDamage(target);
            monster->takeDamage(damage, getOwner());
            
        } else {
        }
    }
};

void ProjectileManager::update(float deltaTime) {
    // Update all active projectiles
    for (auto it = activeProjectiles.begin(); it != activeProjectiles.end();) {
        Projectile* projectile = it->get();
        
        if (projectile->isActive()) {
            projectile->update(deltaTime);
            ++it;
        } else {
            // Remove destroyed projectiles
            it = activeProjectiles.erase(it);
        }
    }
}

void ProjectileManager::render(const Renderer& renderer, const Camera& camera) {
    static int renderCount = 0;
    renderCount++;
    
    if (renderCount % 180 == 0) { // Print every 3 seconds
        
        // List all active projectiles
        for (size_t i = 0; i < activeProjectiles.size(); ++i) {
            auto& projectile = activeProjectiles[i];
            if (projectile->isActive()) {
                std::cout << "  Projectile " << i << ": " << projectile->getName() 
                          << " at (" << projectile->getPosition().x << ", " 
                          << projectile->getPosition().y << ", " 
                          << projectile->getPosition().z << ")" << std::endl;
            }
        }
    }
    
    // Render all active projectiles
    for (auto& projectile : activeProjectiles) {
        if (projectile->isActive()) {
            projectile->render(renderer, camera);
        }
    }
}

void ProjectileManager::cleanup() {
    activeProjectiles.clear();
}

Projectile* ProjectileManager::createProjectile(const ProjectileConfig& config, const std::string& name) {
    // std::cout << "=== PROJECTILE MANAGER: CREATING PROJECTILE ===" << std::endl;
    
    // Use MonsterProjectile for better monster damage handling
    auto projectile = std::make_unique<MonsterProjectile>(name.empty() ? "Projectile" : name, config);
    Projectile* ptr = projectile.get();
    
    // std::cout << "Projectile created, checking initial state:" << std::endl;
    // std::cout << "  Name: " << ptr->getName() << std::endl;
    // std::cout << "  Active: " << (ptr->isActive() ? "true" : "false") << std::endl;
    // std::cout << "  Position: (" << ptr->getPosition().x << ", " << ptr->getPosition().y << ", " << ptr->getPosition().z << ")" << std::endl;
    
    // Initialize the projectile
    if (!ptr->initialize()) {
        // std::cout << "Failed to initialize projectile!" << std::endl;
        return nullptr;
    }
    
    // std::cout << "After initialization:" << std::endl;
    // std::cout << "  Active: " << (ptr->isActive() ? "true" : "false") << std::endl;
    // std::cout << "  Position: (" << ptr->getPosition().x << ", " << ptr->getPosition().y << ", " << ptr->getPosition().z << ")" << std::endl;
    
    activeProjectiles.push_back(std::move(projectile));
    
    // std::cout << "Projectile added to active list. Total active: " << activeProjectiles.size() << std::endl;
    // std::cout << "Projectile pointer: " << ptr << std::endl;
    // std::cout << "Projectile active: " << (ptr->isActive() ? "true" : "false") << std::endl;
    // std::cout << "=============================================" << std::endl;
    
    return ptr;
}

void ProjectileManager::destroyProjectile(Projectile* projectile) {
    if (projectile) {
        projectile->destroy();
    }
}

void ProjectileManager::destroyAllProjectiles() {
    for (auto& projectile : activeProjectiles) {
        projectile->destroy();
    }
    activeProjectiles.clear();
}

// Create a projectile specifically designed for monster hunting
Projectile* ProjectileManager::createMonsterHunterProjectile(const std::string& name) {
    // std::cout << "=== CREATING MONSTER HUNTER PROJECTILE ===" << std::endl;
    
    ProjectileConfig config;
    config.type = ProjectileType::Bullet;
    config.speed = 80.0f;  // Slower for better visibility
    config.maxDistance = 150.0f;
    config.lifetime = 3.0f;  // Longer lifetime to see them
    config.size = 1.0f;  // Larger size for better visibility
    config.damage = 35.0f;  // Good damage against monsters
    config.color = Vec3(1.0f, 0.0f, 0.0f);  // Bright red for maximum visibility
    config.hasTrail = true;
    config.trailLength = 2.0f;
    config.destroyOnCollision = false;  // Don't destroy on terrain collision
    config.penetrateTargets = true;  // Allow penetrating terrain to reach monsters
    config.maxPenetrations = 5;  // Allow multiple terrain penetrations
    
    // std::cout << "Projectile config created:" << std::endl;
    // std::cout << "  Size: " << config.size << std::endl;
    // std::cout << "  Color: (" << config.color.x << ", " << config.color.y << ", " << config.color.z << ")" << std::endl;
    // std::cout << "  Speed: " << config.speed << std::endl;
    // std::cout << "  Lifetime: " << config.lifetime << std::endl;
    
    Projectile* projectile = createProjectile(config, name);
    
    if (projectile) {
        // std::cout << "Monster Hunter Projectile created successfully: " << projectile->getName() << std::endl;
    } else {
        // std::cout << "Failed to create Monster Hunter Projectile!" << std::endl;
    }
    
    return projectile;
}

void ProjectileManager::checkAllCollisions(const std::vector<GameObject*>& gameObjects) {
    static int collisionCheckCount = 0;
    collisionCheckCount++;
    
    if (collisionCheckCount % 180 == 0) { // Print every 3 seconds
        // std::cout << "=== COLLISION CHECK DEBUG ===" << std::endl;
        // std::cout << "Active projectiles: " << activeProjectiles.size() << std::endl;
        // std::cout << "Game objects to check: " << gameObjects.size() << std::endl;
    }
    
    for (auto& projectile : activeProjectiles) {
        if (!projectile->isActive()) continue;
        
        for (GameObject* gameObject : gameObjects) {
            if (projectile->checkCollision(gameObject)) {
                // std::cout << "=== COLLISION DETECTED ===" << std::endl;
                // std::cout << "Projectile: " << projectile->getName() << std::endl;
                // std::cout << "Target: " << gameObject->getName() << std::endl;
                // std::cout << "Target type: " << typeid(*gameObject).name() << std::endl;
                
                projectile->handleCollision(gameObject);
                break; // Handle one collision at a time
            }
        }
    }
}

} // namespace Engine
