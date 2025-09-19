/**
 * Monster.cpp - Implementation of Simplified Monster/Enemy System
 */

#include "Monster.h"
#include "Player.h"
#include "../Engine/Core/Projectile.h"
#include "../Engine/Core/Scene.h"
#include "../Engine/Utils/OBJLoader.h"
#include "../Engine/Rendering/MaterialLoader.h"
#include "../Engine/Rendering/Renderer.h"
#include "../Engine/Rendering/MonsterRenderer.h"
#include "../Engine/Rendering/RendererFactory.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <ctime>
#include <random>
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Engine {

Monster::Monster(const std::string& name, MonsterType monsterType)
    : GameObject(name),
      type(monsterType),
      state(MonsterState::Idle),
      health(100.0f),
      maxHealth(100.0f),
      moveSpeed(2.0f),      // Slower default speed for better gameplay
      attackRange(2.0f),
      attackDamage(25.0f),
      detectionRange(5.0f),   // Reduced default detection range
      dangerRange(8.0f),      // Reduced default danger range
      targetPosition(0.0f, 0.0f, 0.0f),
      moveTimer(0.0f),
      stateTimer(0.0f),
      patrolRadius(15.0f),  // Reasonable patrol radius for visible movement
      chargeSpeed(4.0f),    // Slower speed when charging
      baseSpeed(2.0f),      // Slower normal patrol speed
      isCharging(false),    // Start in non-charging mode
      lastAttackTime(0.0f),
      attackCooldown(2.0f),
      alertTimer(0.0f),
      stunTimer(0.0f),
      retreatTimer(0.0f),
      lastPlayerSeenTime(0.0f),
      lastKnownPlayerPos(0.0f, 0.0f, 0.0f),
      hasLineOfSight(false),
      aggressionLevel(0.5f),  // Start with moderate aggression
      fearLevel(0.0f),        // Start with no fear
      currentPathDirection(0.0f, 0.0f, 0.0f),
      pathUpdateTimer(0.0f),
      pathUpdateInterval(0.5f), // Update pathfinding every 0.5 seconds
      isStuck(false),
      stuckTimer(0.0f),
      lastPosition(0.0f, 0.0f, 0.0f),
      inGroup(false),
      groupAlertRadius(10.0f),  // Alert monsters within 10 units
      groupCoordinationTimer(0.0f),
      hasAlertedGroup(false),
      groupTarget(0.0f, 0.0f, 0.0f),
      damageFlashTimer(0.0f),
      originalColor(0.5f, 0.14f, 0.58f),  // Xenomorph purple color
      damageColor(1.0f, 0.0f, 0.0f),      // Red for damage
      isFlashing(false),
      stateChangeFlashTimer(0.0f),
      isStateFlashing(false),
      alertColor(1.0f, 1.0f, 0.0f),       // Yellow for alert
      chaseColor(1.0f, 0.5f, 0.0f),       // Orange for chasing
      attackColor(1.0f, 0.0f, 0.0f),      // Red for attacking
      pulseTimer(0.0f),
      isPulsing(false),
      pulseSpeed(2.0f),
      experienceReward(10),
      scoreReward(100),
      hasDroppedLoot(false),
      deathAnimationTimer(0.0f),
      deathAnimationDuration(2.0f), // 2 seconds death animation
      isDeathAnimating(false),
      deathScale(1.0f, 1.0f, 1.0f),
      originalScale(1.0f, 1.0f, 1.0f),
      markedForDeletion(false),
      deletionTimer(0.0f),
      playerTarget(nullptr),
      textureHealthBar(nullptr),  // NEW: Using texture-based system
      showHealthBar(true) { // Higher above monster
    
    // Set entity flag to true for monsters
    setEntity(true);
    
    // Configure monster based on type
    configureMonster(monsterType);
}

bool Monster::initialize() {
    if (isInitialized) return true;
    
    // std::cout << "=== INITIALIZING MONSTER: " << getName() << " ===" << std::endl;
    
    // Setup monster mesh
    // std::cout << "Setting up monster mesh..." << std::endl;
    setupMonsterMesh();
    
    // Setup monster material
    // std::cout << "Setting up monster material..." << std::endl;
    setupMonsterMaterial();
    
    // Set initial target position
    targetPosition = getPosition();
    
    // Create health bar - NEW: Using texture-based system
    if (showHealthBar) {
        textureHealthBar = std::make_unique<TextureHealthBar>(2.5f, 0.5f, 2.5f); // Position 2.5 units above monster
        textureHealthBar->setHealth(health, maxHealth);
        textureHealthBar->initialize();
    }
    
    isInitialized = true;
    // std::cout << "Monster initialized successfully: " << getName() << std::endl;
    // std::cout << "=== END INITIALIZATION ===" << std::endl;
    return true;
}

void Monster::update(float deltaTime) {
    if (!isActive || !isInitialized) return;
    
    // CRASH PREVENTION: Skip all updates for dead monsters
    // Don't touch them at all to prevent memory corruption
    if (isDead()) return;
    
    // Debug output every few seconds to track position changes
    static float updateDebugTimer = 0.0f;
    static Vec3 lastLoggedPosition = Vec3(0.0f, 0.0f, 0.0f);
    updateDebugTimer += deltaTime;
    
    Vec3 currentPos = getPosition();
    float positionChange = sqrt(
        (currentPos.x - lastLoggedPosition.x) * (currentPos.x - lastLoggedPosition.x) +
        (currentPos.y - lastLoggedPosition.y) * (currentPos.y - lastLoggedPosition.y) +
        (currentPos.z - lastLoggedPosition.z) * (currentPos.z - lastLoggedPosition.z)
    );
    
    if (updateDebugTimer > 3.0f || positionChange > 0.1f) {
        std::cout << "=== MONSTER POSITION DEBUG ===" << std::endl;
        std::cout << "Monster: " << getName() << std::endl;
        std::cout << "State: " << getStateName(state) << std::endl;
        std::cout << "Position: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
        std::cout << "Position change: " << positionChange << std::endl;
        std::cout << "Target: (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
        std::cout << "Active: " << (getActive() ? "YES" : "NO") << std::endl;
        std::cout << "===============================" << std::endl;
        updateDebugTimer = 0.0f;
        lastLoggedPosition = currentPos;
    }
    
    // Update AI behavior
    // updateAI(deltaTime);  // DISABLED: Focus on health bar positioning
    
    // Update movement
    // updateMovement(deltaTime);  // DISABLED: Focus on health bar positioning
    
    // Update visual effects
    updateVisualEffects(deltaTime);
    
    // Update health bar - NEW: Using texture-based system
    updateHealthBar();
    
    // Call base class update
    GameObject::update(deltaTime);
}

void Monster::markForDeletion() {
    markedForDeletion = true;
    deletionTimer = 0.0f;
    // std::cout << "Monster " << getName() << " marked for deletion" << std::endl;
}



void Monster::render(const Renderer& renderer, const Camera& camera) {
    if (!isActive || !isInitialized) return;
    
    // Skip rendering if monster is dead
    if (isDead()) return;
    
    // Try to use MonsterRenderer for multi-material rendering
    const MonsterRenderer* monsterRenderer = dynamic_cast<const MonsterRenderer*>(&renderer);
    if (monsterRenderer && !materialGroups.empty()) {
        // Use monster renderer for multi-material rendering
        Mat4 monsterMatrix = getModelMatrix();
        
        // std::cout << "Rendering monster " << getName() << " with " << materialGroups.size() << " material groups" << std::endl;
        
        // Render each material group with its own color
        for (const auto& materialGroup : materialGroups) {
            Vec3 renderColor = materialGroup.color;
            
            // Apply damage flash effect
            if (isFlashing) {
                renderColor = damageColor;
                // std::cout << "  Applying damage flash color: " << renderColor.x << ", " << renderColor.y << ", " << renderColor.z << std::endl;
            } else {
                // std::cout << "  Using material color: " << renderColor.x << ", " << renderColor.y << ", " << renderColor.z << std::endl;
            }
            
            monsterRenderer->renderMonsterTriangles(*mesh, monsterMatrix, camera, renderColor, materialGroup.indices, true);
        }
    } else {
        // Fallback to basic renderer - ALWAYS set a color
        Vec3 finalColor;
        
        if (!materialGroups.empty()) {
            // Use the first material group's color as the dominant color
            finalColor = materialGroups[0].color;
            // std::cout << "Monster " << getName() << " using first material color: " << finalColor.x << ", " << finalColor.y << ", " << finalColor.z << std::endl;
        } else {
            // Fallback to original color system
            finalColor = getCurrentColor();
            // std::cout << "Monster " << getName() << " using fallback original color: " << finalColor.x << ", " << finalColor.y << ", " << finalColor.z << std::endl;
        }
            
            // Apply damage flash effect
            if (isFlashing) {
            finalColor = damageColor;
            // std::cout << "Monster " << getName() << " applying damage flash: " << finalColor.x << ", " << finalColor.y << ", " << finalColor.z << std::endl;
        }
        
        // CRITICAL: Always set the color before rendering
        setColor(finalColor);
        
        // Render the monster using the basic renderer
        GameObject::render(renderer, camera);
    }
    
    // Health bar is now rendered separately in Game::render after monster rendering
}

void Monster::cleanup() {
    if (!isInitialized) return;
    
    // std::cout << "Cleaning up Monster: " << getName() << std::endl;
    
    playerTarget = nullptr;
    
    GameObject::cleanup();
}

void Monster::spawn(const Vec3& position) {
    setPosition(position);
    setHealth(maxHealth);
    setState(MonsterState::Idle);  // FIXED: Set to Idle to prevent movement
    resetTimers();
    setActive(true);
    
    // IMPORTANT: Set initial patrol target to a different position to trigger movement
    targetPosition = getRandomPatrolPosition();
    
    // Set initial rotation to face the first patrol target
    Vec3 direction = targetPosition - position;
    if (direction.x != 0.0f || direction.z != 0.0f) {
        setRotationFromDirection(direction);
    }
    
    // std::cout << "=== MONSTER SPAWN ===" << std::endl;
    // std::cout << "Monster " << getName() << " spawned at: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    // std::cout << "Initial patrol target: (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
    // std::cout << "Initial rotation: (" << getRotation().x << ", " << getRotation().y << ", " << getRotation().z << ") degrees" << std::endl;
    // std::cout << "Monster type: " << static_cast<int>(type) << std::endl;
    // std::cout << "Initial state: Patrolling" << std::endl;
    // std::cout << "Health: " << health << "/" << maxHealth << std::endl;
    // std::cout << "Move speed: " << moveSpeed << std::endl;
    // std::cout << "Active: " << (getActive() ? "YES" : "NO") << std::endl;
    // std::cout << "Entity flag: " << (getEntity() ? "YES" : "NO") << std::endl;
    // std::cout << "=================" << std::endl;
}

void Monster::takeDamage(float damage, GameObject* attacker) {
    // CRASH PREVENTION: Multiple safety checks
    if (isDead()) {
        // std::cout << "Monster " << getName() << " is already dead, ignoring damage" << std::endl;
        return;
    }
    
    if (!getActive()) {
        // std::cout << "Monster " << getName() << " is inactive, ignoring damage" << std::endl;
        return;
    }
    
    if (health <= 0.0f) {
        // std::cout << "Monster " << getName() << " has no health, ignoring damage" << std::endl;
        return;
    }
    
    health = std::max(0.0f, health - damage);
    flashDamage();
    
    // Increase aggression when taking damage
    setAggressionLevel(aggressionLevel + 0.2f);
    
    // Alert nearby monsters when taking damage
    if (!hasAlertedGroup) {
        alertNearbyMonsters();
        hasAlertedGroup = true;
    }
    
    // Chance to become stunned based on damage amount
    if (damage > 15.0f && (rand() % 100) < 30) { // 30% chance to stun on heavy damage
        becomeStunned(1.0f + (damage / 50.0f)); // Stun duration based on damage
    }
    
    // Health bar is now rendered inline - no separate object to update
    
    // std::cout << "Monster " << getName() << " took " << damage << " damage. Health: " << health << "/" << maxHealth << std::endl;
    
    // If health reaches 0, die
    if (health <= 0.0f) {
        // std::cout << "Monster " << getName() << " health reached 0, calling die()..." << std::endl;
        die();
    }
    
    // Call damage callback
    onDamage(damage, attacker);
}

void Monster::die() {
    if (isDead()) return;
    
    // std::cout << "=== MONSTER DEATH PROCESS STARTING ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    
    // CRASH PREVENTION: Set state first to prevent further updates
    setState(MonsterState::Dead);
    // std::cout << "State set to Dead" << std::endl;
    
    // Start death animation
    startDeathAnimation();
    // std::cout << "Death animation started" << std::endl;
    
    // CRASH PREVENTION: Mark monster as inactive immediately
    setActive(false);
    // std::cout << "Monster marked as inactive" << std::endl;
    
    // CRASH PREVENTION: Set health to 0 to prevent further damage
    health = 0.0f;
    // std::cout << "Health set to 0" << std::endl;
    
    // Health bar is now rendered inline - no separate object to clean up
    // std::cout << "Monster " << getName() << " died! No health bar cleanup needed (inline rendering)" << std::endl;
    
    // std::cout << "Monster " << getName() << " died! (Scene will ignore it)" << std::endl;
    
    // Drop loot if not already dropped
    if (!hasDroppedLoot) {
        dropLoot();
        hasDroppedLoot = true;
    }
    
    // Call death callback with error handling
    // std::cout << "Calling onDeath callback..." << std::endl;
    try {
    onDeath();
        // std::cout << "onDeath callback completed" << std::endl;
    } catch (const std::exception&) {
        // std::cout << "Error in onDeath callback: " << e.what() << std::endl;
    }
    
    // std::cout << "=== MONSTER DEATH PROCESS COMPLETED ===" << std::endl;
}

void Monster::updateAI(float deltaTime) {
    if (isDead()) return;
    
    // Update all timers
    updateTimers(deltaTime);
    
    // Update behavior modifiers (aggression, fear)
    updateBehaviorModifiers(deltaTime);
    
    // Update line of sight to player
    updateLineOfSight();
    
    // Update pathfinding
    updatePathfinding(deltaTime);
    
    // Update group behavior
    updateGroupBehavior(deltaTime);
    
    // Update state based on conditions
    updateState(deltaTime);
}

void Monster::updateMovement(float deltaTime) {
    if (isDead()) return;
    
    // Debug movement state
    static float movementDebugTimer = 0.0f;
    movementDebugTimer += deltaTime;
    if (movementDebugTimer > 2.0f) {
        // std::cout << "=== MOVEMENT UPDATE ===" << std::endl;
        // std::cout << "Monster: " << getName() << std::endl;
        // std::cout << "State: " << getStateName(state) << std::endl;
        // std::cout << "Delta time: " << deltaTime << std::endl;
        movementDebugTimer = 0.0f;
    }
    
    switch (state) {
        case MonsterState::Idle:
            // Do nothing, just stand still
            if (movementDebugTimer < 0.1f) { // Only print once per debug cycle
                // std::cout << "Monster " << getName() << " is IDLE - not moving" << std::endl;
            }
            break;
            
        case MonsterState::Patrolling:
            patrol(deltaTime);
            break;
            
        case MonsterState::Alert:
            // Look around and prepare for action
            if (movementDebugTimer < 0.1f) {
                // std::cout << "Monster " << getName() << " is ALERT - scanning area" << std::endl;
            }
            break;
            
        case MonsterState::Chasing:
            moveTowardsTarget(deltaTime);
            break;
            
        case MonsterState::Attacking:
            // Stop moving when attacking and perform attack
            attack();
            break;
            
        case MonsterState::Stunned:
            // No movement when stunned
            if (movementDebugTimer < 0.1f) {
                // std::cout << "Monster " << getName() << " is STUNNED - cannot move" << std::endl;
            }
            break;
            
        case MonsterState::Retreating:
            retreat(deltaTime);
            break;
            
        case MonsterState::Dead:
            // No movement when dead
            break;
    }
    
    // Note: Position is set directly by the movement methods (patrol, moveTowardsTarget)
    // No need to call setPosition here as it would overwrite the movement
}

void Monster::updateState(float deltaTime) {
    if (isDead()) return;
    
    MonsterState oldState = state;
    MonsterState newState = determineNextState();
    
    // Debug AI state evaluation every few seconds
    static float aiDebugTimer = 0.0f;
    aiDebugTimer += deltaTime;
    if (aiDebugTimer > 2.0f) {
        // std::cout << "=== AI STATE EVALUATION ===" << std::endl;
        // std::cout << "Monster: " << getName() << std::endl;
        // std::cout << "Current state: " << getStateName(state) << std::endl;
        // std::cout << "Evaluated next state: " << getStateName(newState) << std::endl;
        if (playerTarget) {
            float distance = getDistanceToPlayer();
            // std::cout << "Distance to player: " << distance << " units" << std::endl;
            // std::cout << "Can see player: " << (canSeePlayer() ? "YES" : "NO") << std::endl;
            // std::cout << "In danger zone: " << (isPlayerInDangerZone() ? "YES" : "NO") << std::endl;
            // std::cout << "In attack range: " << (isInAttackRange() ? "YES" : "NO") << std::endl;
            // std::cout << "Detection range: " << detectionRange << ", Danger: " << dangerRange << ", Attack: " << attackRange << std::endl;
        } else {
            // std::cout << "No player target set!" << std::endl;
        }
        // std::cout << "=========================" << std::endl;
        aiDebugTimer = 0.0f;
    }
    
    if (newState != oldState) {
        setState(newState);
    }
}

void Monster::findNewTarget() {
    if (!playerTarget) return;
    
    // Update target to player's current position
    Vec3 oldTarget = targetPosition;
    targetPosition = playerTarget->getPosition();
    
    // std::cout << "Monster " << getName() << " found new target: (" 
    //           << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
    // std::cout << "  Previous target: (" << oldTarget.x << ", " << oldTarget.y << ", " << oldTarget.z << ")" << std::endl;
}

void Monster::moveTowardsTarget(float deltaTime) {
    if (!playerTarget) return;
    
    // Update target position to player's current position (for chasing)
    targetPosition = playerTarget->getPosition();
    
    Vec3 currentPos = getPosition();
    Vec3 targetPos = targetPosition;
    
    // Use pathfinding to get direction
    Vec3 direction = findPathToTarget(targetPos);
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    // Debug output for chasing (less frequent to avoid spam)
    static float chaseDebugTimer = 0.0f;
    chaseDebugTimer += deltaTime;
    if (chaseDebugTimer > 2.0f) { // Print every 2 seconds
        // std::cout << "=== CHASING DEBUG ===" << std::endl;
        // std::cout << "Monster " << getName() << " chasing player" << std::endl;
        // std::cout << "  Current position: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
        // std::cout << "  Target position: (" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")" << std::endl;
        // std::cout << "  Path direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
        // std::cout << "  Distance: " << distance << " units" << std::endl;
        // std::cout << "  Move speed: " << moveSpeed << std::endl;
        // std::cout << "  Is stuck: " << (isStuck ? "YES" : "NO") << std::endl;
        // std::cout << "===================" << std::endl;
        chaseDebugTimer = 0.0f;
    }
    
    if (distance > 0.1f) {
        // Normalize direction
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
        
        // Set rotation to face movement direction
        setRotationFromDirection(direction);
        
        // Move towards target with smooth interpolation
        float moveDistance = moveSpeed * deltaTime;
        if (distance < moveDistance) {
            setPosition(targetPos);
            // std::cout << "Monster " << getName() << " reached target: (" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")" << std::endl;
        } else {
            Vec3 newPos = currentPos + direction * moveDistance;
            setPosition(newPos);
            
            // Debug: Print chasing movement every frame to see if it's smooth
            static int chaseMovementDebugCounter = 0;
            chaseMovementDebugCounter++;
            if (chaseMovementDebugCounter % 60 == 0) { // Print every 60 frames (1 second at 60fps)
                std::cout << "Monster " << getName() << " smooth chasing: (" 
                         << newPos.x << ", " << newPos.y << ", " << newPos.z 
                         << ") deltaTime: " << deltaTime << " moveDistance: " << moveDistance << std::endl;
            }
            
            // Update stuck detection
            updateStuckDetection(currentPos, newPos, deltaTime);
        }
    }
}

void Monster::patrol(float deltaTime) {
    moveTimer += deltaTime;
    
    // Change patrol direction every few seconds OR if we've reached our current target
    Vec3 currentPos = getPosition();
    Vec3 toTarget = targetPosition - currentPos;
    float distanceToTarget = sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
    
    // Debug: Print current patrol status every few seconds
    static float patrolDebugTimer = 0.0f;
    patrolDebugTimer += deltaTime;
    if (patrolDebugTimer > 3.0f) {
        // std::cout << "=== PATROL STATUS ===" << std::endl;
        // std::cout << "Monster " << getName() << " patrolling" << std::endl;
        // std::cout << "  Current position: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
        // std::cout << "  Target position: (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
        // std::cout << "  Distance to target: " << distanceToTarget << " units" << std::endl;
        // std::cout << "  Move timer: " << moveTimer << "s" << std::endl;
        // std::cout << "  Move speed: " << moveSpeed << std::endl;
        patrolDebugTimer = 0.0f;
    }
    
    if (moveTimer > 1.0f || distanceToTarget < 2.0f) {  // Much more frequent target changes for visible movement
        Vec3 oldTarget = targetPosition;
        targetPosition = getRandomPatrolPosition();
        moveTimer = 0.0f;
        
        // Immediately rotate to face the new patrol target
        Vec3 newDirection = targetPosition - currentPos;
        if (newDirection.x != 0.0f || newDirection.z != 0.0f) {
            setRotationFromDirection(newDirection);
        }
        
        // Debug output for patrol target changes
        // std::cout << "Monster " << getName() << " changing patrol target from (" 
        //           << oldTarget.x << ", " << oldTarget.y << ", " << oldTarget.z 
        //           << ") to (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
        // std::cout << "  Distance to old target was: " << distanceToTarget << " units" << std::endl;
        // std::cout << "  New rotation: (" << getRotation().x << ", " << getRotation().y << ", " << getRotation().z << ") degrees" << std::endl;
    }
    
    // Move towards current patrol target
    Vec3 direction = targetPosition - currentPos;
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    if (distance > 0.1f) {
        // Normalize direction
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
        
        // Set rotation to face movement direction  
        setRotationFromDirection(direction);
        
        // Move towards target with smooth interpolation
        float moveDistance = moveSpeed * deltaTime;
        if (distance < moveDistance) {
            setPosition(targetPosition);
            // std::cout << "Monster " << getName() << " reached patrol target: (" << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << ")" << std::endl;
        } else {
            Vec3 newPos = currentPos + direction * moveDistance;
            setPosition(newPos);
            
            // Debug: Print movement every frame to see if it's smooth
            static int movementDebugCounter = 0;
            movementDebugCounter++;
            if (movementDebugCounter % 60 == 0) { // Print every 60 frames (1 second at 60fps)
                std::cout << "Monster " << getName() << " smooth movement: (" 
                         << newPos.x << ", " << newPos.y << ", " << newPos.z 
                         << ") deltaTime: " << deltaTime << " moveDistance: " << moveDistance << std::endl;
            }
        }
    }
}

void Monster::attack() {
    if (!playerTarget) return;
    
    // Use stateTimer as a simple time counter for attack cooldown
    if (stateTimer >= attackCooldown) {
        // Implement actual attack logic
        // std::cout << "=== MONSTER ATTACK ===" << std::endl;
        // std::cout << "Monster " << getName() << " attacks player for " << attackDamage << " damage!" << std::endl;
        
        // Deal damage to player
        try {
            // Try to cast playerTarget to Player and call takeDamage
            Player* player = dynamic_cast<Player*>(playerTarget);
            if (player) {
                player->takeDamage(attackDamage, this);
                // std::cout << "Successfully dealt " << attackDamage << " damage to player!" << std::endl;
            } else {
                // If not a Player, try to call takeDamage if it exists
                // This is a fallback for other GameObject types that might have takeDamage
                // std::cout << "Player target is not a Player object, cannot deal damage" << std::endl;
            }
        } catch (const std::exception&) {
            // std::cout << "Error dealing damage to player: " << e.what() << std::endl;
        }
        
        // std::cout << "Attack cooldown: " << attackCooldown << " seconds" << std::endl;
        // std::cout << "=====================" << std::endl;
        
        // Reset attack timer
        lastAttackTime = stateTimer;
        stateTimer = 0.0f; // Reset state timer for next attack
    }
}

void Monster::setState(MonsterState newState) {
    if (state == newState) return;
    
    MonsterState oldState = state;
    state = newState;
    
    // Debug output for state changes
    std::string oldStateName = getStateName(oldState);
    std::string newStateName = getStateName(newState);
    // std::cout << "=== MONSTER STATE CHANGE ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    // std::cout << "State: " << oldStateName << " -> " << newStateName << std::endl;
    // std::cout << "Position: (" << getPosition().x << ", " << getPosition().y << ", " << getPosition().z << ")" << std::endl;
    if (playerTarget) {
        float distance = getDistanceToPlayer();
        // std::cout << "Distance to player: " << distance << " units" << std::endl;
        // std::cout << "Detection range: " << detectionRange << ", Danger range: " << dangerRange << ", Attack range: " << attackRange << std::endl;
        // std::cout << "In danger zone: " << (isPlayerInDangerZone() ? "YES" : "NO") << std::endl;
    }
    // std::cout << "===========================" << std::endl;
    
    // Handle state-specific behavior
    if (newState == MonsterState::Chasing) {
        // Find new target when entering chasing state
        findNewTarget();
        
        // Immediately rotate to face the player when starting to chase
        if (playerTarget) {
            Vec3 direction = playerTarget->getPosition() - getPosition();
            if (direction.x != 0.0f || direction.z != 0.0f) {
                setRotationFromDirection(direction);
                // std::cout << "Monster " << getName() << " rotated to face player when starting chase" << std::endl;
            }
        }
        
        // Check if player is in danger zone for charging behavior
        if (isPlayerInDangerZone()) {
            if (!isCharging) {
                isCharging = true;
                moveSpeed = chargeSpeed;  // Boost speed for charging
                // std::cout << "Monster " << getName() << " CHARGING! Speed boosted to " << chargeSpeed << std::endl;
            }
        } else {
            if (isCharging) {
                isCharging = false;
                moveSpeed = baseSpeed;  // Return to normal speed
                // std::cout << "Monster " << getName() << " stopped charging. Speed returned to " << baseSpeed << std::endl;
            }
        }
    } else {
        // Not chasing - use normal speed
        if (isCharging) {
            isCharging = false;
            moveSpeed = baseSpeed;  // Return to normal speed
            // std::cout << "Monster " << getName() << " stopped charging. Speed returned to " << baseSpeed << std::endl;
        }
    }
    
    // Reset timers on state change
    resetTimers();
    
    // Trigger visual effects for state change
    // flashStateChange(); // DISABLED: Causing unwanted flashing
    
    // Set pulsing based on state - DISABLED to prevent flashing
    // switch (newState) {
    //     case MonsterState::Alert:
    //         setPulsing(true, 3.0f); // Fast pulsing when alert
    //         break;
    //     case MonsterState::Chasing:
    //         setPulsing(true, 4.0f); // Very fast pulsing when chasing
    //         break;
    //     case MonsterState::Attacking:
    //         setPulsing(true, 6.0f); // Extremely fast pulsing when attacking
    //         break;
    //     default:
    //         setPulsing(false); // No pulsing for other states
    //         break;
    // }
    setPulsing(false); // Disable all pulsing to prevent flashing
    
    // Call state change callback
    onStateChange(oldState, newState);
}

void Monster::setHealth(float newHealth) {
    // Manual clamp instead of std::max/min
    health = (newHealth < 0.0f) ? 0.0f : ((newHealth > maxHealth) ? maxHealth : newHealth);
    
    if (health <= 0.0f && !isDead()) {
        die();
    }
}

void Monster::flashDamage() {
    isFlashing = true;
    damageFlashTimer = 0.5f; // Flash for 0.5 seconds (increased from 0.3)
    // std::cout << "Monster " << getName() << " flashing damage for " << damageFlashTimer << " seconds" << std::endl;
}

void Monster::updateVisualEffects(float deltaTime) {
    updateDamageFlash(deltaTime);
    updateStateVisualEffects(deltaTime);
    updatePulsingEffect(deltaTime);
    updateDeathAnimation(deltaTime);
}

Vec3 Monster::getCurrentColor() const {
    if (isFlashing) {
        return damageColor;
    }
    
    if (isStateFlashing) {
        return getStateColor();
    }
    
    // Apply pulsing effect to base color
    Vec3 baseColor = getStateColor();
    if (isPulsing) {
        float pulseIntensity = (sin(pulseTimer * pulseSpeed) + 1.0f) * 0.5f; // 0.0 to 1.0
        baseColor = baseColor * (0.7f + 0.3f * pulseIntensity); // Pulse between 70% and 100% intensity
    }
    
    return baseColor;
}

void Monster::configureMonster(MonsterType monsterType) {
    type = monsterType;
    
    switch (type) {
        case MonsterType::Xenomorph:
            maxHealth = 100.0f;
            baseSpeed = 2.0f;      // Slower for better gameplay
            moveSpeed = baseSpeed;
            chargeSpeed = 4.0f;    // Slower charging
            attackRange = 2.0f;
            attackDamage = 25.0f;
            detectionRange = 5.0f;  // Reduced detection range
            dangerRange = 8.0f;     // Reduced danger zone
            originalColor = Vec3(0.5f, 0.14f, 0.58f); // Purple
            aggressionLevel = 0.6f; // Moderate aggression
            fearLevel = 0.2f;       // Low fear
            experienceReward = 10;  // Standard XP reward
            scoreReward = 100;      // Standard score reward
            break;
            
        case MonsterType::Runner:
            maxHealth = 50.0f;
            baseSpeed = 3.0f;      // Slower runners
            moveSpeed = baseSpeed;
            chargeSpeed = 6.0f;    // Slower charging
            attackRange = 1.5f;
            attackDamage = 15.0f;
            detectionRange = 15.0f;
            dangerRange = 22.0f;  // Runners have large danger zones
            originalColor = Vec3(0.2f, 0.8f, 0.2f); // Green
            aggressionLevel = 0.8f; // High aggression
            fearLevel = 0.1f;       // Very low fear
            experienceReward = 15;  // Higher XP for fast, hard-to-hit enemies
            scoreReward = 150;      // Higher score for difficulty
            break;
            
        case MonsterType::Tank:
            maxHealth = 200.0f;
            baseSpeed = 1.5f;      // Slower tanks
            moveSpeed = baseSpeed;
            chargeSpeed = 3.0f;    // Slower charging
            attackRange = 3.0f;
            attackDamage = 40.0f;
            detectionRange = 8.0f;
            dangerRange = 12.0f;  // Tanks have smaller but still significant danger zones
            originalColor = Vec3(0.8f, 0.2f, 0.2f); // Red
            aggressionLevel = 0.4f; // Lower aggression but persistent
            fearLevel = 0.3f;       // Higher fear due to slow movement
            experienceReward = 25;  // Highest XP for toughest enemies
            scoreReward = 250;      // Highest score for tanks
            break;
    }
    
    health = maxHealth;
}

// Override to use Monster renderer
RendererType Monster::getPreferredRendererType() const {
    return RendererType::Monster;
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

bool Monster::isPlayerInDangerZone() const {
    if (!playerTarget) return false;
    
    float distance = getDistanceToPlayer();
    return distance <= dangerRange;
}

Vec3 Monster::getRandomPatrolPosition() const {
    Vec3 currentPos = getPosition();
    
    // Use monster ID to create unique random seed for each monster
    // This ensures each monster gets different "random" positions
    unsigned int monsterSeed = static_cast<unsigned int>(std::hash<std::string>{}(getName())) + static_cast<unsigned int>(time(nullptr));
    srand(monsterSeed);
    
    // Generate random offset within patrol radius with MINIMUM distance for visible movement
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    float distance = 5.0f + (float)(rand() % (int)(patrolRadius - 5.0f)); // Minimum 5 units for visible movement
    
    Vec3 offset;
    offset.x = cos(angle) * distance;
    offset.z = sin(angle) * distance;
    offset.y = 0.0f; // Keep on same height
    
    Vec3 newTarget = currentPos + offset;
    
    // Debug output for patrol target generation
    // std::cout << "=== PATROL TARGET GENERATION ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    // std::cout << "Current position: (" << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
    // std::cout << "Generated target: (" << newTarget.x << ", " << newTarget.y << ", " << newTarget.z << ")" << std::endl;
    // std::cout << "Offset: (" << offset.x << ", " << offset.y << ", " << offset.z << ")" << std::endl;
    // std::cout << "Angle: " << (angle * 180.0f / 3.14159f) << "°, Distance: " << distance << " units" << std::endl;
    // std::cout << "Distance to target: " << sqrt(offset.x * offset.x + offset.z * offset.z) << " units" << std::endl;
    // std::cout << "===============================" << std::endl;
    
    return newTarget;
}

void Monster::resetTimers() {
    moveTimer = 0.0f;
    stateTimer = 0.0f;
}

std::string Monster::getStateName(MonsterState state) const {
    switch (state) {
        case MonsterState::Idle: return "Idle";
        case MonsterState::Patrolling: return "Patrolling";
        case MonsterState::Alert: return "Alert";
        case MonsterState::Chasing: return "Chasing";
        case MonsterState::Attacking: return "Attacking";
        case MonsterState::Stunned: return "Stunned";
        case MonsterState::Retreating: return "Retreating";
        case MonsterState::Dead: return "Dead";
        default: return "Unknown";
    }
}

void Monster::setTargetPosition(const Vec3& position) {
    targetPosition = position;
    // std::cout << "Monster " << getName() << " target position set to: (" 
    //           << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void Monster::setRotationFromDirection(const Vec3& direction) {
    // Calculate rotation to face the movement direction
    if (direction.x == 0.0f && direction.z == 0.0f) {
        return; // No movement, no rotation change
    }
    
    // Calculate yaw angle (rotation around Y-axis)
    // atan2(x, z) gives us the angle from the positive Z-axis towards the positive X-axis
    float yaw = atan2(direction.x, direction.z);
    
    // Convert to degrees for GameObject rotation system (which expects degrees)
    float yawDegrees = yaw * 180.0f / 3.14159265359f; // Use explicit PI value
    
    // Get current rotation and update only the Y component (yaw)
    Vec3 currentRotation = getRotation();
    Vec3 oldRotation = currentRotation;
    currentRotation.y = yawDegrees; // Set Y rotation to face movement direction
    setRotation(currentRotation);
    
    // Debug output (less frequent to avoid spam)
    static float rotationDebugTimer = 0.0f;
    rotationDebugTimer += 0.016f; // Approximate frame time
    
    if (rotationDebugTimer > 2.0f) { // Debug every 2 seconds
        // std::cout << "=== MONSTER ROTATION ===" << std::endl;
        // std::cout << "Monster: " << getName() << std::endl;
        // std::cout << "Movement direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
        // std::cout << "Old rotation: (" << oldRotation.x << ", " << oldRotation.y << ", " << oldRotation.z << ") degrees" << std::endl;
        // std::cout << "New rotation: (" << currentRotation.x << ", " << currentRotation.y << ", " << currentRotation.z << ") degrees" << std::endl;
        // std::cout << "Yaw angle: " << yawDegrees << " degrees" << std::endl;
        // std::cout << "=======================" << std::endl;
        rotationDebugTimer = 0.0f;
    }
}

float Monster::getCollisionRadius() const {
    // Return collision radius based on monster type
    switch (type) {
        case MonsterType::Xenomorph:
            return 1.5f; // Larger radius for easier hitting
        case MonsterType::Runner:
            return 1.2f; // Smaller, faster target
        case MonsterType::Tank:
            return 2.0f; // Large, slow target
        default:
            return 1.5f;
    }
}

Vec3 Monster::getCollisionCenter() const {
    Vec3 basePos = getPosition();
    // Adjust collision center to be at the center of the monster's body
    // This accounts for the fact that monster position is at ground level
    basePos.y += 1.0f; // Move collision center up by 1 unit to center it on the body
    return basePos;
}

void Monster::onDamage(float damage, GameObject* attacker) {
    // Override point for custom damage behavior
}

void Monster::onDeath() {
    // Override point for custom death behavior
    // std::cout << "=== onDeath() CALLBACK STARTING ===" << std::endl;
    // std::cout << "Monster " << getName() << " died!" << std::endl;
    // std::cout << "=== onDeath() CALLBACK COMPLETED ===" << std::endl;
}

void Monster::onStateChange(MonsterState oldState, MonsterState newState) {
    // Override point for custom state change behavior
}

void Monster::setupMonsterMesh() {
    // Load the actual Xenomorph model
    std::string modelPath = "Resources/Objects/Xenomorph/model.obj";
    
    // std::cout << "Loading Xenomorph model from: " << modelPath << std::endl;
    
    // Load the OBJ model data
    OBJMeshData meshData = OBJLoader::loadOBJ(modelPath, 1.0f); // Scale of 1.0
    
    // Load materials from MTL file
    std::string mtlPath = MaterialLoader::getMTLPathFromOBJ(modelPath);
    // std::cout << "Trying to load MTL file from: " << mtlPath << std::endl;
    
    if (MaterialLoader::isValidMTLFile(mtlPath)) {
        monsterMaterials = MaterialLoader::loadMTL(mtlPath);
        // std::cout << "Loaded " << monsterMaterials.getMaterialCount() << " materials for monster" << std::endl;
        
        // Create material groups for multi-material rendering
        createMaterialGroups(meshData);
    } else {
        // std::cout << "Warning: MTL file not found at " << mtlPath << std::endl;
        // std::cout << "Trying alternative path: Resources/Objects/Xenomorph/materials.mtl" << std::endl;
        
        // Try alternative path
        std::string altMtlPath = "Resources/Objects/Xenomorph/materials.mtl";
        if (MaterialLoader::isValidMTLFile(altMtlPath)) {
            monsterMaterials = MaterialLoader::loadMTL(altMtlPath);
            // std::cout << "Loaded " << monsterMaterials.getMaterialCount() << " materials from alternative path" << std::endl;
            createMaterialGroups(meshData);
        } else {
            // std::cout << "Failed to load materials from both paths" << std::endl;
        }
    }
    
    if (!meshData.isValid()) {
        // std::cerr << "Failed to load Xenomorph model for '" << getName() << "', falling back to cube" << std::endl;
        
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
            // std::cerr << "Failed to create fallback cube mesh for '" << getName() << "'" << std::endl;
        } else {
            // std::cout << "Created fallback cube mesh for '" << getName() << "'" << std::endl;
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
    
    // std::cout << "=== XENOMORPH MESH CONVERSION ===" << std::endl;
    // std::cout << "Original OBJ data: " << meshData.vertices.size() << " floats" << std::endl;
    // std::cout << "Converted data: " << basicVertexData.size() << " floats" << std::endl;
    // std::cout << "Vertices: " << (meshData.vertices.size() / 8) << ", Indices: " << meshData.indices.size() << std::endl;
    // std::cout << "First few vertices: ";
    for (int i = 0; i < std::min(10, (int)basicVertexData.size()); i++) {
        // std::cout << basicVertexData[i] << " ";
    }
    // std::cout << "..." << std::endl;
    
    if (!mesh->createMeshWithTexCoords(basicVertexData, meshData.indices)) {
        // std::cerr << "Failed to create Xenomorph mesh for '" << getName() << "'" << std::endl;
    } else {
        // std::cout << "Successfully loaded Xenomorph model for '" << getName() << "'" << std::endl;
        // std::cout << "  Vertices: " << meshData.vertexCount << std::endl;
        // std::cout << "  Triangles: " << meshData.triangleCount << std::endl;
        // std::cout << "  Bounds: " << meshData.boundingBoxMin.x << "," << meshData.boundingBoxMin.y << "," << meshData.boundingBoxMin.z 
        //           << " to " << meshData.boundingBoxMax.x << "," << meshData.boundingBoxMax.y << "," << meshData.boundingBoxMax.z << std::endl;
    }
}

void Monster::setupMonsterMaterial() {
    // Materials are now loaded in setupMonsterMesh() from the MTL file
    // Set default color for fallback rendering
    setColor(originalColor);
    
    // std::cout << "=== MONSTER MATERIAL SETUP ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    // std::cout << "Original color: " << originalColor.x << ", " << originalColor.y << ", " << originalColor.z << std::endl;
    
    if (monsterMaterials.getMaterialCount() > 0) {
        // std::cout << "  Loaded " << monsterMaterials.getMaterialCount() << " materials from MTL file" << std::endl;
        // std::cout << "  Created " << materialGroups.size() << " material groups" << std::endl;
        
        // Debug: List material group colors
        for (size_t i = 0; i < materialGroups.size(); i++) {
            // std::cout << "    Group " << i << " color: " << materialGroups[i].color.x << ", " 
            //               << materialGroups[i].color.y << ", " << materialGroups[i].color.z << std::endl;
        }
    } else {
        // std::cout << "  No materials loaded - using fallback color" << std::endl;
        // std::cout << "  Fallback color: " << originalColor.x << ", " << originalColor.y << ", " << originalColor.z << std::endl;
    }
    
    // CRITICAL: Verify the color was set
    Vec3 currentSetColor = getCurrentColor();
    // std::cout << "  Final set color: " << currentSetColor.x << ", " << currentSetColor.y << ", " << currentSetColor.z << std::endl;
    // std::cout << "=========================" << std::endl;
}

void Monster::createMaterialGroups(const OBJMeshData& objData) {
    // Implementation for creating material groups from OBJ data
    // Parse the OBJ data and create material groups based on material assignments
    // std::cout << "Creating material groups from OBJ data..." << std::endl;
    
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
                
                // std::cout << "  Material group '" << materialName << "': " 
                //           << group.indices.size() << " indices, color(" 
                //           << group.color.x << ", " << group.color.y << ", " << group.color.z << ")" << std::endl;
            }
        }
    }
    
    // std::cout << "Created " << materialGroups.size() << " material groups for monster" << std::endl;
}

