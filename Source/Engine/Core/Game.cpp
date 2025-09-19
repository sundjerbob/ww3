    /**
 * Game.cpp - Implementation of Main Game Engine Class
 * 
 * Complete implementation of the game engine root class.
 * Handles initialization, main loop, and coordination of all engine systems.
 */

#include "Game.h"
#include "../../GameObjects/Cube.h"
#include "../../GameObjects/Ground.h"
#include "../../GameObjects/SimpleChunkTerrainGround.h"
#include "../../GameObjects/Water.h"
#include "../Utils/TerrainGenerator.h"
#include "../Rendering/RendererFactory.h"
#include "../Rendering/WeaponRenderer.h"
#include "../Rendering/MonsterRenderer.h"
#include "../Rendering/LightingRenderer.h"
#include "../Rendering/WaterRenderer.h"
#include "../../GameObjects/Crosshair.h"
#include "../../GameObjects/Minimap.h"
#include "../../GameObjects/Arrow.h"
#include "../../GameObjects/Weapon.h"
#include "../../GameObjects/AmmoUI.h"
#include "../../GameObjects/Monster.h"
#include "../Input/Input.h"
#include <GL/glew.h>
#include <glfw3.h>
#include <iostream>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Engine {

Game::Game(int width, int height, const char* title)
    : window(nullptr), windowWidth(width), windowHeight(height), windowTitle(title),
      isRunning(false), isInitialized(false), deltaTime(0.0f), lastFrame(0.0f),
      isFullscreen(false), windowed_width(width), windowed_height(height),
      crosshair(nullptr) {}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    
    if (!initializeGLFW()) {
        return false;
    }
    
    if (!createWindow()) {
        return false;
    }
    
    if (!initializeGLEW()) {
        return false;
    }
    
    setupSystems();
    
    isInitialized = true;
    isRunning = true;
    
    printControls();
    
    return true;
}

bool Game::initializeGLFW() {
    if (!glfwInit()) {
        return false;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    return true;
}

bool Game::createWindow() {
    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    
    // Set window user pointer for callback access
    glfwSetWindowUserPointer(window, this);
    
    return true;
}

bool Game::initializeGLEW() {
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        return false;
    }
    
    
    return true;
}

void Game::setupSystems() {
    // Initialize camera
    camera = std::make_unique<Camera>();
    
    // Initialize renderer factory (creates and manages all renderers)
    if (!RendererFactory::getInstance().initialize(windowWidth, windowHeight)) {
        isRunning = false;
        return;
    }
    
    // Initialize scene
    scene = std::make_unique<Scene>("MainScene");
    if (!scene->initialize()) {
        isRunning = false;
        return;
    }
    
    // Initialize projectile manager (before scene objects so weapon can use it)
    projectileManager = std::make_unique<ProjectileManager>();
    projectileManager->initialize(nullptr, nullptr, nullptr); // No collision, particles, or audio for now
    
    // Setup scene objects
    setupSceneObjects();

    // Add crosshair object (will automatically use CrosshairRenderer via factory)
    auto crosshairPtr = std::make_unique<Crosshair>("Crosshair");
    if (!crosshairPtr->initialize()) {
    } else {
        // Store reference before moving
        crosshair = crosshairPtr.get();
        scene->addGameObject(std::move(crosshairPtr));
    }
    
    // Initialize input system
    Input& input = Input::getInstance();
    input.initialize(window, camera.get());
    input.setFullscreenToggleCallback([this]() { this->toggleFullscreen(); });
    
    // Set up window callbacks
    glfwSetFramebufferSizeCallback(window, Game::framebufferSizeCallback);
    glfwSetWindowIconifyCallback(window, Game::windowIconifyCallback);
}

void Game::run() {
    if (!isInitialized) {
        return;
    }
    
    // Main game loop
    while (isRunning && !glfwWindowShouldClose(window)) {
        calculateDeltaTime();
        
        // Process input
        glfwPollEvents();
        
        // Update game logic
        update(deltaTime);
        
        // Render frame
        render();
    }
}

