/**
 * HealthBar.h - 3D Health Bar System for Monsters
 * 
 * OVERVIEW:
 * Implements a 3D health bar that appears above monsters in world space.
 * The health bar is always oriented towards the camera and shows current health percentage.
 * 
 * FEATURES:
 * - 3D world-space positioning above monsters
 * - Always faces the camera (billboard effect)
 * - Color-coded health display (green->yellow->red)
 * - Smooth health transitions
 * - Configurable size and offset
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Math/Math.h"
#include "../Engine/Rendering/Mesh.h"
#include "../Engine/Rendering/Shader.h"
#include <memory>

namespace Engine {

class Camera;
class Renderer;

/**
 * HealthBar - 3D Health Bar for Monsters
 * 
 * Renders a health bar above a monster that always faces the camera.
 * The bar shows the current health percentage with color coding.
 */
class HealthBar : public GameObject {
private:
    // Health data
    float currentHealth;
    float maxHealth;
    float targetHealth;  // For smooth transitions
    float healthTransitionSpeed;
    
    // Visual properties
    Vec3 backgroundColor;    // Background color of the bar
    Vec3 healthColor;        // Current health color
    Vec3 borderColor;        // Border color
    float barWidth;          // Width of the health bar
    float barHeight;         // Height of the health bar
    float borderThickness;   // Thickness of the border
    float offsetY;           // Offset above the monster
    
    // Billboard properties
    bool alwaysFaceCamera;
    Vec3 billboardUp;        // Up vector for billboard orientation
    
    // Animation
    float pulseTimer;
    float pulseSpeed;
    bool isPulsing;
    
    // References
    GameObject* targetMonster;  // Monster this health bar belongs to
    
    // Rendering
    std::unique_ptr<Mesh> backgroundMesh;
    std::unique_ptr<Mesh> healthMesh;
    std::unique_ptr<Mesh> borderMesh;
    std::unique_ptr<Shader> healthBarShader;

public:
    // Constructor/Destructor
    HealthBar(const std::string& name, GameObject* monster = nullptr);
    virtual ~HealthBar() = default;
    
    // Core functionality
    virtual bool initialize() override;
    virtual void update(float deltaTime) override;
    virtual void render(const Renderer& renderer, const Camera& camera) override;
    virtual void cleanup() override;
    
    // Health management
    void setHealth(float health, float max = 100.0f);
    void setCurrentHealth(float health);
    void setMaxHealth(float max);
    float getCurrentHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercentage() const { return maxHealth > 0.0f ? currentHealth / maxHealth : 0.0f; }
    
    // Visual configuration
    void setBarSize(float width, float height);
    void setBarWidth(float width) { barWidth = width; }
    void setBarHeight(float height) { barHeight = height; }
    void setOffsetY(float offset) { offsetY = offset; }
    void setBorderThickness(float thickness) { borderThickness = thickness; }
    
    // Color configuration
    void setBackgroundColor(const Vec3& color) { backgroundColor = color; }
    void setHealthColor(const Vec3& color) { healthColor = color; }
    void setBorderColor(const Vec3& color) { borderColor = color; }
    Vec3 getBackgroundColor() const { return backgroundColor; }
    Vec3 getHealthColor() const { return healthColor; }
    Vec3 getBorderColor() const { return borderColor; }
    
    // Animation configuration
    void setHealthTransitionSpeed(float speed) { healthTransitionSpeed = speed; }
    void setPulseSpeed(float speed) { pulseSpeed = speed; }
    void setPulsing(bool pulse) { isPulsing = pulse; }
    
    // Target monster
    void setTargetMonster(GameObject* monster) { targetMonster = monster; }
    GameObject* getTargetMonster() const { return targetMonster; }
    
    // Billboard configuration
    void setAlwaysFaceCamera(bool face) { alwaysFaceCamera = face; }
    void setBillboardUp(const Vec3& up) { billboardUp = up; }
    
    // Utility
    bool isVisible() const;
    Vec3 getHealthColorForPercentage(float percentage) const;
    Mat4 getBillboardMatrix(const Camera& camera) const;
    void updatePositionFromMonster();

protected:
    // Override points
    virtual void onHealthChanged(float oldHealth, float newHealth);
    virtual void onHealthPercentageChanged(float oldPercentage, float newPercentage);
    
    // Internal helpers
    void setupMeshes();
    void setupShader();
    void updateHealthTransition(float deltaTime);
    void updatePulseAnimation(float deltaTime);
    void updateHealthMesh();
    void renderHealthBar(const Renderer& renderer, const Camera& camera) const;
    void renderBackground(const Renderer& renderer, const Camera& camera) const;
    void renderHealth(const Renderer& renderer, const Camera& camera) const;
    void renderBorder(const Renderer& renderer, const Camera& camera) const;
    
    // New health bar rendering methods
    void renderHealthBarBackground(const Renderer& renderer, const Camera& camera, const Mat4& billboardMatrix);
    void renderHealthBarFill(const Renderer& renderer, const Camera& camera, const Mat4& billboardMatrix);
    void renderHealthBarBorder(const Renderer& renderer, const Camera& camera, const Mat4& billboardMatrix);
};

} // namespace Engine
