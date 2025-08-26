/**
 * Input.h - Input Management System
 * 
 * OVERVIEW:
 * Handles keyboard and mouse input for the game engine.
 * Provides clean interface for input polling and event handling.
 * 
 * FEATURES:
 * - Keyboard state management
 * - Mouse movement tracking
 * - First-person camera controls
 * - Extensible input system
 */

#pragma once
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE  // Prevent GLFW from including OpenGL headers
#include <glfw3.h>
#include <functional>
#include "../Math/Camera.h"

namespace Engine {

/**
 * Input Manager Class
 * 
 * Centralizes all input handling for the engine:
 * - Keyboard state tracking
 * - Mouse movement processing
 * - Camera control integration
 * - Input polling and event processing
 */
class Input {
private:
    static Input* instance;
    
    // Input state
    bool keys[1024];
    bool mouseButtons[8]; // Support for up to 8 mouse buttons
    float lastX, lastY;
    bool firstMouse;
    float mouseSensitivity;
    
    // Camera reference for movement
    Camera* camera;
    
    // Fullscreen toggle callback
    std::function<void()> fullscreenToggleCallback;
    
    // Private constructor for singleton
    Input();

public:
    // Singleton access
    static Input& getInstance();
    static void cleanup();
    
    // Initialization
    void initialize(GLFWwindow* window, Camera* cam);
    
    // Input processing
    void processInput(float deltaTime);
    void setCamera(Camera* cam) { camera = cam; }
    
    // Callback setters
    void setFullscreenToggleCallback(std::function<void()> callback) { fullscreenToggleCallback = callback; }
    
    // GLFW callback functions (static)
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // Settings
    void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
    float getMouseSensitivity() const { return mouseSensitivity; }
    
    // Key state queries
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    
    // Mouse state
    void resetMousePosition(float x, float y);
};

} // namespace Engine