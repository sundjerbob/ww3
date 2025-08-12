/**
 * Camera.h - First-Person Camera System
 * 
 * OVERVIEW:
 * Implements a Counter-Strike style first-person camera with mouse look and WASD movement.
 * Handles view matrix generation and camera state management.
 * 
 * MATHEMATICAL FOUNDATION:
 * Uses yaw and pitch angles to calculate viewing direction vectors.
 * Provides smooth first-person navigation with proper movement constraints.
 */

#pragma once
#include "Math.h"

namespace Engine {

/**
 * First Person Camera System (Counter-Strike Style)
 * 
 * MATHEMATICAL FOUNDATION:
 * Implements a first-person camera where the player IS the camera.
 * Uses yaw and pitch angles to calculate the viewing direction vector.
 * 
 * COORDINATE SYSTEM:
 * - Yaw (θ): Horizontal rotation around Y-axis (left/right looking)
 * - Pitch (φ): Vertical rotation around local X-axis (up/down looking)
 * - Position: Direct camera/player position in world space
 * 
 * DIRECTION VECTOR CALCULATION:
 * Forward direction from spherical angles:
 * x = cos(pitch) * cos(yaw)
 * y = sin(pitch)  
 * z = cos(pitch) * sin(yaw)
 */
class Camera {
private:
    Vec3 position;       // Current camera/player position in world space
    Vec3 up;            // World up vector (usually [0,1,0])
    Vec3 forward;       // Forward viewing direction vector
    Vec3 right;         // Right strafe direction vector
    float yaw, pitch;   // Viewing direction angles (radians)

public:
    // Constructor
    Camera();
    
    // Camera control
    void updateCameraVectors();
    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix() const;
    
    // Movement
    void moveForward(float distance);
    void moveBackward(float distance);
    void strafeLeft(float distance);
    void strafeRight(float distance);
    void moveUp(float distance);
    void moveDown(float distance);
    
    // Rotation
    void rotate(float yawOffset, float pitchOffset);
    
    // Getters
    Vec3 getPosition() const { return position; }
    Vec3 getForward() const { return forward; }
    Vec3 getRight() const { return right; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    
    // Setters
    void setPosition(const Vec3& pos) { position = pos; }
    void setRotation(const Vec3& rot); // Set rotation in degrees (x=pitch, y=yaw, z=roll)
};

} // namespace Engine