void Game::update(float deltaTime) {
    // Process input
    Input& input = Input::getInstance();
    input.processInput(deltaTime);
    
    // Update scene
    if (scene) {
        scene->update(deltaTime);
        
        // Update chunk terrain based on player position
        auto groundObjects = scene->getAllGameObjects();
        for (auto* obj : groundObjects) {
            if (auto* chunkGround = dynamic_cast<SimpleChunkTerrainGround*>(obj)) {
                // Get player position from camera
                Vec3 playerPos = camera->getPosition();
                chunkGround->updateChunksForPlayer(playerPos);
                break;
            }
        }
    }
    
    // Update minimap
    if (minimap) {
        minimap->setPlayerPosition(camera->getPosition());
        minimap->update(deltaTime);
    }
    
    // Update weapon
    if (weapon) {
        weapon->update(deltaTime);
        
        // Update AmmoUI
        if (ammoUI) {
            ammoUI->update(deltaTime);
        }
        
        
        // Update camera recoil recovery
        if (camera) {
            camera->updateRecoil(deltaTime);
        }
        
        // Debug timer for periodic yaw output
        static float yawDebugTimer = 0.0f;
        yawDebugTimer += deltaTime;
        
        if (yawDebugTimer >= 1.0f) {
            // Print player yaw for debugging bullet trajectory issues
            if (camera) {
                Vec3 cameraRotation = camera->getRotation();
                std::cout << "=== PLAYER DEBUG INFO ===" << std::endl;
                std::cout << "Player Yaw: " << cameraRotation.y << " degrees" << std::endl;
                std::cout << "Player Pitch: " << cameraRotation.x << " degrees" << std::endl;
                std::cout << "Player Position: (" << camera->getPosition().x << ", " 
                          << camera->getPosition().y << ", " << camera->getPosition().z << ")" << std::endl;
                
                // Also print barrel tip position for comparison
                Vec3 barrelTip = weapon->getBarrelTipPosition();
                std::cout << "Barrel Tip: (" << barrelTip.x << ", " 
                          << barrelTip.y << ", " << barrelTip.z << ")" << std::endl;
                std::cout << "=========================" << std::endl;
            }
            
            // Reset timer
            yawDebugTimer = 0.0f;
        }
        
        // Update projectile manager and check for collisions
        if (projectileManager) {
            projectileManager->update(deltaTime);
            
            // Get all game objects for collision checking (including monsters)
            std::vector<GameObject*> allGameObjects;
            if (scene) {
                allGameObjects = scene->getAllObjectsForCollision(); // Use collision-safe method
            }
            
            // Check for projectile collisions with monsters
            projectileManager->checkAllCollisions(allGameObjects);
        }
        
        // Handle shooting with mouse buttons
        static bool leftMousePressed = false, rightMousePressed = false;
        
        // Left mouse button for firing
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && !leftMousePressed) {
            weapon->startFiring();
            leftMousePressed = true;
        } else if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && leftMousePressed) {
            weapon->stopFiring();
            leftMousePressed = false;
        }
        
        // Right mouse button for single shot (alternative firing)
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && !rightMousePressed) {
            weapon->fireSingleShot();
            rightMousePressed = true;
        } else if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            rightMousePressed = false;
        }
        
        // Middle mouse button for monster hunter shot
        static bool middleMousePressed = false;
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE) && !middleMousePressed) {
            weapon->fireMonsterHunterShot();
            middleMousePressed = true;
        } else if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
            middleMousePressed = false;
        }
        
        // R key for reload
        static bool reloadKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_R) && !reloadKeyPressed) {
            weapon->reload();
            reloadKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_R)) {
            reloadKeyPressed = false;
        }
        
        // H key for monster hunter shot
        static bool monsterHunterKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_H) && !monsterHunterKeyPressed) {
            weapon->fireMonsterHunterShot();
            monsterHunterKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_H)) {
            monsterHunterKeyPressed = false;
        }
        
        // Handle weapon switching with number keys 1-5
        static bool key1Pressed = false, key2Pressed = false, key3Pressed = false, key4Pressed = false, key5Pressed = false;
        
        if (input.isKeyPressed(GLFW_KEY_1) && !key1Pressed) {
            weapon->switchToWeapon(0); // Assault Rifle
            key1Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_1)) {
            key1Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_2) && !key2Pressed) {
            weapon->switchToWeapon(1); // Sniper Rifle
            key2Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_2)) {
            key2Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_3) && !key3Pressed) {
            weapon->switchToWeapon(2); // Submachine Gun
            key3Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_3)) {
            key3Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_4) && !key4Pressed) {
            weapon->switchToWeapon(3); // Pistol
            key4Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_4)) {
            key4Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_5) && !key5Pressed) {
            weapon->switchToWeapon(4); // Shotgun
            key5Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_5)) {
            key5Pressed = false;
        }
        
        // T key for terrain statistics
        static bool terrainStatsKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_T) && !terrainStatsKeyPressed) {
            terrainStatsKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_T)) {
            terrainStatsKeyPressed = false;
        }
        
        // W key for water statistics
        static bool waterStatsKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_W) && !waterStatsKeyPressed) {
            waterStatsKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_W)) {
            waterStatsKeyPressed = false;
        }
    }
    
    // Update monster spawner
    if (monsterSpawner) {
        monsterSpawner->update(deltaTime);
        
        // Debug monster spawner status
        static int spawnerDebugCount = 0;
        spawnerDebugCount++;
        if (spawnerDebugCount % 300 == 0) { // Print every 5 seconds
            std::cout << "=== MONSTER SPAWNER DEBUG ===" << std::endl;
            std::cout << "MonsterSpawner exists: YES" << std::endl;
            const auto& activeMonsters = monsterSpawner->getActiveMonsters();
            std::cout << "Active monsters count: " << activeMonsters.size() << std::endl;
            std::cout << "=============================" << std::endl;
        }
    } else {
        static int spawnerDebugCount = 0;
        spawnerDebugCount++;
        if (spawnerDebugCount % 300 == 0) { // Print every 5 seconds
            std::cout << "=== MONSTER SPAWNER DEBUG ===" << std::endl;
            std::cout << "MonsterSpawner exists: NO!" << std::endl;
            std::cout << "=============================" << std::endl;
        }
    }
}

