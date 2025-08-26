/**
 * Projectile.cpp - Implementation of Modular Projectile System
 */

#include "Projectile.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Mesh.h"
#include "../Rendering/Shader.h"
#include "../Math/Math.h"
#include <iostream>
#include <algorithm>
#include <cmath>

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
    
    // Set projectile to not be a world entity (it's a temporary object)
    setEntity(false);
    
    // Set initial scale based on projectile size
    setScale(Vec3(config.size, config.size, config.size));
    
    // Set color
    setColor(config.color);
}

bool Projectile::initialize() {
    if (!GameObject::initialize()) {
        return false;
    }
    
    // Create projectile mesh (simple sphere for now)
    setupProjectileMesh();
    
    return true;
}

void Projectile::setupProjectileMesh() {
    // Create a simple sphere mesh for the projectile
    // This can be enhanced with different meshes for different projectile types
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Generate sphere vertices (simplified)
    const int segments = 8;
    const int rings = 6;
    const float radius = 0.5f;
    
    // Generate vertices
    for (int ring = 0; ring <= rings; ++ring) {
        float phi = (float)ring * 3.14159f / rings;
        float y = radius * cos(phi);
        float ringRadius = radius * sin(phi);
        
        for (int segment = 0; segment <= segments; ++segment) {
            float theta = (float)segment * 2.0f * 3.14159f / segments;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }
    
    // Generate indices
    for (int ring = 0; ring < rings; ++ring) {
        for (int segment = 0; segment < segments; ++segment) {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(next);
            indices.push_back(next + 1);
            indices.push_back(current + 1);
        }
    }
    
    mesh = std::make_unique<Mesh>();
    if (!mesh->createMesh(vertices, indices)) {
        std::cerr << "Failed to create projectile mesh" << std::endl;
    }
}

void Projectile::update(float deltaTime) {
    if (isDestroyed) return;
    
    // Update lifetime
    currentLifetime += deltaTime;
    
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
    
    // Render the projectile
    GameObject::render(renderer, camera);
    
    // Render trail if enabled
    if (config.hasTrail && !trailPoints.empty()) {
        renderTrail(renderer, camera);
    }
}

void Projectile::fire(const Vec3& position, const Vec3& direction, GameObject* owner) {
    startPosition = position;
    setPosition(position);
    
    // Normalize direction and set velocity
    Vec3 normalizedDir = Engine::normalize(direction);
    velocity = normalizedDir * config.speed;
    
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
    
    // Update position based on velocity
    Vec3 newPosition = getPosition() + velocity * deltaTime;
    setPosition(newPosition);
    
    // Update distance traveled
    Vec3 movement = newPosition - getPosition();
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
        // For now, just destroy projectile after some distance
        if (distanceTraveled > 50.0f) {
            destroy();
        }
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
    
    // Simple sphere-sphere collision for now
    // This can be enhanced with more sophisticated collision detection
    Vec3 projectilePos = getPosition();
    Vec3 targetPos = target->getPosition();
    float distance = (projectilePos - targetPos).length();
    
    float projectileRadius = config.size;
    float targetRadius = 1.0f; // Default target radius, should be configurable
    
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
    
    // Destroy projectile if configured to do so
    if (config.destroyOnCollision) {
        destroy();
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
    std::cout << "Projectile " << getName() << " dealt " << damage << " damage to " << target->getName() << std::endl;
}

void Projectile::spawnImpactEffect(const Vec3& position, const Vec3& normal) {
    // Placeholder for impact effects
    // This should spawn particles, play sounds, etc.
    std::cout << "Impact effect at position (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void Projectile::spawnTrailEffect() {
    // Placeholder for trail effects
    // This should spawn particles along the trail
}

void Projectile::playSound(const std::string& soundName) {
    // Placeholder for sound system
    // This should play the specified sound
    std::cout << "Playing sound: " << soundName << std::endl;
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
    std::cout << "Explosion at position (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Explosion radius: " << config.explosionRadius << ", Force: " << config.explosionForce << std::endl;
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
    auto projectile = std::make_unique<Projectile>(name.empty() ? "Projectile" : name, config);
    Projectile* ptr = projectile.get();
    activeProjectiles.push_back(std::move(projectile));
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

void ProjectileManager::checkAllCollisions(const std::vector<GameObject*>& gameObjects) {
    for (auto& projectile : activeProjectiles) {
        if (!projectile->isActive()) continue;
        
        for (GameObject* gameObject : gameObjects) {
            if (projectile->checkCollision(gameObject)) {
                projectile->handleCollision(gameObject);
                break; // Handle one collision at a time
            }
        }
    }
}

} // namespace Engine
