/**
 * PhysicsSystem.h - Physics and Collision Detection System
 * 
 * Handles gravity, collision detection, and terrain interaction for game objects.
 * Provides a unified physics system for the WW3 engine.
 */

#pragma once
#include "../Math/Math.h"
#include "../Utils/TerrainGenerator.h"
#include <vector>
#include <memory>

namespace Engine {

// Physics body types
enum class BodyType {
    Static,     // Immovable objects (terrain, buildings)
    Dynamic,    // Movable objects (player, monsters, projectiles)
    Kinematic   // Script-controlled objects
};

// Collision shape types
enum class CollisionShape {
    Box,
    Sphere,
    Capsule
};

// Physics material properties
struct PhysicsMaterial {
    float friction = 0.5f;
    float restitution = 0.2f;  // Bounciness
    float density = 1.0f;
};

// Collision detection result
struct CollisionResult {
    bool collided = false;
    Vec3 normal = Vec3(0, 1, 0);
    float penetration = 0.0f;
    Vec3 contactPoint = Vec3(0, 0, 0);
};

// Physics body for game objects
class PhysicsBody {
private:
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 size;  // For box collision
    float radius;  // For sphere/capsule collision
    
    BodyType bodyType;
    CollisionShape collisionShape;
    PhysicsMaterial material;
    
    bool onGround = false;
    float groundHeight = 0.0f;
    
    // Terrain interaction
    bool affectedByGravity = true;
    bool canCollideWithTerrain = true;

public:
    PhysicsBody(const Vec3& pos, const Vec3& size, BodyType type = BodyType::Dynamic);
    PhysicsBody(const Vec3& pos, float radius, BodyType type = BodyType::Dynamic);
    
    // Getters
    Vec3 getPosition() const { return position; }
    Vec3 getVelocity() const { return velocity; }
    Vec3 getSize() const { return size; }
    float getRadius() const { return radius; }
    BodyType getBodyType() const { return bodyType; }
    CollisionShape getCollisionShape() const { return collisionShape; }
    bool isOnGround() const { return onGround; }
    float getGroundHeight() const { return groundHeight; }
    
    // Setters
    void setPosition(const Vec3& pos) { position = pos; }
    void setVelocity(const Vec3& vel) { velocity = vel; }
    void setAcceleration(const Vec3& acc) { acceleration = acc; }
    void setAffectedByGravity(bool affected) { affectedByGravity = affected; }
    void setCanCollideWithTerrain(bool canCollide) { canCollideWithTerrain = canCollide; }
    
    // Physics update
    void update(float deltaTime, const TerrainGenerator& terrainGenerator);
    
    // Collision detection
    CollisionResult checkCollision(const PhysicsBody& other) const;
    bool checkTerrainCollision(const TerrainGenerator& terrainGenerator);
    
    // Terrain interaction
    void snapToGround(const TerrainGenerator& terrainGenerator);
    float getTerrainHeightAt(const TerrainGenerator& terrainGenerator, const Vec3& pos) const;
};

/**
 * PhysicsSystem - Main physics management system
 * 
 * Manages all physics bodies, handles gravity, collision detection,
 * and terrain interaction for the entire game world.
 */
class PhysicsSystem {
private:
    std::vector<std::unique_ptr<PhysicsBody>> bodies;
    Vec3 gravity = Vec3(0, -9.81f, 0);  // Earth gravity
    TerrainGenerator* terrainGenerator = nullptr;
    
    // Physics settings
    float fixedTimeStep = 1.0f / 60.0f;  // 60 FPS physics
    int maxSubSteps = 5;  // Maximum substeps for stability
    
public:
    PhysicsSystem();
    ~PhysicsSystem();
    
    // Body management
    PhysicsBody* createBody(const Vec3& position, const Vec3& size, BodyType type = BodyType::Dynamic);
    PhysicsBody* createBody(const Vec3& position, float radius, BodyType type = BodyType::Dynamic);
    void removeBody(PhysicsBody* body);
    
    // Terrain integration
    void setTerrainGenerator(TerrainGenerator* generator) { terrainGenerator = generator; }
    TerrainGenerator* getTerrainGenerator() const { return terrainGenerator; }
    
    // Physics update
    void update(float deltaTime);
    
    // Collision detection
    void checkCollisions();
    
    // Gravity and terrain
    void applyGravity();
    void handleTerrainCollisions();
    
    // Utility
    void setGravity(const Vec3& grav) { gravity = grav; }
    Vec3 getGravity() const { return gravity; }
    
    // Debug
    void debugDraw() const;
};

} // namespace Engine