void Game::render() {
    if (!scene) return;
    
    // Get the default renderer for frame control
    Renderer* defaultRenderer = RendererFactory::getInstance().getDefaultRenderer();
    if (!defaultRenderer) return;
    
    defaultRenderer->beginFrame();
    
    // Get water renderer for reflection/refraction passes
    WaterRenderer* waterRenderer = dynamic_cast<WaterRenderer*>(RendererFactory::getInstance().getRenderer(RendererType::Water));
    
    // Render reflection pass if water renderer is available
    if (waterRenderer) {
        waterRenderer->bindReflectionFramebuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render scene from reflection camera (flipped Y)
        // For now, just render normally - we'll enhance this later
        scene->render(*camera, *defaultRenderer);
        
        waterRenderer->unbindCurrentFramebuffer();
        
        // Render refraction pass
        waterRenderer->bindRefractionFramebuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render scene normally for refraction
        scene->render(*camera, *defaultRenderer);
        
        waterRenderer->unbindCurrentFramebuffer();
    }
    
    // Try to get LightingRenderer for shadow rendering
    LightingRenderer* lightingRenderer = dynamic_cast<LightingRenderer*>(defaultRenderer);
    if (lightingRenderer) {
        // Use shadow rendering if available
        std::vector<GameObject*> sceneObjects = scene->getAllGameObjects();
        lightingRenderer->renderSceneWithShadows(sceneObjects, *camera);
    } else {
        // Fall back to regular scene rendering
        scene->render(*camera, *defaultRenderer);
    }
    
    // Render water separately after the main scene
    // This ensures water is rendered on top of terrain with proper depth testing
    if (waterRenderer) {
        // Enable depth testing for water
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // Enable blending for water transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Get water GameObject from scene and render it
        GameObject* waterObj = scene->getGameObject("WaterSurface");
        if (waterObj && waterObj->getActive()) {
            waterObj->render(*waterRenderer, *camera);
        }
        
        // Disable blending after water rendering
        glDisable(GL_BLEND);
    }
    
    // Render minimap (UI overlay)
    if (minimap) {
        // Update minimap with current player position and camera direction
        minimap->setPlayerPosition(camera->getPosition());
        minimap->setPlayerDirectionFromYaw(camera->getYaw() * 180.0f / 3.14159f); // Convert radians to degrees
        minimap->render(*defaultRenderer, *camera);
    }
    
    // Render weapon (FPS-style weapon overlay)
    if (weapon) {
        // Get the weapon-specific renderer
        Renderer* weaponRenderer = RendererFactory::getInstance().getRenderer(RendererType::Weapon);
        if (weaponRenderer) {
            weapon->render(*weaponRenderer, *camera);
        } else {
            // Fallback to default renderer
            weapon->render(*defaultRenderer, *camera);
        }
    }
    
    // DEBUG: Render projectile start position marker
    if (weapon) {
        renderProjectileStartPositionDebug(*defaultRenderer, *camera);
    } else {
        // Debug: Check if weapon is available
        static int weaponDebugCounter = 0;
        weaponDebugCounter++;
        if (weaponDebugCounter % 300 == 0) { // Every 5 seconds
            std::cout << "=== WEAPON DEBUG ===" << std::endl;
            std::cout << "Weapon is NULL - debug sphere not rendered!" << std::endl;
            std::cout << "===================" << std::endl;
        }
    }
    
    // Render monsters with MonsterRenderer for multi-material support
    if (monsterSpawner) {
        Renderer* monsterRenderer = RendererFactory::getInstance().getRenderer(RendererType::Monster);
        if (monsterRenderer) {
            const auto& activeMonsters = monsterSpawner->getActiveMonsters();
            // Debug rendering every few seconds to avoid spam
            static float renderDebugTimer = 0.0f;
            renderDebugTimer += 0.016f; // Approximate frame time
            if (renderDebugTimer > 3.0f) {
                // std::cout << "=== MONSTER RENDERING ===" << std::endl;
                // std::cout << "MonsterRenderer available: YES" << std::endl;
                // std::cout << "Active monsters count: " << activeMonsters.size() << std::endl;
                renderDebugTimer = 0.0f;
            }
            
            // First, render all monsters
            for (const auto& monster : activeMonsters) {
                if (monster && monster->isAlive() && monster->getActive()) {
                    monster->render(*monsterRenderer, *camera);
                }
            }
        } else {
            // std::cout << "=== MONSTER RENDERING ===" << std::endl;
            // std::cout << "MonsterRenderer NOT AVAILABLE!" << std::endl;
            // std::cout << "========================" << std::endl;
        }
        
    }
    
    // Render projectiles
    if (projectileManager) {
        projectileManager->render(*defaultRenderer, *camera);
    }
    
    // Render health bars - AFTER EVERYTHING ELSE for maximum visibility
    if (monsterSpawner) {
        const auto& activeMonsters = monsterSpawner->getActiveMonsters();
        static int healthBarDebugCount = 0;
        healthBarDebugCount++;
        std::cout << "=== HEALTH BAR RENDERING DEBUG ===" << std::endl;
        if (healthBarDebugCount % 300 == 0) { // Print every 5 seconds
            std::cout << "Active monsters count: " << activeMonsters.size() << std::endl;
            for (size_t i = 0; i < activeMonsters.size(); ++i) {
                if (activeMonsters[i]) {
                    std::cout << "Monster " << i << ": alive=" << activeMonsters[i]->isAlive() 
                              << ", active=" << activeMonsters[i]->getActive() << std::endl;
                }
            }
            std::cout << "================================" << std::endl;
        }
        
        for (const auto& monster : activeMonsters) {
            if (monster && monster->getActive() && monster->isAlive()) {
                // Only render health bars for alive monsters
                monster->renderHealthBar(*camera);
            }
        }
    }
    
    // Render AmmoUI (UI overlay)
    if (ammoUI) {
        // Get the text renderer for UI elements
        Renderer* textRenderer = RendererFactory::getInstance().getRenderer(RendererType::Text);
        if (textRenderer) {
            ammoUI->render(*textRenderer, *camera);
        } else {
            // Fallback to crosshair renderer
            Renderer* uiRenderer = RendererFactory::getInstance().getRenderer(RendererType::Crosshair);
            if (uiRenderer) {
                ammoUI->render(*uiRenderer, *camera);
            } else {
                // Final fallback to default renderer
                ammoUI->render(*defaultRenderer, *camera);
            }
        }
    }
    
    
    defaultRenderer->endFrame(window);
}

