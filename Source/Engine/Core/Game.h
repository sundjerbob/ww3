/**
 * Game.h - Main Game Engine Class
 * 
 * OVERVIEW:
 * Root class of the game engine hierarchy. Orchestrates all engine systems
 * and provides the main game loop. Acts as the central coordinator for
 * graphics, input, camera, and other subsystems.
 * 
 * FEATURES:
 * - Engine initialization and cleanup
 * - Main game loop management
 * - System coordination (renderer, input, camera)
 * - Frame timing and delta time calculation
 * - Window management integration
 */

#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE  // Prevent GLFW from including OpenGL headers
#include <glfw3.h>
#include <memory>
#include "../Rendering/Renderer.h"
#include "../Math/Camera.h"
#include "../Input/Input.h"
#include "Scene.h"

namespace Engine {

// Forward declarations
class Minimap;

/**
 * Game Class - Engine Root and Main Coordinator
 * 
 * Central engine class that manages all subsystems:
 * - Window creation and management
 * - Renderer initialization and coordination
 * - Camera system management
 * - Input system integration
 * - Main game loop execution
 * - Frame timing and performance
 */
class Game {
private:
    // Window management
    GLFWwindow* window;
    int windowWidth, windowHeight;
    const char* windowTitle;
    bool isFullscreen;
    int windowed_width, windowed_height; // Store windowed mode dimensions
    
    // Engine systems
    std::unique_ptr<Renderer> renderer; // Renderer for the main game world
    std::unique_ptr<Renderer> overlayRenderer; // for UI elements like crosshair
    std::unique_ptr<Camera> camera; // Camera for the main game world
    std::unique_ptr<Scene> scene; // Scene for the main game world
    std::unique_ptr<Minimap> minimap; // Minimap for bird's-eye view
    
    // Game state
    bool isRunning; // Whether the game is running
    bool isInitialized; // Whether the engine is initialized
    
    // Timing
    float deltaTime; // Time between frames
    float lastFrame; // Time of the last frame
    
public:
    // Constructor/Destructor
    Game(int width = 1200, int height = 800, const char* title = "Game Engine");
    ~Game();
    
    // Engine lifecycle
    bool initialize();
    void run();
    void cleanup();
    
    // Game loop components
    void update(float deltaTime);
    void render();
    
    // Utility
    bool isValid() const { return isInitialized && isRunning; }
    void stop() { isRunning = false; }
    
    // Window control
    void toggleFullscreen();
    
    // Window resize handling
    void onWindowResize(int width, int height);
    float getAspectRatio() const { return static_cast<float>(windowWidth) / static_cast<float>(windowHeight); }
    
private:
    // Helper methods
    bool initializeGLFW();
    bool createWindow();
    bool initializeGLEW();
    void setupSystems();
    void setupSceneObjects();
    void calculateDeltaTime();
    void printControls();
    
    // Static callback functions
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void windowIconifyCallback(GLFWwindow* window, int iconified);
};

} // namespace Engine