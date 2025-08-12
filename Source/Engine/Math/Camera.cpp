/**
 * Camera.cpp - Implementation of First-Person Camera System
 * 
 * Implements all camera functionality including movement, rotation, and view matrix generation.
 * Provides smooth Counter-Strike style first-person controls.
 */

#include "Camera.h"

namespace Engine {

Camera::Camera() 
    : position(0, 0, 5), up(0, 1, 0), yaw(-90.0f * 3.14159f / 180.0f), pitch(0) {
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // Calculate forward direction from spherical coordinates
    forward.x = cos(pitch) * cos(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * sin(yaw);
    forward = forward.normalize();
    
    // Calculate right and up vectors for movement
    right = forward.cross(Vec3(0, 1, 0)).normalize();
    up = right.cross(forward).normalize();
}

Mat4 Camera::getViewMatrix() const {
    Vec3 target = position + forward; // Look at point in forward direction
    return Engine::lookAt(position, target, up);
}

Mat4 Camera::getProjectionMatrix() const {
    // Create a perspective projection matrix
    // Using typical FPS camera settings
    float fovy = 45.0f * 3.14159f / 180.0f; // 45 degrees field of view
    float aspect = 16.0f / 9.0f; // 16:9 aspect ratio
    float near = 0.1f;
    float far = 100.0f;
    
    return Engine::perspective(fovy, aspect, near, far);
}

void Camera::moveForward(float distance) {
    // Move forward along camera's viewing direction (horizontal only)
    Vec3 forwardMovement = forward;
    forwardMovement.y = 0; // Keep movement horizontal (no flying when looking up/down)
    forwardMovement = forwardMovement.normalize();
    position = position + forwardMovement * distance;
}

void Camera::moveBackward(float distance) {
    // Move backward opposite to camera's viewing direction (horizontal only)
    Vec3 backwardMovement = forward;
    backwardMovement.y = 0; // Keep movement horizontal
    backwardMovement = backwardMovement.normalize();
    position = position - backwardMovement * distance;
}

void Camera::strafeLeft(float distance) {
    // Strafe left perpendicular to viewing direction
    position = position - right * distance;
}

void Camera::strafeRight(float distance) {
    // Strafe right perpendicular to viewing direction
    position = position + right * distance;
}

void Camera::moveUp(float distance) {
    // Move up along world Y-axis (jumping)
    position.y += distance;
}

void Camera::moveDown(float distance) {
    // Move down along world Y-axis (crouching)
    position.y -= distance;
}

void Camera::rotate(float yawOffset, float pitchOffset) {
    yaw += yawOffset;
    pitch += pitchOffset;
    
    // Constrain pitch to prevent camera flipping (typical FPS constraint)
    const float maxPitch = 89.0f * 3.14159f / 180.0f; // 89 degrees in radians
    if (pitch > maxPitch) pitch = maxPitch;
    if (pitch < -maxPitch) pitch = -maxPitch;
    
    // Update camera direction vectors when view angles change
    updateCameraVectors();
}

void Camera::setRotation(const Vec3& rot) {
    // Convert degrees to radians and set yaw and pitch
    // Note: rot.x = pitch, rot.y = yaw, rot.z = roll (unused for FPS camera)
    pitch = rot.x * 3.14159f / 180.0f;
    yaw = rot.y * 3.14159f / 180.0f;
    
    // Constrain pitch to prevent camera flipping
    const float maxPitch = 89.0f * 3.14159f / 180.0f;
    if (pitch > maxPitch) pitch = maxPitch;
    if (pitch < -maxPitch) pitch = -maxPitch;
    
    updateCameraVectors();
}

} // namespace Engine