void Game::calculateDeltaTime() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Game::cleanup() {
    
    // Clean up systems
    weapon.reset();
    minimap.reset();
    ammoUI.reset();
    monsterSpawner.reset();
    scene.reset();
    camera.reset();
    Input::cleanup();
    
    // Clean up renderer factory
    RendererFactory::getInstance().cleanup();
    
    
    // Clean up GLFW
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
    
    isInitialized = false;
    isRunning = false;
    
}

void Game::setupSceneObjects() {
    if (!scene) return;
    
    
    // Add simple terrain ground (much faster and simpler)
    auto simpleGround = std::make_unique<SimpleChunkTerrainGround>("SimpleChunkTerrain", 200.0f, Vec3(0.4f, 0.3f, 0.2f));
    
    // Configure simple chunk terrain parameters for natural terrain with Perlin noise
    SimpleChunkTerrainParams terrainParams;
    terrainParams.baseHeight = -8.0f;         // Higher base height for brighter terrain
    terrainParams.amplitude = 8.0f;           // Reduced amplitude to cap height lower
    terrainParams.frequency = 0.15f;          // Higher frequency for more detailed terrain
    terrainParams.octaves = 4;                // 4 octaves for natural fractal noise
    terrainParams.persistence = 0.5;          // How much each octave contributes
    terrainParams.lacunarity = 2.0;           // How frequency changes between octaves
    terrainParams.seed = 12345;               // Random seed for terrain generation
    terrainParams.chunkSize = 16;             // 16x16 world units per chunk
    terrainParams.chunkResolution = 32;       // 32x32 vertices per chunk
            
    simpleGround->setTerrainParams(terrainParams);
    simpleGround->setRenderDistance(8);
    
    if (!simpleGround->initialize()) {
    } else {
    }
    
    // Store ground reference for entity system
    Ground* groundPtr = simpleGround.get();
    scene->addGameObject(std::move(simpleGround));
    scene->setGroundReference(groundPtr);
    
    // Add water surface
    auto waterSurface = std::make_unique<Water>("WaterSurface", -10.0f); // Water at terrain base level (-10.0f)
    waterSurface->setPosition(Vec3(0.0f, 0.0f, 0.0f)); // Position at origin, height handled by shader
    waterSurface->setScale(Vec3(1.0f, 1.0f, 1.0f)); // No scaling needed
    
    // Configure water parameters
    waterSurface->setWaveSpeed(0.05f);        // Faster waves
    waterSurface->setDistortionScale(0.02f);  // More distortion
    waterSurface->setShineDamper(15.0f);      // Softer shine
    waterSurface->setReflectivity(0.7f);      // More reflective
    
    if (!waterSurface->initialize()) {
    } else {
    }
    
    scene->addGameObject(std::move(waterSurface));
    
    // Add rotating cube at center
    auto centerCube = std::make_unique<Cube>("CenterCube", Vec3(1.0f, 0.5f, 0.0f));
    centerCube->setPosition(Vec3(0.0f, 0.0f, 0.0f));
    centerCube->setRotating(true);
    centerCube->setRotationSpeed(90.0f);
    scene->addGameObject(std::move(centerCube));
    
    // Add static cubes around the scene
    auto redCube = std::make_unique<Cube>("RedCube", Vec3(1.0f, 0.0f, 0.0f));
    redCube->setPosition(Vec3(5.0f, 0.0f, 3.0f));
    scene->addGameObject(std::move(redCube));
    
    auto greenCube = std::make_unique<Cube>("GreenCube", Vec3(0.0f, 1.0f, 0.0f));
    greenCube->setPosition(Vec3(-3.0f, 0.0f, 7.0f));
    scene->addGameObject(std::move(greenCube));
    
    auto blueCube = std::make_unique<Cube>("BlueCube", Vec3(0.0f, 0.0f, 1.0f));
    blueCube->setPosition(Vec3(8.0f, 0.0f, -2.0f));
    scene->addGameObject(std::move(blueCube));
    
    auto magentaCube = std::make_unique<Cube>("MagentaCube", Vec3(1.0f, 0.0f, 1.0f));
    magentaCube->setPosition(Vec3(-6.0f, 0.0f, -4.0f));
    scene->addGameObject(std::move(magentaCube));
    
    // Add minimap (UI element, not part of 3D scene)
    minimap = std::make_unique<Minimap>("Minimap", 0.25f);
    minimap->setScene(scene.get());
    
    // Assign renderer to minimap (needed for child objects like arrow)
    Renderer* defaultRenderer = RendererFactory::getInstance().getDefaultRenderer();
    if (defaultRenderer) {
        minimap->setRenderer(defaultRenderer);
    } else {
    }
    
    // Store minimap reference for ground reference setup
    Minimap* minimapPtr = minimap.get();
    
    // Set ground reference in minimap for entity visibility checking
    minimapPtr->setGroundReference(groundPtr);
    
    // Set minimap reference in ground for chunk change notifications
    groundPtr->setMinimapReference(minimapPtr);
    
    // Store minimap reference for camera direction updates
    // Note: minimapPtr is already owned by the unique_ptr, so we don't need to store it separately
    
    // Configure minimap dimensions and scope
    minimap->setMinimapDimensions(512, 512);  // Higher resolution for better quality
    minimap->setScopeSize(12.0f);  // Smaller scope for larger entities (was 24.0f)
    minimap->setOrthographicScope(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);  // This is now overridden by dynamic scope

    if (!minimap->initialize()) {
    }
    
    // Create weapon (FPS-style weapon rendering)
    weapon = std::make_unique<Weapon>("PlayerWeapon", 
                                     "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/AssaultRifle_01.obj",
                                     Vec3(0.8f, 0.8f, 0.8f));  // Light metallic color for better visibility
    
    
    // Configure weapon properties - restore to original working position
    weapon->setWeaponScale(0.3f);  // Original scale
    weapon->setScreenPosition(Vec3(0.4f, -0.4f, 0.0f));  // Original bottom-right position
    weapon->setWeaponOffset(Vec3(-0.05f, 0.1f, 0.0f));  // Original offset
    weapon->setAimSensitivity(0.5f);  // Reduce aim sensitivity
    weapon->setDefaultRotation(Vec3(0.0f, 0.0f, 0.0f));   // Face -Z (into screen)
    weapon->setPlayerCamera(camera.get());
    
    if (!weapon->initialize()) {
    } else {
    }
    
    // Configure weapon with recoil settings
    WeaponStats weaponStats;
    weaponStats.fireRate = 8.0f;  // 8 shots per second (faster firing)
    weaponStats.recoil = 1.0f;    // Much stronger recoil force for visibility
    weaponStats.spread = 0.02f;   // Small spread increase per shot
    weaponStats.maxAmmo = 30;
    weaponStats.currentAmmo = 30;
    weaponStats.maxReserveAmmo = 90;
    weaponStats.currentReserveAmmo = 90;
    weaponStats.reloadTime = 2.0f;
    weaponStats.infiniteAmmo = true; // Enable infinite ammo for testing recoil
    weaponStats.projectileType = ProjectileType::Bullet;
    weaponStats.projectileConfig.speed = 50.0f;
    weaponStats.projectileConfig.lifetime = 5.0f;
    weaponStats.projectileConfig.damage = 25.0f;
    
    weapon->configureShooting(weaponStats);
    
    // Set up projectile manager for weapon shooting system
    if (projectileManager && weapon) {
        weapon->setProjectileManager(projectileManager.get());
    }
    
    // Connect crosshair and camera to weapon recoil system
    if (crosshair && weapon) {
        // Set up weapon recoil callback (triggers when weapon recoil is applied)
        weapon->setRecoilCallback([this](const Vec3& recoil) {
            // Apply recoil to crosshair
            if (this->crosshair) {
                this->crosshair->applyRecoil(recoil);
            }
            
            // Apply recoil to camera using new recoil system
            if (this->camera) {
                this->camera->applyRecoil(recoil);
            }
        });
        
        // Also set up shooting system recoil callback (triggers when shots are fired)
        WeaponShootingComponent& shootingComponent = weapon->getShootingComponent();
        ShootingSystem* shootingSystem = shootingComponent.getShootingSystem();
        if (shootingSystem) {
            shootingSystem->setRecoilCallback([this](const Vec3& recoil) {
                // Apply recoil to crosshair
                if (this->crosshair) {
                    this->crosshair->applyRecoil(recoil);
                }
                
                // Apply recoil to camera using new recoil system
                if (this->camera) {
                    this->camera->applyRecoil(recoil);
                }
            });
        }
    }
    
    // Create and initialize AmmoUI
    ammoUI = std::make_unique<AmmoUI>("AmmoUI");
    
    // Configure AmmoUI properties
    ammoUI->setScreenPosition(Vec2(0.85f, -0.85f));  // Bottom-right corner
    ammoUI->setSize(Vec2(0.25f, 0.15f));             // UI size
    ammoUI->setTextColor(Vec3(1.0f, 1.0f, 1.0f));   // White text
    ammoUI->setBackgroundColor(Vec3(0.0f, 0.0f, 0.0f)); // Black background
    ammoUI->setLowAmmoColor(Vec3(1.0f, 0.3f, 0.3f));    // Red for low ammo
    ammoUI->setReloadColor(Vec3(1.0f, 1.0f, 0.0f));     // Yellow for reloading
    ammoUI->setLowAmmoThreshold(0.25f);                  // 25% threshold
    
    
    // Connect AmmoUI to weapon
    if (weapon) {
        ammoUI->setWeapon(weapon.get());
    }
    
    // Initialize AmmoUI
    if (!ammoUI->initialize()) {
    } else {
    }
    
    
    // Initialize monster spawner
    monsterSpawner = std::make_unique<MonsterSpawner>(scene.get(), weapon.get());
    
    
    // TESTING: Directly spawn 3 monsters for health bar testing
    if (monsterSpawner) {
        std::cout << "=== DIRECT SPAWN 3 MONSTERS FOR TESTING ===" << std::endl;
        monsterSpawner->spawnMonsterAt(Vec3(8.0f, 0.0f, 8.0f), MonsterType::Xenomorph);   // Monster 1
        monsterSpawner->spawnMonsterAt(Vec3(12.0f, 0.0f, 10.0f), MonsterType::Xenomorph); // Monster 2  
        monsterSpawner->spawnMonsterAt(Vec3(10.0f, 0.0f, 14.0f), MonsterType::Xenomorph); // Monster 3
        std::cout << "Direct spawned 3 monsters for health bar testing" << std::endl;
        std::cout << "Active monsters count: " << monsterSpawner->getActiveMonsters().size() << std::endl;
        std::cout << "===============================================" << std::endl;
    }
    
    // TEMPORARILY DISABLED: Adding weapon to scene has ownership issues
    // std::cout << "=== ADDING WEAPON TO SCENE FOR TESTING ===" << std::endl;
    // scene->addGameObject(weapon.get());
    // std::cout << "=== WEAPON ADDED TO SCENE ===" << std::endl;
    
    scene->printSceneInfo();
}

