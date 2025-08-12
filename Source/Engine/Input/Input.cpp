/**
 * Input.cpp - Implementation of Input Management System
 * 
 * Handles all keyboard and mouse input processing for the game engine.
 * Provides smooth first-person controls with proper sensitivity settings.
 */

#include "Input.h"
#include <iostream>
#include <cstring>

namespace Engine {

// Static instance for singleton pattern
Input* Input::instance = nullptr;

Input::Input() : lastX(300), lastY(300), firstMouse(true), mouseSensitivity(0.002f), camera(nullptr) {
    // Initialize all keys to false
    memset(keys, false, sizeof(keys));
}

Input& Input::getInstance() {
    if (instance == nullptr) {
        instance = new Input();
    }
    return *instance;
}

void Input::cleanup() {
    delete instance;
    instance = nullptr;
}

void Input::initialize(GLFWwindow* window, Camera* cam) {
    camera = cam;
    
    // Set GLFW callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    
    // Capture mouse cursor for FPS controls
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::processInput(float deltaTime) {
    if (!camera) return;
    
    // Reduced movement speed for more controlled camera movement
    float speed = 2.0f * deltaTime;
    
    // WASD movement
    if (keys[GLFW_KEY_W]) {
        camera->moveForward(speed);
    }
    if (keys[GLFW_KEY_S]) {
        camera->moveBackward(speed);
    }
    if (keys[GLFW_KEY_A]) {
        camera->strafeLeft(speed);
    }
    if (keys[GLFW_KEY_D]) {
        camera->strafeRight(speed);
    }
    
    // Vertical movement
    if (keys[GLFW_KEY_SPACE]) {
        camera->moveUp(speed);
    }
    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        camera->moveDown(speed);
    }
}

bool Input::isKeyPressed(int key) const {
    if (key >= 0 && key < 1024) {
        return keys[key];
    }
    return false;
}

void Input::resetMousePosition(float x, float y) {
    lastX = x;
    lastY = y;
    firstMouse = true;
}

// Static callback functions

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    Input& input = getInstance();
    
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            input.keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            input.keys[key] = false;
        }
    }
    
    // Handle escape key to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    // Handle F9 key to toggle fullscreen (changed from F11 for easier debugging)
    if (key == GLFW_KEY_F9 && action == GLFW_PRESS) {
        if (input.fullscreenToggleCallback) {
            input.fullscreenToggleCallback();
        }
    }
}

void Input::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Input& input = getInstance();
    
    if (input.firstMouse) {
        input.lastX = static_cast<float>(xpos);
        input.lastY = static_cast<float>(ypos);
        input.firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - input.lastX);
    float yoffset = static_cast<float>(input.lastY - ypos); // Reversed since y-coordinates go from bottom to top
    input.lastX = static_cast<float>(xpos);
    input.lastY = static_cast<float>(ypos);

    xoffset *= input.mouseSensitivity;
    yoffset *= input.mouseSensitivity;

    if (input.camera) {
        input.camera->rotate(xoffset, yoffset);
    }
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Scroll functionality disabled for first-person camera
    // Could be used for weapon switching or other FPS mechanics in the future
}

} // namespace Engine