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
#include <iostream>

namespace Engine {

Game::Game(int width, int height, const char* title)
    : window(nullptr), windowWidth(width), windowHeight(height), windowTitle(title),
      isRunning(false), isInitialized(false), deltaTime(0.0f), lastFrame(0.0f),
      isFullscreen(false), windowed_width(width), windowed_height(height) {}

Game::~Game() {
    cleanup();
}

bool Game::initialize() {
    std::cout << "Initializing Game Engine..." << std::endl;
    
    if (!initializeGLFW()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    if (!createWindow()) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    
    if (!initializeGLEW()) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    setupSystems();
    
    isInitialized = true;
    isRunning = true;
    
    printControls();
    
    std::cout << "Game Engine initialized successfully!" << std::endl;
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
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
    
    return true;
}

void Game::setupSystems() {
    // Initialize camera
    camera = std::make_unique<Camera>();
    
    // Initialize renderer factory (creates and manages all renderers)
    if (!RendererFactory::getInstance().initialize(windowWidth, windowHeight)) {
        std::cerr << "Failed to initialize renderer factory" << std::endl;
        isRunning = false;
        return;
    }
    
    // Initialize scene
    scene = std::make_unique<Scene>("MainScene");
    if (!scene->initialize()) {
        std::cerr << "Failed to initialize scene" << std::endl;
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
        std::cerr << "Failed to initialize crosshair" << std::endl;
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
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetWindowIconifyCallback(window, windowIconifyCallback);
}

void Game::run() {
    if (!isInitialized) {
        std::cerr << "Game not initialized! Call initialize() first." << std::endl;
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
        
        // Handle shooting with mouse buttons
        static bool leftMousePressed = false, rightMousePressed = false;
        
        // Left mouse button for firing
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && !leftMousePressed) {
            std::cout << "=== STARTING TO FIRE ===" << std::endl;
            weapon->startFiring();
            leftMousePressed = true;
        } else if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) && leftMousePressed) {
            std::cout << "=== STOPPING FIRE ===" << std::endl;
            weapon->stopFiring();
            leftMousePressed = false;
        }
        
        // Right mouse button for single shot (alternative firing)
        if (input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT) && !rightMousePressed) {
            std::cout << "=== FIRING SINGLE SHOT ===" << std::endl;
            weapon->fireSingleShot();
            rightMousePressed = true;
        } else if (!input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            rightMousePressed = false;
        }
        