void Game::toggleFullscreen() {
    if (!window) return;
    
    if (isFullscreen) {
        // Switch to windowed mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, nullptr, 100, 100, windowed_width, windowed_height, mode->refreshRate);
        isFullscreen = false;
        
        // Update dimensions after switching to windowed mode
        windowWidth = windowed_width;
        windowHeight = windowed_height;
    } else {
        // Switch to fullscreen mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        isFullscreen = true;
        
        // Update dimensions after switching to fullscreen mode
        windowWidth = mode->width;
        windowHeight = mode->height;
    }
    
    // Update all renderers with new dimensions
    RendererFactory::getInstance().setViewport(windowWidth, windowHeight);
    
}

void Game::printControls() {
    // Controls information removed to eliminate console output
}

void Game::onWindowResize(int width, int height) {
    // Update window dimensions
    windowWidth = width;
    windowHeight = height;
    
    // Update all renderers with new dimensions
    RendererFactory::getInstance().setViewport(width, height);
    
    
    // Additional debugging information
    Renderer* defaultRenderer = RendererFactory::getInstance().getDefaultRenderer();
    if (defaultRenderer) {
    }
}

void Game::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Get the Game instance from the window user pointer
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        game->onWindowResize(width, height);
    }
}

void Game::windowIconifyCallback(GLFWwindow* window, int iconified) {
    // Get the Game instance from the window user pointer
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) {
        if (iconified) {
        } else {
            // Get current window size and update
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            game->onWindowResize(width, height);
        }
    }
}

