/**
 * Camera.cpp - Implementation of First-Person Camera System
 * 
 * Implements all camera functionality including movement, rotation, and view matrix generation.
 * Provides smooth Counter-Strike style first-person controls.
 */

#include "Camera.h"
#include <iostream>
#include <algorithm>

namespace Engine {

Camera::Camera() 
    : position(8, 10, 8), lastPosition(8, 10, 8), up(0, 1, 0), yaw(-90.0f * 3.14159f / 180.0f), pitch(0),
      baseRotation(0.0f, 0.0f, 0.0f), recoilRotation(0.0f, 0.0f, 0.0f), recoilRecoveryRate(5.0f) {
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // PROPER ANGLE NORMALIZATION: Always keep angles within (-π, π] range
    // This prevents floating-point error accumulation from large angle values
    const float PI = 3.14159f;
    
    // Normalize yaw to (-π, π] range to prevent trigonometric drift
    while (yaw > PI) yaw -= 2.0f * PI;
    while (yaw <= -PI) yaw += 2.0f * PI;
    
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
    // Store last position before moving
    lastPosition = position;
    
    // Move forward along camera's viewing direction (horizontal only)
    Vec3 forwardMovement = forward;
    forwardMovement.y = 0; // Keep movement horizontal (no flying when looking up/down)
    forwardMovement = forwardMovement.normalize();
    position = position + forwardMovement * distance;
}

void Camera::moveBackward(float distance) {
    // Store last position before moving
    lastPosition = position;
    
    // Move backward opposite to camera's viewing direction (horizontal only)
    Vec3 backwardMovement = forward;
    backwardMovement.y = 0; // Keep movement horizontal
    backwardMovement = backwardMovement.normalize();
    position = position - backwardMovement * distance;
}

void Camera::strafeLeft(float distance) {
    // Store last position before moving
    lastPosition = position;
    
    // Strafe left perpendicular to viewing direction
    position = position - right * distance;
}

void Camera::strafeRight(float distance) {
    // Store last position before moving
    lastPosition = position;
    
    // Strafe right perpendicular to viewing direction
    position = position + right * distance;
}

void Camera::moveUp(float distance) {
    // Store last position before moving
    lastPosition = position;
    
    // Move up along world Y-axis (jumping)
    position.y += distance;
}

void Camera::moveDown(float distance) {
    // Store last position before moving
    lastPosition = position;
    
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
    
    // Yaw normalization is now handled in updateCameraVectors()
    
    // Update camera direction vectors when view angles change
    updateCameraVectors();
}

void Camera::setRotation(const Vec3& rot) {
    // Convert degrees to radians and set yaw and pitch
    // Note: rot.x = pitch, rot.y = yaw, rot.z = roll (unused for FPS camera)
    pitch = rot.x * 3.14159f / 180.0f;
    yaw = rot.y * 3.14159f / 180.0f;
    
    // Constrain pitch to prevent camera flipping (but allow -90° for top-down view)
    const float maxPitch = 89.0f * 3.14159f / 180.0f;
    if (pitch > maxPitch) pitch = maxPitch;
    if (pitch < -maxPitch) pitch = -maxPitch;
    
    // Special case: Allow exactly -90° for top-down view (minimap)
    if (rot.x == -90.0f) {
        pitch = -90.0f * 3.14159f / 180.0f;
    }
    
    // Yaw normalization is now handled in updateCameraVectors()
    
    updateCameraVectors();
}

void Camera::setTopDownView() {
    // Set camera to true top-down view
    pitch = -90.0f * 3.14159f / 180.0f; // Exactly -90 degrees (looking straight down)
    yaw = 0.0f; // Facing north (doesn't matter for top-down, but keep it consistent)
    
    updateCameraVectors();
}

Vec3 Camera::getRotation() const {
    // Return rotation in degrees (x=pitch, y=yaw, z=roll)
    return Vec3(
        pitch * 180.0f / 3.14159f,  // Convert pitch from radians to degrees
        yaw * 180.0f / 3.14159f,    // Convert yaw from radians to degrees
        0.0f                        // Roll is always 0 for FPS camera
    );
}

// Recoil system implementation
void Camera::applyRecoil(const Vec3& recoil) {
    // Store base rotation if this is the first recoil
    if (recoilRotation.x == 0.0f && recoilRotation.y == 0.0f) {
        baseRotation = getRotation();
    }
    
    // Apply recoil to camera rotation (pitch upward)
    float cameraRecoil = recoil.y * 3.0f; // Much stronger camera recoil for visibility
    recoilRotation.x -= cameraRecoil; // Negative for upward camera movement
    
    // Apply the combined rotation (base + recoil)
    Vec3 finalRotation = baseRotation + recoilRotation;
    setRotation(finalRotation);
    
}

void Camera::updateRecoil(float deltaTime) {
    // Fast camera recoil recovery - always return to base rotation
    if (recoilRotation.x != 0.0f || recoilRotation.y != 0.0f || recoilRotation.z != 0.0f) {
        // Recover recoil rotation quickly
        if (recoilRotation.x < 0.0f) {  // Negative values = upward recoil
            recoilRotation.x += recoilRecoveryRate * deltaTime;
            recoilRotation.x = std::min(0.0f, recoilRotation.x); // Don't go past zero
        } else if (recoilRotation.x > 0.0f) {
            recoilRotation.x -= recoilRecoveryRate * deltaTime;
            recoilRotation.x = std::max(0.0f, recoilRotation.x);
        }
        
        // Apply the updated rotation
        Vec3 finalRotation = baseRotation + recoilRotation;
        setRotation(finalRotation);
        
        // Update base rotation when recoil is nearly zero
        if (std::abs(recoilRotation.x) < 0.01f) {
            baseRotation = getRotation();
            recoilRotation.x = 0.0f;
        }
    }
}

} // namespace Engine