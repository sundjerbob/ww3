/**
 * Crosshair.h - GameObject representing a 2D crosshair overlay
 */

#pragma once
#include "../Engine/Core/GameObject.h"

namespace Engine {

class Crosshair : public GameObject {
public:
    Crosshair(const std::string& name = "Crosshair");
    ~Crosshair() override = default;

    bool initialize() override;
    void update(float deltaTime) override;
    void render(const Renderer& renderer, const Camera& camera) override;
    RendererType getPreferredRendererType() const override { return RendererType::Crosshair; }
    
    // Recoil methods
    void applyRecoil(const Vec3& recoil);
    void updateRecoil(float deltaTime);

protected:
    void setupMesh() override;
    
private:
    // Recoil system
    Vec3 recoilOffset;
    Vec3 recoilVelocity;
    float recoilRecoveryRate;
};

} // namespace Engine


