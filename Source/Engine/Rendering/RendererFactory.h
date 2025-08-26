/**
 * RendererFactory.h - Factory for creating and accessing renderer instances
 * 
 * Provides centralized access to different renderer implementations.
 * GameObjects can request the appropriate renderer type for their needs.
 */

#pragma once
#include "Renderer.h"
#include "BasicRenderer.h"
#include "CrosshairRenderer.h"
#include "WeaponRenderer.h"
#include "MonsterRenderer.h"
#include "LightingRenderer.h"
#include "SimpleTextRenderer.h"
#include "WaterRenderer.h"
#include <memory>
#include <unordered_map>

namespace Engine {

enum class RendererType {
    Basic,      // For 3D world objects
    Crosshair,  // For 2D overlay elements
    Weapon,     // For weapon rendering with texture support
    Monster,    // For 3D monster rendering with multi-material support
    Lighting,   // For advanced lighting rendering
    Text,       // For text rendering with fonts
    Water,      // For water rendering with reflection/refraction
    Minimap     // For minimap rendering (future)
};

class RendererFactory {
private:
    static RendererFactory* instance;
    std::unordered_map<RendererType, std::unique_ptr<Renderer>> renderers;
    bool isInitialized;

    RendererFactory() : isInitialized(false) {}

public:
    static RendererFactory& getInstance();
    
    // Initialize all renderers
    bool initialize(int width, int height);
    void cleanup();
    
    // Get renderer by type
    Renderer* getRenderer(RendererType type);
    
    // Check if a specific renderer type is available
    bool hasRenderer(RendererType type) const;
    
    // Get the default renderer (Basic)
    Renderer* getDefaultRenderer();
    
    // Update viewport for all renderers
    void setViewport(int width, int height);

private:
    RendererFactory(const RendererFactory&) = delete;
    RendererFactory& operator=(const RendererFactory&) = delete;
};

} // namespace Engine