void Game::renderProjectileStartPositionDebug(const Renderer& renderer, const Camera& camera) {
    if (!weapon) {
        std::cout << "DEBUG: Weapon is NULL in renderProjectileStartPositionDebug!" << std::endl;
        return;
    }
    
    // SCREEN-SPACE APPROACH: Render a fixed point on screen
    // This simulates where the projectile would start from the player's perspective
    
    // Get the projectile start position from the weapon (for debug output)
    Vec3 startPos = weapon->getBarrelTipPosition();
    
    // Debug: Always print when this method is called
    static int methodCallCounter = 0;
    methodCallCounter++;
    if (methodCallCounter % 60 == 0) { // Every 1 second
        std::cout << "=== SCREEN-SPACE DEBUG MARKER ===" << std::endl;
        std::cout << "Method called " << methodCallCounter << " times" << std::endl;
        std::cout << "Weapon world position: (" << startPos.x << ", " << startPos.y << ", " << startPos.z << ")" << std::endl;
        std::cout << "Camera position: (" << camera.getPosition().x << ", " << camera.getPosition().y << ", " << camera.getPosition().z << ")" << std::endl;
        std::cout << "=================================" << std::endl;
    }
    
    // Create a simple debug quad for screen-space rendering
    static std::unique_ptr<Mesh> debugQuad = nullptr;
    if (!debugQuad) {
        debugQuad = std::make_unique<Mesh>();
        
        // Create a simple quad (2 triangles) for screen-space rendering
        std::vector<float> vertices = {
            // Position (x, y, z) - small quad in front of camera
            -0.05f, -0.05f, 0.0f,  // Bottom-left
             0.05f, -0.05f, 0.0f,  // Bottom-right
             0.05f,  0.05f, 0.0f,  // Top-right
            -0.05f,  0.05f, 0.0f   // Top-left
        };
        
        std::vector<unsigned int> indices = {
            0, 1, 2,  // First triangle
            2, 3, 0   // Second triangle
        };
        
        debugQuad->createMesh(vertices, indices);
        
        std::cout << "=== DEBUG QUAD CREATED ===" << std::endl;
        std::cout << "Quad mesh valid: " << (debugQuad->isValid() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << "=========================" << std::endl;
    }
    
    // Create model matrix for screen-space positioning with camera-facing basis
    Mat4 modelMatrix = Mat4();
    
    // Build an orthonormal basis from the camera so the quad doesn't rotate relative to the screen
    Vec3 cameraPos = camera.getPosition();
    Vec3 cameraForward = camera.getForward().normalize();
    Vec3 cameraRight = camera.getRight().normalize();
    Vec3 cameraUp = camera.getUpVector().normalize();
    
    // Scale the quad to a small size in world units
    const float quadScale = 0.1f;
    
    // Set rotation/scale rows (right, up, forward) — billboard locked to camera orientation
    modelMatrix.m[0]  = cameraRight.x * quadScale;  modelMatrix.m[1]  = cameraRight.y * quadScale;  modelMatrix.m[2]  = cameraRight.z * quadScale;  modelMatrix.m[3]  = 0.0f;
    modelMatrix.m[4]  = cameraUp.x    * quadScale;  modelMatrix.m[5]  = cameraUp.y    * quadScale;  modelMatrix.m[6]  = cameraUp.z    * quadScale;  modelMatrix.m[7]  = 0.0f;
    modelMatrix.m[8]  = cameraForward.x;            modelMatrix.m[9]  = cameraForward.y;            modelMatrix.m[10] = cameraForward.z;            modelMatrix.m[11] = 0.0f;
    
    // Position the quad at a fixed offset from camera (like a muzzle point in view space)
    Vec3 screenOffset = Vec3(0.2f, -0.1f, 0.8f); // Right, down, forward from camera
    Vec3 quadWorldPos = cameraPos + 
                       cameraRight * screenOffset.x +
                       cameraUp    * screenOffset.y +
                       cameraForward * screenOffset.z;
    
    modelMatrix.m[12] = quadWorldPos.x;
    modelMatrix.m[13] = quadWorldPos.y;
    modelMatrix.m[14] = quadWorldPos.z;
    modelMatrix.m[15] = 1.0f;
    
    // Render the debug quad in bright red
    Vec3 debugColor(1.0f, 0.0f, 0.0f); // Bright red
    
    // Debug: Print rendering info
    static int renderCounter = 0;
    renderCounter++;
    if (renderCounter % 60 == 0) { // Every 1 second
        std::cout << "=== RENDERING SCREEN-SPACE QUAD ===" << std::endl;
        std::cout << "Render call #" << renderCounter << std::endl;
        std::cout << "Quad world position: (" << quadWorldPos.x << ", " << quadWorldPos.y << ", " << quadWorldPos.z << ")" << std::endl;
        std::cout << "Screen offset: (" << screenOffset.x << ", " << screenOffset.y << ", " << screenOffset.z << ")" << std::endl;
        std::cout << "===================================" << std::endl;
    }
    
    renderer.renderMesh(*debugQuad, modelMatrix, camera, debugColor);
    
    // SECOND MARKER: End position (center of screen / crosshair position)
    // This represents where the player is aiming - the end of the projectile vector
    
    // Create a second debug quad for the end position
    static std::unique_ptr<Mesh> debugEndQuad = nullptr;
    if (!debugEndQuad) {
        debugEndQuad = std::make_unique<Mesh>();
        
        // Create a simple quad for the end position marker
        std::vector<float> endVertices = {
            // Position (x, y, z) - small quad at center of screen
            -0.03f, -0.03f, 0.0f,  // Bottom-left
             0.03f, -0.03f, 0.0f,  // Bottom-right
             0.03f,  0.03f, 0.0f,  // Top-right
            -0.03f,  0.03f, 0.0f   // Top-left
        };
        
        std::vector<unsigned int> endIndices = {
            0, 1, 2,  // First triangle
            2, 3, 0   // Second triangle
        };
        
        debugEndQuad->createMesh(endVertices, endIndices);
        
        std::cout << "=== DEBUG END QUAD CREATED ===" << std::endl;
        std::cout << "End quad mesh valid: " << (debugEndQuad->isValid() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << "=============================" << std::endl;
    }
    
    // Create model matrix for the end position (center of screen)
    Mat4 endModelMatrix = Mat4();
    
    // Position the end quad at the center of the screen (where crosshair is)
    // This is a fixed distance in front of the camera, centered
    Vec3 endScreenOffset = Vec3(0.0f, 0.0f, 1.0f); // Center, center, forward from camera
    Vec3 endWorldPos = cameraPos + 
                      cameraRight * endScreenOffset.x +
                      cameraUp    * endScreenOffset.y +
                      cameraForward * endScreenOffset.z;
    
    // Set rotation/scale rows (right, up, forward) — billboard locked to camera orientation
    const float endQuadScale = 0.08f; // Slightly smaller than start quad
    endModelMatrix.m[0]  = cameraRight.x * endQuadScale;  endModelMatrix.m[1]  = cameraRight.y * endQuadScale;  endModelMatrix.m[2]  = cameraRight.z * endQuadScale;  endModelMatrix.m[3]  = 0.0f;
    endModelMatrix.m[4]  = cameraUp.x    * endQuadScale;  endModelMatrix.m[5]  = cameraUp.y    * endQuadScale;  endModelMatrix.m[6]  = cameraUp.z    * endQuadScale;  endModelMatrix.m[7]  = 0.0f;
    endModelMatrix.m[8]  = cameraForward.x;               endModelMatrix.m[9]  = cameraForward.y;               endModelMatrix.m[10] = cameraForward.z;               endModelMatrix.m[11] = 0.0f;
    
    // Set position in model matrix
    endModelMatrix.m[12] = endWorldPos.x;
    endModelMatrix.m[13] = endWorldPos.y;
    endModelMatrix.m[14] = endWorldPos.z;
    endModelMatrix.m[15] = 1.0f;
    
    // Render the debug end quad in bright green
    Vec3 endDebugColor(0.0f, 1.0f, 0.0f); // Bright green
    
    // Debug: Print rendering info for end position
    if (renderCounter % 60 == 0) { // Every 1 second
        std::cout << "End quad world position: (" << endWorldPos.x << ", " << endWorldPos.y << ", " << endWorldPos.z << ")" << std::endl;
        std::cout << "End screen offset: (" << endScreenOffset.x << ", " << endScreenOffset.y << ", " << endScreenOffset.z << ")" << std::endl;
    }
    
    renderer.renderMesh(*debugEndQuad, endModelMatrix, camera, endDebugColor);
}

} // namespace Engine