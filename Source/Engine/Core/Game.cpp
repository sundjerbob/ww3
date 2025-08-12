/**
 * Game.cpp - Implementation of Main Game Engine Class
 * 
 * Complete implementation of the game engine root class.
 * Handles initialization, main loop, and coordination of all engine systems.
 */

#include "Game.h"
#include "../../GameObjects/Cube.h"
#include "../../GameObjects/Ground.h"
#include "../Rendering/BasicRenderer.h"
#include "../../GameObjects/Crosshair.h"
#include "../Rendering/CrosshairRenderer.h"
#include "../../GameObjects/Minimap.h"
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
    
    // Initialize renderer (world)
    renderer = std::make_unique<BasicRenderer>();
    if (!renderer->initialize(windowWidth, windowHeight)) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        isRunning = false;
        return;
    }

    // Initialize overlay renderer (for crosshair)
    overlayRenderer = std::make_unique<CrosshairRenderer>();
    if (!overlayRenderer->initialize(windowWidth, windowHeight)) {
        std::cerr << "Failed to initialize overlay renderer" << std::endl;
        // Not fatal; continue without overlay
    }
    
    // Initialize scene
    scene = std::make_unique<Scene>("MainScene");
    if (!scene->initialize()) {
        std::cerr << "Failed to initialize scene" << std::endl;
        isRunning = false;
        return;
    }
    
    // Setup scene objects
    setupSceneObjects();

    // Add crosshair object rendered by a specialized renderer implementation
    auto crosshair = std::make_unique<Crosshair>("Crosshair");
    crosshair->setRenderer(overlayRenderer.get());
    if (!crosshair->initialize()) {
        std::cerr << "Failed to initialize crosshair" << std::endl;
    } else {
        scene->addGameObject(std::move(crosshair));
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
    
    std::cout << "Starting main game loop..." << std::endl;
    
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
    
    std::cout << "Exiting main game loop..." << std::endl;
}

void Game::update(float deltaTime) {
    // Process input
    Input& input = Input::getInstance();
    input.processInput(deltaTime);
    
    // Update scene
    if (scene) {
        scene->update(deltaTime);
    }
}

void Game::render() {
    if (!renderer || !scene) return;
    
    renderer->beginFrame();
    
    // Render scene using the new GameObject system
    scene->render(*camera, *renderer);
    
    // Render minimap (UI overlay)
    if (minimap) {
        minimap->render(*renderer, *camera);
    }
    
    // Crosshair rendering removed from BasicRenderer; keep call out
    
    renderer->endFrame(window);
}

void Game::calculateDeltaTime() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Game::cleanup() {
    std::cout << "Cleaning up Game Engine..." << std::endl;
    
    // Clean up systems
    minimap.reset();
    scene.reset();
    renderer.reset();
    camera.reset();
    Input::cleanup();
    
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
    
    // Add ground
    auto ground = std::make_unique<Ground>("Ground", 50.0f, Vec3(0.4f, 0.3f, 0.2f));
    ground->setRenderer(renderer.get());
    scene->addGameObject(std::move(ground));
    
    // Add rotating cube at center
    auto centerCube = std::make_unique<Cube>("CenterCube", Vec3(1.0f, 0.5f, 0.0f));
    centerCube->setPosition(Vec3(0.0f, 0.0f, 0.0f));
    centerCube->setRotating(true);
    centerCube->setRotationSpeed(90.0f);
    centerCube->setRenderer(renderer.get());
    scene->addGameObject(std::move(centerCube));
    
    // Add static cubes around the scene
    auto redCube = std::make_unique<Cube>("RedCube", Vec3(1.0f, 0.0f, 0.0f));
    redCube->setPosition(Vec3(5.0f, 0.0f, 3.0f));
    redCube->setRenderer(renderer.get());
    scene->addGameObject(std::move(redCube));
    
    auto greenCube = std::make_unique<Cube>("GreenCube", Vec3(0.0f, 1.0f, 0.0f));
    greenCube->setPosition(Vec3(-3.0f, 0.0f, 7.0f));
    greenCube->setRenderer(renderer.get());
    scene->addGameObject(std::move(greenCube));
    
    auto blueCube = std::make_unique<Cube>("BlueCube", Vec3(0.0f, 0.0f, 1.0f));
    blueCube->setPosition(Vec3(8.0f, 0.0f, -2.0f));
    blueCube->setRenderer(renderer.get());
    scene->addGameObject(std::move(blueCube));
    
    auto magentaCube = std::make_unique<Cube>("MagentaCube", Vec3(1.0f, 0.0f, 1.0f));
    magentaCube->setPosition(Vec3(-6.0f, 0.0f, -4.0f));
    magentaCube->setRenderer(renderer.get());
    scene->addGameObject(std::move(magentaCube));
    
    // Add minimap (UI element, not part of 3D scene)
    minimap = std::make_unique<Minimap>("Minimap", 0.25f);
    minimap->setScene(scene.get());
    minimap->setRenderer(renderer.get());
    if (!minimap->initialize()) {
        std::cerr << "Failed to initialize minimap" << std::endl;
    }
    
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
    
    // Update renderer with new dimensions
    if (renderer) {
        renderer->setViewport(windowWidth, windowHeight);
    }
    
    std::cout << "Fullscreen " << (isFullscreen ? "enabled" : "disabled") 
              << " - Resolution: " << windowWidth << "x" << windowHeight 
              << " (Aspect: " << getAspectRatio() << ")" << std::endl;
}

void Game::printControls() {
    std::cout << "\n=== 3D Scene with Ground Plane and First-Person Camera (Counter-Strike Style) ===" << std::endl;
    std::cout << "WASD - Move forward/backward and strafe left/right on ground" << std::endl;
    std::cout << "Mouse - Look around (first-person view)" << std::endl;
    std::cout << "Space/Shift - Jump up/crouch down from ground level" << std::endl;
    std::cout << "F9 - Toggle fullscreen mode" << std::endl;
    std::cout << "ESC - Exit" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "- Crosshair for aiming" << std::endl;
    std::cout << "- Larger window (1200x800) with fullscreen support" << std::endl;
    std::cout << "- Dynamic window resizing with proper aspect ratio handling" << std::endl;
    std::cout << "==================================================================================\n" << std::endl;
}

void Game::onWindowResize(int width, int height) {
    // Update window dimensions
    windowWidth = width;
    windowHeight = height;
    
    // Update renderer viewport and projection matrix
    if (renderer) {
        renderer->setViewport(width, height);
    }
    

    
    std::cout << "Window resized to: " << width << "x" << height << " (Aspect ratio: " 
              << static_cast<float>(width) / static_cast<float>(height) << ")" << std::endl;
    
    // Additional debugging information
    if (renderer) {
        std::cout << "  Renderer aspect ratio: " << renderer->getAspectRatio() << std::endl;
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