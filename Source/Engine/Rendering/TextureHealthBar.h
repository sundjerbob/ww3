/**
 * TextureHealthBar.h - 2D Texture-Based Health Bar System
 * 
 * This system renders health bars using 2D textures instead of 3D geometry.
 * Benefits:
 * - Single draw call per health bar (vs 3 for old system)
 * - Better performance
 * - More visual flexibility (gradients, effects, etc.)
 * - Easier to modify and customize
 */

#pragma once
#include "../Math/Math.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>
#include <vector>

namespace Engine {

class Camera;

class TextureHealthBar {
private:
    // Texture and rendering
    std::unique_ptr<Texture> healthBarTexture;
    std::unique_ptr<Shader> healthBarShader;
    unsigned int healthBarVAO;
    unsigned int healthBarVBO;
    unsigned int healthBarEBO;
    
    // Health bar properties
    float barWidth;
    float barHeight;
    float offsetY;
    
    // Health state
    float currentHealth;
    float maxHealth;
    float targetHealth;
    float healthTransitionSpeed;
    
    // Visual properties
    Vec3 backgroundColor;
    Vec3 healthColor;
    Vec3 borderColor;
    float alpha;
    
    // Billboard properties
    bool alwaysFaceCamera;
    Vec3 billboardUp;
    
    // Animation
    float pulseTimer;
    float pulseSpeed;
    bool isPulsing;
    
    // State
    bool isInitialized;
    bool isActive;
    
public:
    TextureHealthBar(float width = 2.0f, float height = 0.3f, float offset = 2.0f);
    ~TextureHealthBar();
    
    // Initialization and cleanup
    bool initialize();
    void cleanup();
    
    // Health management
    void setHealth(float health, float maxHealth = 100.0f);
    void setTargetHealth(float target);
    float getHealth() const { return currentHealth; }
    float getMaxHealth() const { return maxHealth; }
    float getHealthPercentage() const { return currentHealth / maxHealth; }
    
    // Visual properties
    void setColors(const Vec3& background, const Vec3& health, const Vec3& border);
    void setAlpha(float alpha) { this->alpha = alpha; }
    void setPulsing(bool pulsing) { isPulsing = pulsing; }
    void setPulseSpeed(float speed) { pulseSpeed = speed; }
    
    // Positioning
    void setOffsetY(float offset) { offsetY = offset; }
    float getOffsetY() const { return offsetY; }
    
    // State management
    void setActive(bool active) { isActive = active; }
    bool getActive() const { return isActive; }
    
    // Update and rendering
    void update(float deltaTime);
    void render(const Vec3& monsterPosition, const Camera& camera);
    
    // Texture generation
    void generateHealthBarTexture();
    void updateHealthBarTexture();
    
private:
    // Internal methods
    void setupGeometry();
    void setupShader();
    void cleanupGeometry();
    Mat4 getBillboardMatrix(const Vec3& monsterPosition, const Camera& camera) const;
    Vec3 getHealthColorForPercentage(float percentage) const;
    void updateHealthTransition(float deltaTime);
    void updatePulseAnimation(float deltaTime);
};

} // namespace Engine