        // R key for reload
        static bool reloadKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_R) && !reloadKeyPressed) {
            std::cout << "=== RELOADING WEAPON ===" << std::endl;
            weapon->reload();
            reloadKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_R)) {
            reloadKeyPressed = false;
        }
        
        // Handle weapon switching with number keys 1-5
        static bool key1Pressed = false, key2Pressed = false, key3Pressed = false, key4Pressed = false, key5Pressed = false;
        
        if (input.isKeyPressed(GLFW_KEY_1) && !key1Pressed) {
            weapon->switchToWeapon(0); // Assault Rifle
            std::cout << "=== WEAPON SWITCHED ===" << std::endl;
            std::cout << "Current Weapon: " << weapon->getCurrentWeaponName() << " (Slot 1)" << std::endl;
            std::cout << "=======================" << std::endl;
            key1Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_1)) {
            key1Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_2) && !key2Pressed) {
            weapon->switchToWeapon(1); // Sniper Rifle
            std::cout << "=== WEAPON SWITCHED ===" << std::endl;
            std::cout << "Current Weapon: " << weapon->getCurrentWeaponName() << " (Slot 2)" << std::endl;
            std::cout << "=======================" << std::endl;
            key2Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_2)) {
            key2Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_3) && !key3Pressed) {
            weapon->switchToWeapon(2); // Submachine Gun
            std::cout << "=== WEAPON SWITCHED ===" << std::endl;
            std::cout << "Current Weapon: " << weapon->getCurrentWeaponName() << " (Slot 3)" << std::endl;
            std::cout << "=======================" << std::endl;
            key3Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_3)) {
            key3Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_4) && !key4Pressed) {
            weapon->switchToWeapon(3); // Pistol
            std::cout << "=== WEAPON SWITCHED ===" << std::endl;
            std::cout << "Current Weapon: " << weapon->getCurrentWeaponName() << " (Slot 4)" << std::endl;
            std::cout << "=======================" << std::endl;
            key4Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_4)) {
            key4Pressed = false;
        }
        
        if (input.isKeyPressed(GLFW_KEY_5) && !key5Pressed) {
            weapon->switchToWeapon(4); // Shotgun
            std::cout << "=== WEAPON SWITCHED ===" << std::endl;
            std::cout << "Current Weapon: " << weapon->getCurrentWeaponName() << " (Slot 5)" << std::endl;
            std::cout << "=======================" << std::endl;
            key5Pressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_5)) {
            key5Pressed = false;
        }
        
        // T key for terrain statistics
        static bool terrainStatsKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_T) && !terrainStatsKeyPressed) {
            std::cout << "=== TERRAIN STATISTICS ===" << std::endl;
            auto groundObjects = scene->getAllGameObjects();
                            for (auto* obj : groundObjects) {
                    if (auto* simpleGround = dynamic_cast<SimpleChunkTerrainGround*>(obj)) {
                        const auto& params = simpleGround->getTerrainParams();
                        std::cout << "Simple Chunk Terrain Parameters (Perlin Noise):" << std::endl;
                        std::cout << "  Base Height: " << params.baseHeight << std::endl;
                        std::cout << "  Amplitude: " << params.amplitude << std::endl;
                        std::cout << "  Frequency: " << params.frequency << std::endl;
                        std::cout << "  Octaves: " << params.octaves << std::endl;
                        std::cout << "  Persistence: " << params.persistence << std::endl;
                        std::cout << "  Lacunarity: " << params.lacunarity << std::endl;
                        std::cout << "  Seed: " << params.seed << std::endl;
                        std::cout << "  Chunk Size: " << params.chunkSize << std::endl;
                        std::cout << "  Chunk Resolution: " << params.chunkResolution << std::endl;
                        std::cout << "  Render Distance: " << simpleGround->getRenderDistance() << std::endl;
                        std::cout << "  Loaded Chunks: " << simpleGround->getLoadedChunkCount() << std::endl;
                    }
                }
            std::cout << "=========================" << std::endl;
            terrainStatsKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_T)) {
            terrainStatsKeyPressed = false;
        }
        
        // W key for water statistics
        static bool waterStatsKeyPressed = false;
        if (input.isKeyPressed(GLFW_KEY_W) && !waterStatsKeyPressed) {
            std::cout << "=== WATER STATISTICS ===" << std::endl;
            auto sceneObjects = scene->getAllGameObjects();
            for (auto* obj : sceneObjects) {
                if (auto* water = dynamic_cast<Water*>(obj)) {
                    std::cout << "Water Surface Parameters:" << std::endl;
                    std::cout << "  Water Height: " << water->getWaterHeight() << std::endl;
                    std::cout << "  Wave Speed: " << water->getWaveSpeed() << std::endl;
                    std::cout << "  Distortion Scale: " << water->getDistortionScale() << std::endl;
                    std::cout << "  Shine Damper: " << water->getShineDamper() << std::endl;
                    std::cout << "  Reflectivity: " << water->getReflectivity() << std::endl;
                    std::cout << "  Position: (" << water->getPosition().x << ", " 
                              << water->getPosition().y << ", " << water->getPosition().z << ")" << std::endl;
                    std::cout << "  Scale: (" << water->getScale().x << ", " 
                              << water->getScale().y << ", " << water->getScale().z << ")" << std::endl;
                }
            }
            std::cout << "=======================" << std::endl;
            waterStatsKeyPressed = true;
        } else if (!input.isKeyPressed(GLFW_KEY_W)) {
            waterStatsKeyPressed = false;
        }
    }
    
    // Update monster spawner
    if (monsterSpawner) {
        monsterSpawner->update(deltaTime);
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
    
    // Render monsters with MonsterRenderer for multi-material support
    if (monsterSpawner) {
        Renderer* monsterRenderer = RendererFactory::getInstance().getRenderer(RendererType::Monster);
        if (monsterRenderer) {
            const auto& activeMonsters = monsterSpawner->getActiveMonsters();
            for (const auto& monster : activeMonsters) {
                if (monster && monster->isAlive() && monster->getActive()) {
                    monster->render(*monsterRenderer, *camera);
                }
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
    std::cout << "Cleaning up Game Engine..." << std::endl;
    
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
    
    std::cout << "Game Engine cleanup complete." << std::endl;
}

void Game::setupSceneObjects() {
    if (!scene) return;
    
    std::cout << "Setting up scene objects..." << std::endl;
    
    // Add simple terrain ground (much faster and simpler)
    std::cout << "Creating SimpleChunkTerrainGround..." << std::endl;
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
    
    std::cout << "Setting terrain parameters..." << std::endl;
    simpleGround->setTerrainParams(terrainParams);
    simpleGround->setRenderDistance(8);
    std::cout << "Simple chunk terrain ground created with infinite terrain capability" << std::endl;
    
    std::cout << "Initializing SimpleChunkTerrainGround..." << std::endl;
    if (!simpleGround->initialize()) {
        std::cerr << "Failed to initialize SimpleChunkTerrainGround!" << std::endl;
    } else {
        std::cout << "SimpleChunkTerrainGround initialized successfully!" << std::endl;
    }
    
    // Store ground reference for entity system
    Ground* groundPtr = simpleGround.get();
    scene->addGameObject(std::move(simpleGround));
    scene->setGroundReference(groundPtr);
    
    // Add water surface
    std::cout << "Creating water surface..." << std::endl;
    auto waterSurface = std::make_unique<Water>("WaterSurface", -10.0f); // Water at terrain base level (-10.0f)
    waterSurface->setPosition(Vec3(0.0f, 0.0f, 0.0f)); // Position at origin, height handled by shader
    waterSurface->setScale(Vec3(1.0f, 1.0f, 1.0f)); // No scaling needed
    
    // Configure water parameters
    waterSurface->setWaveSpeed(0.05f);        // Faster waves
    waterSurface->setDistortionScale(0.02f);  // More distortion
    waterSurface->setShineDamper(15.0f);      // Softer shine
    waterSurface->setReflectivity(0.7f);      // More reflective
    
    if (!waterSurface->initialize()) {
        std::cerr << "Failed to initialize water surface!" << std::endl;
    } else {
        std::cout << "Water surface initialized successfully!" << std::endl;
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
        std::cout << "Renderer assigned to minimap" << std::endl;
    } else {
        std::cerr << "Warning: No default renderer available for minimap" << std::endl;
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
        std::cerr << "Failed to initialize minimap" << std::endl;
    }
    
    // Create weapon (FPS-style weapon rendering)
    weapon = std::make_unique<Weapon>("PlayerWeapon", 
                                     "Resources/Objects/WeaponsPack_V.1/WeaponsPack_V.1/OBJ/AssaultRifle_01.obj",
                                     Vec3(0.8f, 0.8f, 0.8f));  // Light metallic color for better visibility
    
    std::cout << "=== WEAPON CREATION DEBUG ===" << std::endl;
    std::cout << "Weapon created with color: (0.8f, 0.8f, 0.8f)" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Configure weapon properties - restore to original working position
    weapon->setWeaponScale(0.3f);  // Original scale
    weapon->setScreenPosition(Vec3(0.4f, -0.4f, 0.0f));  // Original bottom-right position
    weapon->setWeaponOffset(Vec3(-0.05f, 0.1f, 0.0f));  // Original offset
    weapon->setAimSensitivity(0.5f);  // Reduce aim sensitivity
    weapon->setDefaultRotation(Vec3(0.0f, 0.0f, 0.0f));   // Face -Z (into screen)
    weapon->setPlayerCamera(camera.get());
    
    std::cout << "=== CALLING WEAPON INITIALIZATION ===" << std::endl;
    if (!weapon->initialize()) {
        std::cerr << "Failed to initialize weapon" << std::endl;
    } else {
        std::cout << "Weapon initialized successfully" << std::endl;
    }
    std::cout << "=== WEAPON INITIALIZATION COMPLETE ===" << std::endl;
    
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
    std::cout << "Weapon configured with recoil settings" << std::endl;
    
    // Set up projectile manager for weapon shooting system
    if (projectileManager && weapon) {
        weapon->setProjectileManager(projectileManager.get());
        std::cout << "Projectile manager connected to weapon shooting system" << std::endl;
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
                std::cout << "=== APPLYING CAMERA RECOIL ===" << std::endl;
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
                    std::cout << "=== APPLYING CAMERA RECOIL (SHOOTING) ===" << std::endl;
                    this->camera->applyRecoil(recoil);
                }
            });
        }
        std::cout << "Crosshair and camera recoil callbacks connected to weapon and shooting system" << std::endl;
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
        std::cout << "AmmoUI connected to weapon" << std::endl;
    }
    
    // Initialize AmmoUI
    if (!ammoUI->initialize()) {
        std::cerr << "Failed to initialize AmmoUI" << std::endl;
    } else {
        std::cout << "AmmoUI initialized successfully" << std::endl;
    }
    
    // Initialize monster spawner
    monsterSpawner = std::make_unique<MonsterSpawner>();
    monsterSpawner->initialize(scene.get(), weapon.get());
    std::cout << "MonsterSpawner initialized successfully" << std::endl;
    
    // TEMPORARILY DISABLED: Adding weapon to scene has ownership issues
    // std::cout << "=== ADDING WEAPON TO SCENE FOR TESTING ===" << std::endl;
    // scene->addGameObject(weapon.get());
    // std::cout << "=== WEAPON ADDED TO SCENE ===" << std::endl;
    
    std::cout << "Scene objects setup complete!" << std::endl;
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
    
    std::cout << "Fullscreen " << (isFullscreen ? "enabled" : "disabled") 
              << " - Resolution: " << windowWidth << "x" << windowHeight 
              << " (Aspect: " << getAspectRatio() << ")" << std::endl;
}

