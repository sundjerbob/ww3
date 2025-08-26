/**
 * Water.h - Water Surface GameObject
 * 
 * Represents a water surface with reflection, refraction, and wave animation.
 * Uses the WaterRenderer for specialized water rendering effects.
 */

#pragma once
#include "../Engine/Core/GameObject.h"
#include "../Engine/Rendering/WaterRenderer.h"
#include <memory>

namespace Engine {

class Water : public GameObject {
private:
    float waterHeight;
    float waveSpeed;
    float distortionScale;
    float shineDamper;
    float reflectivity;
    
    // Water-specific renderer
    WaterRenderer* waterRenderer;

public:
    Water(const std::string& name, float height = 0.0f);
    ~Water() override;

    // GameObject interface
    bool initialize() override;
    void update(float deltaTime) override;
    void render(const Renderer& renderer, const Camera& camera) override;
    
    // Water-specific methods
    void setWaterHeight(float height) { waterHeight = height; }
    float getWaterHeight() const { return waterHeight; }
    
    void setWaveSpeed(float speed) { waveSpeed = speed; }
    float getWaveSpeed() const { return waveSpeed; }
    
    void setDistortionScale(float scale) { distortionScale = scale; }
    float getDistortionScale() const { return distortionScale; }
    
    void setShineDamper(float damper) { shineDamper = damper; }
    float getShineDamper() const { return shineDamper; }
    
    void setReflectivity(float reflect) { reflectivity = reflect; }
    float getReflectivity() const { return reflectivity; }
    
    // Renderer selection
    RendererType getPreferredRendererType() const override { return RendererType::Water; }
    
    // Water renderer access
    WaterRenderer* getWaterRenderer() const { return waterRenderer; }
    void setWaterRenderer(WaterRenderer* renderer) { waterRenderer = renderer; }
    
    // Override model matrix to exclude Y translation (water height is handled in shader)
    Mat4 getWaterModelMatrix() const;

protected:
    void setupMesh() override;
};

} // namespace Engine