void Monster::updateDamageFlash(float deltaTime) {
    if (isFlashing) {
        damageFlashTimer -= deltaTime;
        if (damageFlashTimer <= 0.0f) {
            isFlashing = false;
            // std::cout << "Monster " << getName() << " damage flash ended" << std::endl;
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
    
    // If stunned, stay stunned until timer expires
    if (isStunned()) {
        return MonsterState::Stunned;
    }
    
    // Check if monster should retreat (low health + high fear)
    if (shouldRetreat()) {
        return MonsterState::Retreating;
    }
    
    // Check if player is in attack range
    if (isInAttackRange() && hasLineOfSight) {
            return MonsterState::Attacking;
    }
    
    // Check if player is in danger zone (aggressive behavior)
    if (isPlayerInDangerZone() && hasLineOfSight) {
            return MonsterState::Chasing;
    }
    
    // Check if player is visible (normal detection)
    if (hasLineOfSight) {
        // If we just spotted the player, go to alert first
        if (state == MonsterState::Patrolling || state == MonsterState::Idle) {
            return MonsterState::Alert;
        }
            return MonsterState::Chasing;
        }
    
    // If we lost sight of player but have a last known position
    if (state == MonsterState::Chasing && !hasLineOfSight && 
        getDistanceToLastKnownPlayerPos() > 2.0f) {
        return MonsterState::Alert; // Search for player
    }
    
    // If we've been alert for too long without seeing player, go back to patrolling
    if (state == MonsterState::Alert && alertTimer > 5.0f) {
    return MonsterState::Patrolling;
}

    // Default to patrolling
    return MonsterState::Patrolling;
}

// Health bar methods - NEW: Using texture-based system
void Monster::updateHealthBar() {
    if (textureHealthBar && showHealthBar) {
        textureHealthBar->setHealth(health, maxHealth);
        textureHealthBar->update(0.016f); // Approximate frame time
    }
}

void Monster::renderHealthBar(const Camera& camera) {
    // IMMEDIATE DEBUG: Check why health bars aren't rendering
    static int immediateDebugCount = 0;
    immediateDebugCount++;
    if (immediateDebugCount % 30 == 0) { // Every 0.5 seconds at 60fps
        std::cout << "*** MONSTER RENDER HEALTH BAR CALLED ***" << std::endl;
        std::cout << "Monster: " << getName() << std::endl;
        std::cout << "textureHealthBar exists: " << (textureHealthBar ? "YES" : "NO") << std::endl;
        std::cout << "showHealthBar: " << (showHealthBar ? "YES" : "NO") << std::endl;
        std::cout << "health: " << health << "/" << maxHealth << std::endl;
        std::cout << "isAlive(): " << (isAlive() ? "YES" : "NO") << std::endl;
        std::cout << "******************************************" << std::endl;
    }
    
    // Only render health bars for alive monsters
    if (textureHealthBar && showHealthBar && isAlive()) {
        // Debug: Compare raw position vs model matrix position
        Vec3 rawPosition = getPosition();
        Mat4 monsterMatrix = getModelMatrix();
        Vec3 matrixPosition = Vec3(monsterMatrix.m[12], monsterMatrix.m[13], monsterMatrix.m[14]);
        
        static int debugCount = 0;
        debugCount++;
        if (debugCount % 60 == 0) { // Every 1 second
            std::cout << "=== MONSTER POSITION INVESTIGATION ===" << std::endl;
            std::cout << "Monster name: " << getName() << std::endl;
            std::cout << "Raw monster position: (" << rawPosition.x << ", " << rawPosition.y << ", " << rawPosition.z << ")" << std::endl;
            std::cout << "Matrix position: (" << matrixPosition.x << ", " << matrixPosition.y << ", " << matrixPosition.z << ")" << std::endl;
            std::cout << "Position difference: (" << (matrixPosition.x - rawPosition.x) << ", " << (matrixPosition.y - rawPosition.y) << ", " << (matrixPosition.z - rawPosition.z) << ")" << std::endl;
            
            // Check if monster is actually being rendered
            std::cout << "Monster active: " << (isActive ? "YES" : "NO") << std::endl;
            std::cout << "Monster health: " << health << "/" << maxHealth << std::endl;
            std::cout << "Monster state: " << static_cast<int>(state) << std::endl;
            
            // Print the actual model matrix to see what's happening
            std::cout << "Model matrix:" << std::endl;
            for (int i = 0; i < 4; i++) {
                std::cout << "[" << monsterMatrix.m[i*4] << ", " << monsterMatrix.m[i*4+1] << ", " << monsterMatrix.m[i*4+2] << ", " << monsterMatrix.m[i*4+3] << "]" << std::endl;
            }
            std::cout << "======================================" << std::endl;
        }
        
        // Use model matrix position (now that transformation order is fixed)
        
        // Additional debug: Show what position we're passing to health bar
        if (debugCount % 600 == 0) {
            std::cout << "=== PASSING TO HEALTH BAR ===" << std::endl;
            std::cout << "Monster matrix position passed to health bar: (" << matrixPosition.x << ", " << matrixPosition.y << ", " << matrixPosition.z << ")" << std::endl;
            std::cout << "Expected health bar world position: (" << matrixPosition.x << ", " << (matrixPosition.y + 2.5f) << ", " << (matrixPosition.z + 0.1f) << ")" << std::endl;
            std::cout << "=============================" << std::endl;
        }
        
        textureHealthBar->render(matrixPosition, camera);
    }
}

// MonsterSpawner implementation
MonsterSpawner::MonsterSpawner(Scene* scene, GameObject* player)
    : gameScene(scene),
      playerTarget(player),
      maxMonsters(3),  // TESTING: Spawn 3 monsters to test health bar positioning
      spawnInterval(0.1f),  // TESTING: Spawn extremely fast for multiple monster testing
      lastSpawnTime(0.0f),
      spawnRadius(8.0f),  // Spread monsters out for better health bar visibility testing
      spawnCenter(10.0f, 0.0f, 10.0f),  // Spawn CLOSE to player starting position
      monsterTypes({MonsterType::Xenomorph}),
      activeMonsters(),
      currentWave(0),
      monstersInCurrentWave(0),
      monstersSpawnedInWave(0),
      waveStartTime(0.0f),
      waveDuration(60.0f),  // 60 seconds per wave
      waveInProgress(false),
      timeBetweenWaves(10.0f),  // 10 seconds between waves
      lastWaveEndTime(0.0f),
      difficultyLevel(1.0f),
      difficultyIncreaseRate(0.1f),
      lastDifficultyIncrease(0.0f),
      difficultyIncreaseInterval(30.0f) {  // Increase difficulty every 30 seconds
    
    // Seed random number generator for spawn positions
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // std::cout << "MonsterSpawner initialized with spawn center: (" << spawnCenter.x << ", " << spawnCenter.y << ", " << spawnCenter.z << ")" << std::endl;
    // std::cout << "Spawn radius: " << spawnRadius << " units" << std::endl;
    // std::cout << "Spawn interval: " << spawnInterval << " seconds" << std::endl;
}



void MonsterSpawner::update(float deltaTime) {
    if (!gameScene || !playerTarget) return;
    
    // Update difficulty
    updateDifficulty(deltaTime);
    
    // Update spawn timer
    lastSpawnTime += deltaTime;
    
    // Check if we should start a new wave
    if (!waveInProgress && shouldStartNewWave()) {
        startNewWave();
    }
    
    // Check if current wave should end
    if (waveInProgress && (lastSpawnTime >= waveDuration || monstersSpawnedInWave >= monstersInCurrentWave)) {
        endCurrentWave();
    }
    
    // Check if we should spawn a new monster (only during waves)
    if (waveInProgress && shouldSpawnMonster()) {
        std::cout << "=== SPAWNING NEW MONSTER (Wave " << currentWave << ") ===" << std::endl;
        spawnRandomMonster();
        lastSpawnTime = 0.0f;
        monstersSpawnedInWave++;
    }
    
    // CRASH PREVENTION: Check for dead monsters but don't remove them
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 3.0f) { // Print every 3 seconds instead of every frame
        // std::cout << "=== MONSTER SPAWNER UPDATE ===" << std::endl;
        // std::cout << "Active monsters in tracking: " << activeMonsters.size() << std::endl;
        for (const auto& monster : activeMonsters) {
            if (monster) {
                try {
                    // CRASH PREVENTION: Check if monster is still valid before accessing
                    if (monster->getActive() && !monster->isDead()) {
                        // std::cout << "Monster: " << monster->getName() << " - Dead: NO" << std::endl;
                    } else {
                        // std::cout << "Monster: " << monster->getName() << " - Dead: YES (or inactive)" << std::endl;
                    }
                } catch (const std::exception&) {
                    // std::cout << "Error accessing monster: " << e.what() << std::endl;
                }
            }
        }
        // std::cout << "=============================" << std::endl;
        debugTimer = 0.0f;
    }
    
    // NOTE: Monster updates are now handled by the Scene update cycle
    // This allows for proper full monster behavior including movement and AI
    
    // Monster updates are now handled by the Scene update cycle
    // This enables full monster behavior including movement and AI
}

void MonsterSpawner::cleanup() {
    clearAllMonsters();
    gameScene = nullptr;
    playerTarget = nullptr;
}

void MonsterSpawner::spawnMonster(MonsterType type) {
    if (!gameScene) return;
    
    // Count only alive monsters
    int aliveMonsters = 0;
    for (const auto& monster : activeMonsters) {
        if (monster && monster->getActive() && !monster->isDead()) {
            aliveMonsters++;
        }
    }
    
    if (aliveMonsters >= maxMonsters) return;
    
    Vec3 spawnPos = getRandomSpawnPosition();
    spawnMonsterAt(spawnPos, type);
}

void MonsterSpawner::spawnMonsterAt(const Vec3& position, MonsterType type) {
    if (!gameScene) return;
    
    // Count only alive monsters
    int aliveMonsters = 0;
    for (const auto& monster : activeMonsters) {
        if (monster && monster->getActive() && !monster->isDead()) {
            aliveMonsters++;
        }
    }
    
    if (aliveMonsters >= maxMonsters) return;
    
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
        
        // Health bar is now rendered inline - no separate object needed
        
        // Store reference in our active monsters list
        activeMonsters.push_back(monsterPtr);
        
        std::cout << "=== MONSTER SPAWNED ===" << std::endl;
        std::cout << "Spawned " << monsterName << " at " << position.x << ", " << position.y << ", " << position.z << std::endl;
        std::cout << "Monster entity flag: " << (monsterPtr->getEntity() ? "true" : "false") << std::endl;
        std::cout << "Monster active: " << (monsterPtr->getActive() ? "true" : "false") << std::endl;
        std::cout << "Monster renderer type: " << static_cast<int>(monsterPtr->getPreferredRendererType()) << std::endl;
        std::cout << "Monster position: (" << monsterPtr->getPosition().x << ", " << monsterPtr->getPosition().y << ", " << monsterPtr->getPosition().z << ")" << std::endl;
        std::cout << "Monster state: " << monsterPtr->getStateName(monsterPtr->getState()) << std::endl;
        std::cout << "Player target: " << (monsterPtr->getPlayerTarget() ? "SET" : "NULL") << std::endl;
        // std::cout << "Total active monsters: " << activeMonsters.size() << std::endl;
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

void MonsterSpawner::removeMonster(Monster* monster) {
    if (!monster) return;
    
    // Remove monster from active list
    activeMonsters.erase(
        std::remove_if(activeMonsters.begin(), activeMonsters.end(),
            [monster](const Monster* m) {
                return m == monster;
            }),
        activeMonsters.end()
    );
    
    // std::cout << "Monster removed from MonsterSpawner: " << (monster ? monster->getName() : "Unknown") << std::endl;
}

void MonsterSpawner::addSpawnPoint(const Vec3& point) {
    spawnPoints.push_back(point);
}

void MonsterSpawner::addMonsterType(MonsterType type) {
    monsterTypes.push_back(type);
}

void MonsterSpawner::removeDeadMonsters() {
    // CRASH PREVENTION: COMPLETELY DISABLE DEAD MONSTER CLEANUP
    // This method was causing crashes when trying to access dead monsters
    // Dead monsters will remain in tracking forever to prevent any access attempts
    
    // DO NOTHING - let dead monsters exist in tracking without cleanup
    // This prevents any method calls on potentially corrupted monster objects
    
    static int cleanupCallCount = 0;
    cleanupCallCount++;
    
    if (cleanupCallCount % 180 == 0) { // Print every 3 seconds
        // std::cout << "=== MONSTER CLEANUP DISABLED ===" << std::endl;
        // std::cout << "Active monsters in tracking: " << activeMonsters.size() << std::endl;
        // std::cout << "NOTE: Dead monsters remain in tracking to prevent crashes" << std::endl;
        // std::cout << "===============================" << std::endl;
    }
}

Vec3 MonsterSpawner::getRandomSpawnPosition() const {
    if (spawnPoints.empty()) {
        // Use current time and spawn count to create unique spawn positions
        static int spawnCount = 0;
        spawnCount++;
        
        // Create unique seed for each spawn with better randomization
        unsigned int spawnSeed = static_cast<unsigned int>(time(nullptr)) + spawnCount * 1000 + (spawnCount * 7);
        srand(spawnSeed);
        
        // Generate random position around spawn center - CLOSE to player
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float distance = 2.0f + (float)(rand() % (int)(spawnRadius - 2.0f)); // Minimum 2 units from center
        
        Vec3 pos = spawnCenter;
        pos.x += cos(angle) * distance;
        pos.z += sin(angle) * distance;
        
        // Debug output for spawn position
        // std::cout << "=== SPAWN POSITION GENERATION ===" << std::endl;
        // std::cout << "Spawn #" << spawnCount << " generated at: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
        // std::cout << "  From center: (" << spawnCenter.x << ", " << spawnCenter.y << ", " << spawnCenter.z << ")" << std::endl;
        // std::cout << "  Angle: " << (angle * 180.0f / 3.14159f) << "°, Distance: " << distance << " units" << std::endl;
        // std::cout << "  Spawn radius: " << spawnRadius << " units" << std::endl;
        // std::cout << "  Distance from origin: " << sqrt(pos.x * pos.x + pos.z * pos.z) << " units" << std::endl;
        // std::cout << "================================" << std::endl;
        
        return pos;
    }
    
    // Pick random spawn point
    int index = rand() % spawnPoints.size();
    Vec3 pos = spawnPoints[index];
    // std::cout << "Using predefined spawn point " << index << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    return pos;
}

MonsterType MonsterSpawner::getRandomMonsterType() const {
    if (monsterTypes.empty()) {
        return MonsterType::Xenomorph;
    }
    
    int index = rand() % monsterTypes.size();
    return monsterTypes[index];
}

bool MonsterSpawner::shouldSpawnMonster() const {
    // Count only alive monsters
    int aliveMonsters = 0;
    for (const auto& monster : activeMonsters) {
        if (monster && monster->getActive() && !monster->isDead()) {
            aliveMonsters++;
        }
    }
    
    return aliveMonsters < maxMonsters && lastSpawnTime >= spawnInterval;
}

// Wave system method implementations
void MonsterSpawner::startNewWave() {
    currentWave++;
    waveInProgress = true;
    waveStartTime = 0.0f;
    monstersSpawnedInWave = 0;
    
    // TESTING: Spawn 3 monsters per wave for health bar positioning test
    monstersInCurrentWave = 3;
    
    // Adjust spawn interval based on difficulty
    spawnInterval = std::max(0.5f, 3.0f - (difficultyLevel - 1.0f) * 0.5f);
    
    std::cout << "=== WAVE " << currentWave << " STARTED ===" << std::endl;
    std::cout << "Monsters in wave: " << monstersInCurrentWave << std::endl;
    std::cout << "Difficulty level: " << difficultyLevel << std::endl;
    std::cout << "Spawn interval: " << spawnInterval << " seconds" << std::endl;
    std::cout << "=========================" << std::endl;
}

void MonsterSpawner::endCurrentWave() {
    waveInProgress = false;
    lastWaveEndTime = 0.0f;
    
    // std::cout << "=== WAVE " << currentWave << " ENDED ===" << std::endl;
    // std::cout << "Monsters spawned: " << monstersSpawnedInWave << "/" << monstersInCurrentWave << std::endl;
    // std::cout << "Next wave in: " << timeBetweenWaves << " seconds" << std::endl;
    // std::cout << "========================" << std::endl;
}

float MonsterSpawner::getWaveProgress() const {
    if (!waveInProgress) return 0.0f;
    
    float timeProgress = lastSpawnTime / waveDuration;
    float monsterProgress = static_cast<float>(monstersSpawnedInWave) / monstersInCurrentWave;
    
    return std::min(1.0f, std::max(timeProgress, monsterProgress));
}

void MonsterSpawner::updateDifficulty(float deltaTime) {
    lastDifficultyIncrease += deltaTime;
    
    if (lastDifficultyIncrease >= difficultyIncreaseInterval) {
        increaseDifficulty();
        lastDifficultyIncrease = 0.0f;
    }
}

void MonsterSpawner::increaseDifficulty(float amount) {
    difficultyLevel += amount;
    difficultyLevel = std::min(5.0f, difficultyLevel); // Cap at 5.0
    
    // Adjust max monsters based on difficulty
    maxMonsters = static_cast<int>(3 + difficultyLevel * 2);
    
    // std::cout << "=== DIFFICULTY INCREASED ===" << std::endl;
    // std::cout << "New difficulty level: " << difficultyLevel << std::endl;
    // std::cout << "Max monsters: " << maxMonsters << std::endl;
    // std::cout << "===========================" << std::endl;
}

bool MonsterSpawner::shouldStartNewWave() const {
    if (waveInProgress) return false;
    
    // Start first wave immediately
    if (currentWave == 0) return true;
    
    // Check if enough time has passed since last wave
    return lastWaveEndTime >= timeBetweenWaves;
}

bool MonsterSpawner::isWaveInProgress() const {
    return waveInProgress;
}

int MonsterSpawner::getCurrentWave() const {
    return currentWave;
}

int MonsterSpawner::getMonstersInCurrentWave() const {
    return monstersInCurrentWave;
}

int MonsterSpawner::getMonstersSpawnedInWave() const {
    return monstersSpawnedInWave;
}

float MonsterSpawner::getDifficultyLevel() const {
    return difficultyLevel;
}

size_t MonsterSpawner::getActiveMonsterCount() const {
    return activeMonsters.size();
}

const std::vector<Monster*>& MonsterSpawner::getActiveMonsters() const {
    return activeMonsters;
}

void MonsterSpawner::setDifficultyLevel(float level) {
    difficultyLevel = std::max(0.1f, std::min(5.0f, level));
}

void MonsterSpawner::setMaxMonsters(int max) {
    maxMonsters = max;
}

void MonsterSpawner::setSpawnInterval(float interval) {
    spawnInterval = interval;
}

void MonsterSpawner::setSpawnRadius(float radius) {
    spawnRadius = radius;
}

void MonsterSpawner::setSpawnCenter(const Vec3& center) {
    spawnCenter = center;
}

// Enhanced AI method implementations
void Monster::updateTimers(float deltaTime) {
    stateTimer += deltaTime;
    
    if (alertTimer > 0.0f) {
        alertTimer += deltaTime;
    }
    
    if (stunTimer > 0.0f) {
        stunTimer -= deltaTime;
        if (stunTimer <= 0.0f) {
            stunTimer = 0.0f;
            // std::cout << "Monster " << getName() << " recovered from stun" << std::endl;
        }
    }
    
    if (retreatTimer > 0.0f) {
        retreatTimer += deltaTime;
    }
}

void Monster::updateBehaviorModifiers(float deltaTime) {
    // Update aggression based on damage taken and player proximity
    float distanceToPlayer = getDistanceToPlayer();
    float threatLevel = calculateThreatLevel();
    
    // Increase aggression when taking damage or when player is close
    if (threatLevel > 0.5f) {
        aggressionLevel = std::min(1.0f, aggressionLevel + deltaTime * 0.5f);
    } else {
        // Gradually decrease aggression when not threatened
        aggressionLevel = std::max(0.1f, aggressionLevel - deltaTime * 0.1f);
    }
    
    // Update fear based on health and player strength
    float healthPercentage = getHealthPercentage();
    if (healthPercentage < 0.3f) {
        fearLevel = std::min(1.0f, fearLevel + deltaTime * 0.3f);
    } else {
        fearLevel = std::max(0.0f, fearLevel - deltaTime * 0.1f);
    }
}

void Monster::updateLineOfSight() {
    if (!playerTarget) {
        hasLineOfSight = false;
        return;
    }
    
    hasLineOfSight = checkLineOfSightToPlayer();
    
    if (hasLineOfSight) {
        lastPlayerSeenTime = stateTimer;
        updateLastKnownPlayerPosition();
    }
}

void Monster::updateAggression(float deltaTime) {
    // This method is called from updateBehaviorModifiers
    // Aggression is updated there to avoid duplicate code
}

void Monster::updateFear(float deltaTime) {
    // This method is called from updateBehaviorModifiers
    // Fear is updated there to avoid duplicate code
}

void Monster::retreat(float deltaTime) {
    if (!playerTarget) return;
    
    retreatTimer += deltaTime;
    
    // Move away from player
    Vec3 currentPos = getPosition();
    Vec3 playerPos = playerTarget->getPosition();
    Vec3 direction = currentPos - playerPos; // Direction away from player
    
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    
    if (distance > 0.1f) {
        // Normalize direction
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
        
        // Set rotation to face retreat direction
        setRotationFromDirection(direction);
        
        // Move away from player at reduced speed
        float retreatSpeed = moveSpeed * 0.7f; // Slower when retreating
        float moveDistance = retreatSpeed * deltaTime;
        
        Vec3 newPos = currentPos + direction * moveDistance;
        setPosition(newPos);
        
        // std::cout << "Monster " << getName() << " retreating from player" << std::endl;
    }
    
    // Stop retreating after some time or if health improves
    if (retreatTimer > 3.0f || getHealthPercentage() > 0.5f) {
        setState(MonsterState::Patrolling);
        retreatTimer = 0.0f;
    }
}

void Monster::becomeAlert() {
    if (state != MonsterState::Alert) {
        setState(MonsterState::Alert);
        alertTimer = 0.0f;
        // std::cout << "Monster " << getName() << " became alert!" << std::endl;
    }
}

void Monster::becomeStunned(float duration) {
    stunTimer = duration;
    setState(MonsterState::Stunned);
    // std::cout << "Monster " << getName() << " stunned for " << duration << " seconds" << std::endl;
}

bool Monster::hasPlayerInSight() const {
    return hasLineOfSight;
}

float Monster::getDistanceToLastKnownPlayerPos() const {
    Vec3 currentPos = getPosition();
    Vec3 diff = lastKnownPlayerPos - currentPos;
    return sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

bool Monster::checkLineOfSightToPlayer() const {
    if (!playerTarget) return false;
    
    // Simple line of sight check - can be enhanced with raycasting
    float distance = getDistanceToPlayer();
    return distance <= detectionRange;
}

void Monster::updateLastKnownPlayerPosition() {
    if (playerTarget) {
        lastKnownPlayerPos = playerTarget->getPosition();
    }
}

float Monster::calculateThreatLevel() const {
    if (!playerTarget) return 0.0f;
    
    float distance = getDistanceToPlayer();
    float healthPercentage = getHealthPercentage();
    
    // Threat increases as player gets closer and monster health decreases
    float distanceThreat = 1.0f - (distance / detectionRange);
    float healthThreat = 1.0f - healthPercentage;
    
    return (distanceThreat + healthThreat) * 0.5f;
}

// Pathfinding method implementations
void Monster::updatePathfinding(float deltaTime) {
    pathUpdateTimer += deltaTime;
    
    // Update stuck detection
    Vec3 currentPos = getPosition();
    if (lastPosition.x != 0.0f || lastPosition.y != 0.0f || lastPosition.z != 0.0f) {
        float distanceMoved = sqrt(
            (currentPos.x - lastPosition.x) * (currentPos.x - lastPosition.x) +
            (currentPos.y - lastPosition.y) * (currentPos.y - lastPosition.y) +
            (currentPos.z - lastPosition.z) * (currentPos.z - lastPosition.z)
        );
        
        if (distanceMoved < 0.1f) { // Not moving much
            stuckTimer += deltaTime;
            if (stuckTimer > 2.0f) { // Stuck for 2 seconds
                isStuck = true;
                // std::cout << "Monster " << getName() << " is stuck!" << std::endl;
            }
        } else {
            stuckTimer = 0.0f;
            isStuck = false;
        }
    }
    
    lastPosition = currentPos;
}

Vec3 Monster::findPathToTarget(const Vec3& target) {
    Vec3 currentPos = getPosition();
    Vec3 direction = target - currentPos;
    
    // CRITICAL FIX: Normalize the direction vector to prevent teleporting!
    float distance = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (distance > 0.1f) {
        direction.x /= distance;
        direction.y /= distance;
        direction.z /= distance;
    }
    
    // If stuck, try to get unstuck
    if (isStuck) {
        direction = getRandomDirection();
        // std::cout << "Monster " << getName() << " using random direction to get unstuck" << std::endl;
    }
    
    // Apply obstacle avoidance
    direction = avoidObstacles(direction);
    
    return direction;
}

Vec3 Monster::avoidObstacles(const Vec3& direction) {
    Vec3 currentPos = getPosition();
    Vec3 forward = direction;
    Vec3 right = Vec3(-forward.z, 0.0f, forward.x); // Perpendicular to forward
    Vec3 left = Vec3(forward.z, 0.0f, -forward.x);  // Opposite of right
    
    // Check for obstacles in different directions
    float avoidanceDistance = 2.0f; // Distance to check for obstacles
    
    // Check forward
    Vec3 forwardCheck = currentPos + forward * avoidanceDistance;
    if (isPathBlocked(currentPos, forwardCheck)) {
        // Try right
        Vec3 rightCheck = currentPos + right * avoidanceDistance;
        if (!isPathBlocked(currentPos, rightCheck)) {
            return right;
        }
        
        // Try left
        Vec3 leftCheck = currentPos + left * avoidanceDistance;
        if (!isPathBlocked(currentPos, leftCheck)) {
            return left;
        }
        
        // Try backward
        Vec3 backwardCheck = currentPos - forward * avoidanceDistance;
        if (!isPathBlocked(currentPos, backwardCheck)) {
            return Vec3(-forward.x, -forward.y, -forward.z);
        }
        
        // If all directions blocked, use random direction
        return getRandomDirection();
    }
    
    return forward;
}

bool Monster::isPathBlocked(const Vec3& from, const Vec3& to) const {
    // Simple path blocking check - can be enhanced with actual collision detection
    // For now, just check if the path goes through certain areas
    
    // Check if path goes through origin (0,0,0) which might be blocked
    float distance = sqrt(
        (to.x - from.x) * (to.x - from.x) +
        (to.y - from.y) * (to.y - from.y) +
        (to.z - from.z) * (to.z - from.z)
    );
    
    if (distance < 0.1f) return true; // Too close
    
    // Simple obstacle check - can be enhanced with actual terrain collision
    // For now, just return false (no obstacles)
    return false;
}

Vec3 Monster::getRandomDirection() const {
    // Generate random direction
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    Vec3 direction;
    direction.x = cos(angle);
    direction.y = 0.0f; // Keep on same height
    direction.z = sin(angle);
    
    return direction;
}

void Monster::updateStuckDetection(const Vec3& oldPos, const Vec3& newPos, float deltaTime) {
    // This method is called from moveTowardsTarget
    // The actual stuck detection is handled in updatePathfinding
}

// Group behavior method implementations
void Monster::updateGroupBehavior(float deltaTime) {
    groupCoordinationTimer += deltaTime;
    
    // Update group coordination every few seconds
    if (groupCoordinationTimer >= 2.0f) {
        communicateWithNearbyMonsters();
        groupCoordinationTimer = 0.0f;
    }
    
    // Reset group alert flag after some time
    if (hasAlertedGroup && stateTimer > 10.0f) {
        hasAlertedGroup = false;
    }
}

void Monster::communicateWithNearbyMonsters() {
    if (isDead()) return;
    
    std::vector<Monster*> nearbyMonsters = getNearbyMonsters(groupAlertRadius);
    
    if (nearbyMonsters.empty()) {
        // No nearby monsters, leave group
        if (inGroup) {
            leaveGroup();
        }
        return;
    }
    
    // Join group if not already in one
    if (!inGroup) {
        joinGroup();
    }
    
    // Share information with group members
    for (Monster* monster : nearbyMonsters) {
        if (monster && monster != this && !monster->isDead()) {
            // Share player position if we can see the player
            if (hasLineOfSight && playerTarget) {
                monster->lastKnownPlayerPos = playerTarget->getPosition();
                monster->hasLineOfSight = true;
                
                // If we're chasing, alert the other monster
                if (state == MonsterState::Chasing || state == MonsterState::Attacking) {
                    monster->becomeAlert();
                }
            }
            
            // Share group target
            if (playerTarget) {
                monster->groupTarget = playerTarget->getPosition();
            }
        }
    }
}

void Monster::respondToGroupAlert() {
    if (isDead()) return;
    
    // If a nearby monster is alert, become alert too
    if (state == MonsterState::Patrolling || state == MonsterState::Idle) {
        becomeAlert();
    }
}

void Monster::coordinateAttack() {
    if (isDead() || !inGroup) return;
    
    std::vector<Monster*> nearbyMonsters = getNearbyMonsters(groupAlertRadius);
    
    // Coordinate attack timing with nearby monsters
    for (Monster* monster : nearbyMonsters) {
        if (monster && monster != this && !monster->isDead()) {
            // If we're attacking and they're not, encourage them to attack
            if (state == MonsterState::Attacking && monster->state != MonsterState::Attacking) {
                if (monster->isInAttackRange()) {
                    monster->setState(MonsterState::Attacking);
                }
            }
        }
    }
}

std::vector<Monster*> Monster::getNearbyMonsters(float radius) const {
    std::vector<Monster*> nearbyMonsters;
    
    // This would need access to the scene or monster spawner to get all monsters
    // For now, return empty vector - this would be implemented with proper scene access
    // In a real implementation, you'd query the scene for all monsters within radius
    
    return nearbyMonsters;
}

void Monster::alertNearbyMonsters() {
    if (isDead()) return;
    
    std::vector<Monster*> nearbyMonsters = getNearbyMonsters(groupAlertRadius);
    
    for (Monster* monster : nearbyMonsters) {
        if (monster && monster != this && !monster->isDead()) {
            // Alert nearby monsters to the threat
            monster->respondToGroupAlert();
            
            // Share player position
            if (playerTarget) {
                monster->lastKnownPlayerPos = playerTarget->getPosition();
                monster->groupTarget = playerTarget->getPosition();
            }
            
            // Increase their aggression
            monster->setAggressionLevel(monster->aggressionLevel + 0.1f);
        }
    }
    
    // std::cout << "Monster " << getName() << " alerted " << nearbyMonsters.size() << " nearby monsters!" << std::endl;
}

bool Monster::isInGroup() const {
    return inGroup;
}

void Monster::joinGroup() {
    if (!inGroup) {
        inGroup = true;
        // std::cout << "Monster " << getName() << " joined a group" << std::endl;
    }
}

void Monster::leaveGroup() {
    if (inGroup) {
        inGroup = false;
        // std::cout << "Monster " << getName() << " left the group" << std::endl;
    }
}

// Enhanced visual effect method implementations
void Monster::flashStateChange() {
    isStateFlashing = true;
    stateChangeFlashTimer = 0.3f; // Flash for 0.3 seconds
    // std::cout << "Monster " << getName() << " flashing state change for " << stateChangeFlashTimer << " seconds" << std::endl;
}

void Monster::updateStateVisualEffects(float deltaTime) {
    if (isStateFlashing) {
        stateChangeFlashTimer -= deltaTime;
        if (stateChangeFlashTimer <= 0.0f) {
            isStateFlashing = false;
            // std::cout << "Monster " << getName() << " state change flash ended" << std::endl;
        }
    }
}

void Monster::updatePulsingEffect(float deltaTime) {
    if (isPulsing) {
        pulseTimer += deltaTime;
    }
}

Vec3 Monster::getStateColor() const {
    switch (state) {
        case MonsterState::Alert:
            return alertColor;
        case MonsterState::Chasing:
            return chaseColor;
        case MonsterState::Attacking:
            return attackColor;
        case MonsterState::Stunned:
            return Vec3(0.5f, 0.5f, 0.5f); // Gray when stunned
        case MonsterState::Retreating:
            return Vec3(0.8f, 0.2f, 0.8f); // Purple when retreating
        default:
            return originalColor;
    }
}

void Monster::setPulsing(bool pulsing, float speed) {
    isPulsing = pulsing;
    pulseSpeed = speed;
    if (!pulsing) {
        pulseTimer = 0.0f;
    }
}

// Loot system method implementations
void Monster::dropLoot() {
    if (hasDroppedLoot) return;
    
    // std::cout << "=== MONSTER LOOT DROP ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    // std::cout << "Experience reward: " << experienceReward << " XP" << std::endl;
    // std::cout << "Score reward: " << scoreReward << " points" << std::endl;
    
    // In a real game, you would:
    // 1. Create loot objects at the monster's position
    // 2. Add experience to the player
    // 3. Add score to the game score
    // 4. Possibly drop items, weapons, or other rewards
    
    // For now, just log the rewards
    // std::cout << "Loot dropped at position: (" << getPosition().x << ", " << getPosition().y << ", " << getPosition().z << ")" << std::endl;
    // std::cout << "=========================" << std::endl;
}

int Monster::getExperienceReward() const {
    return experienceReward;
}

int Monster::getScoreReward() const {
    return scoreReward;
}

// Death animation method implementations
void Monster::startDeathAnimation() {
    if (isDeathAnimating) return;
    
    isDeathAnimating = true;
    deathAnimationTimer = 0.0f;
    
    // Store original scale
    originalScale = getScale();
    
    // std::cout << "=== DEATH ANIMATION STARTED ===" << std::endl;
    // std::cout << "Monster: " << getName() << std::endl;
    // std::cout << "Animation duration: " << deathAnimationDuration << " seconds" << std::endl;
    // std::cout << "Original scale: (" << originalScale.x << ", " << originalScale.y << ", " << originalScale.z << ")" << std::endl;
    // std::cout << "==============================" << std::endl;
}

void Monster::updateDeathAnimation(float deltaTime) {
    if (!isDeathAnimating) return;
    
    deathAnimationTimer += deltaTime;
    
    // Calculate animation progress (0.0 to 1.0)
    float progress = deathAnimationTimer / deathAnimationDuration;
    
    if (progress >= 1.0f) {
        // Animation finished
        isDeathAnimating = false;
        deathAnimationTimer = deathAnimationDuration;
        progress = 1.0f;
        
        // std::cout << "=== DEATH ANIMATION COMPLETED ===" << std::endl;
        // std::cout << "Monster: " << getName() << std::endl;
        // std::cout << "================================" << std::endl;
    }
    
    // Create shrinking effect (monster shrinks and fades)
    float scaleMultiplier = 1.0f - (progress * 0.8f); // Shrink to 20% of original size
    deathScale = originalScale * scaleMultiplier;
    
    // Apply the death scale
    setScale(deathScale);
    
    // Create fading effect by modifying color alpha (if supported)
    // For now, we'll just make the monster darker
    Vec3 fadeColor = getCurrentColor() * (1.0f - progress * 0.5f);
    setColor(fadeColor);
    
    // Debug output every few frames
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer > 0.5f) {
        // std::cout << "Death animation progress: " << (progress * 100.0f) << "%" << std::endl;
        // std::cout << "Current scale: (" << deathScale.x << ", " << deathScale.y << ", " << deathScale.z << ")" << std::endl;
        debugTimer = 0.0f;
    }
}


} // namespace Engine
