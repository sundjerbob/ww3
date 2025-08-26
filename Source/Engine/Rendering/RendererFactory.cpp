/**
 * RendererFactory.cpp - Implementation of the renderer factory
 */

#include "RendererFactory.h"
#include <iostream>

namespace Engine {

RendererFactory* RendererFactory::instance = nullptr;

RendererFactory& RendererFactory::getInstance() {
    if (!instance) {
        instance = new RendererFactory();
    }
    return *instance;
}

bool RendererFactory::initialize(int width, int height) {
    if (isInitialized) return true;
    
    std::cout << "Initializing RendererFactory..." << std::endl;
    
    // Create and initialize Basic renderer
    auto basicRenderer = std::make_unique<BasicRenderer>();
    if (!basicRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize BasicRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Basic] = std::move(basicRenderer);
    
    // Create and initialize Crosshair renderer
    auto crosshairRenderer = std::make_unique<CrosshairRenderer>();
    if (!crosshairRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize CrosshairRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Crosshair] = std::move(crosshairRenderer);
    
    // Create and initialize Weapon renderer
    auto weaponRenderer = std::make_unique<WeaponRenderer>();
    if (!weaponRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize WeaponRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Weapon] = std::move(weaponRenderer);
    
    // Create and initialize Monster renderer
    auto monsterRenderer = std::make_unique<MonsterRenderer>();
    if (!monsterRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize MonsterRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Monster] = std::move(monsterRenderer);
    
    // Create and initialize Lighting renderer
    auto lightingRenderer = std::make_unique<LightingRenderer>();
    if (!lightingRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize LightingRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Lighting] = std::move(lightingRenderer);
    
    // Create and initialize Simple Text renderer
    auto textRenderer = std::make_unique<SimpleTextRenderer>();
    if (!textRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize SimpleTextRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Text] = std::move(textRenderer);
    
    // Create and initialize Water renderer
    auto waterRenderer = std::make_unique<WaterRenderer>();
    if (!waterRenderer->initialize(width, height)) {
        std::cerr << "Failed to initialize WaterRenderer" << std::endl;
        return false;
    }
    renderers[RendererType::Water] = std::move(waterRenderer);
    
    isInitialized = true;
    std::cout << "RendererFactory initialized successfully" << std::endl;
    return true;
}

void RendererFactory::cleanup() {
    if (!isInitialized) return;
    
    std::cout << "Cleaning up RendererFactory..." << std::endl;
    
    for (auto& pair : renderers) {
        if (pair.second) {
            pair.second->cleanup();
        }
    }
    renderers.clear();
    isInitialized = false;
}

Renderer* RendererFactory::getRenderer(RendererType type) {
    if (!isInitialized) {
        std::cerr << "RendererFactory not initialized!" << std::endl;
        return nullptr;
    }
    
    auto it = renderers.find(type);
    if (it != renderers.end()) {
        return it->second.get();
    }
    
    std::cerr << "Renderer type not found, returning default renderer" << std::endl;
    return getDefaultRenderer();
}

bool RendererFactory::hasRenderer(RendererType type) const {
    return renderers.find(type) != renderers.end();
}

Renderer* RendererFactory::getDefaultRenderer() {
    return getRenderer(RendererType::Basic);
}

void RendererFactory::setViewport(int width, int height) {
    for (auto& pair : renderers) {
        if (pair.second) {
            pair.second->setViewport(width, height);
        }
    }
}

} // namespace Engine
