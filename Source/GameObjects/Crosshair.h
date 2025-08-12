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
    void render(const Renderer& renderer, const Camera& camera) override;

protected:
    void setupMesh() override;
};

} // namespace Engine