void Game::printControls() {
    std::cout << "\n=== 3D Scene with Ground Plane, Water, and First-Person Camera (Counter-Strike Style) ===" << std::endl;
    std::cout << "WASD - Move forward/backward and strafe left/right on ground" << std::endl;
    std::cout << "Mouse - Look around (first-person view)" << std::endl;
    std::cout << "Space/Shift - Jump up/crouch down from ground level" << std::endl;
    std::cout << "F9 - Toggle fullscreen mode" << std::endl;
    std::cout << "ESC - Exit" << std::endl;
    std::cout << "\n=== WEAPON SWITCHING ===" << std::endl;
    std::cout << "1 - Assault Rifle" << std::endl;
    std::cout << "2 - Sniper Rifle" << std::endl;
    std::cout << "3 - Submachine Gun" << std::endl;
    std::cout << "4 - Pistol" << std::endl;
    std::cout << "5 - Shotgun" << std::endl;
    std::cout << "\n=== SHOOTING CONTROLS ===" << std::endl;
    std::cout << "Left Mouse Button - Start/Stop firing" << std::endl;
    std::cout << "Right Mouse Button - Single shot" << std::endl;
    std::cout << "R - Reload weapon" << std::endl;
    std::cout << "\n=== DEBUG CONTROLS ===" << std::endl;
    std::cout << "T - Show terrain statistics" << std::endl;
    std::cout << "W - Show water statistics" << std::endl;
    std::cout << "\nFeatures:" << std::endl;
    std::cout << "- Crosshair for aiming" << std::endl;
    std::cout << "- 5 different weapon models with unique properties" << std::endl;
    std::cout << "- Modular shooting system with projectile physics" << std::endl;
    std::cout << "- Realistic water rendering with reflection and refraction" << std::endl;
    std::cout << "- Wave animation and distortion effects" << std::endl;
    std::cout << "- Larger window (1200x800) with fullscreen support" << std::endl;
    std::cout << "- Dynamic window resizing with proper aspect ratio handling" << std::endl;
    std::cout << "==================================================================================\n" << std::endl;
}

void Game::onWindowResize(int width, int height) {
    // Update window dimensions
    windowWidth = width;
    windowHeight = height;
    
    // Update all renderers with new dimensions
    RendererFactory::getInstance().setViewport(width, height);
    
    std::cout << "Window resized to: " << width << "x" << height << " (Aspect ratio: " 
              << static_cast<float>(width) / static_cast<float>(height) << ")" << std::endl;
    
    // Additional debugging information
    Renderer* defaultRenderer = RendererFactory::getInstance().getDefaultRenderer();
    if (defaultRenderer) {
        std::cout << "  Renderer aspect ratio: " << defaultRenderer->getAspectRatio() << std::endl;
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
            std::cout << "Window minimized" << std::endl;
        } else {
            std::cout << "Window restored" << std::endl;
            // Get current window size and update
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            game->onWindowResize(width, height);
        }
    }
}

} // namespace Engine