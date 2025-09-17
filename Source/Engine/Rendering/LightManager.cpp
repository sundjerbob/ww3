/**
 * LightManager.cpp - Light Management Implementation
 */

#include "LightManager.h"
#include <iostream>
#include <algorithm>

namespace Engine {

LightManager::LightManager() {
    // Initialize with default lighting
    setupDefaultLighting();
}

LightManager::~LightManager() {
    clearAllLights();
}

void LightManager::addLight(std::shared_ptr<Light> light) {
    if (!light) {
        return;
    }
    
    const std::string& lightName = light->getName();
    
    // Check if light already exists
    if (hasLight(lightName)) {
        removeLight(lightName);
    }
    
    // Add to appropriate collection based on type
    switch (light->getType()) {
        case LightType::Directional: {
            auto directionalLight = std::dynamic_pointer_cast<DirectionalLight>(light);
            if (directionalLight) {
                if (directionalLights.size() < MAX_DIRECTIONAL_LIGHTS) {
                    directionalLights.push_back(directionalLight);
                    lightMap[lightName] = light;
                } else {
                }
            }
            break;
        }
        case LightType::Point: {
            auto pointLight = std::dynamic_pointer_cast<PointLight>(light);
            if (pointLight) {
                if (pointLights.size() < MAX_POINT_LIGHTS) {
                    pointLights.push_back(pointLight);
                    lightMap[lightName] = light;
                } else {
                }
            }
            break;
        }
        case LightType::Ambient: {
            auto ambientLight = std::dynamic_pointer_cast<AmbientLight>(light);
            if (ambientLight) {
                if (ambientLights.size() < MAX_AMBIENT_LIGHTS) {
                    ambientLights.push_back(ambientLight);
                    lightMap[lightName] = light;
                } else {
                }
            }
            break;
        }
    }
}

void LightManager::removeLight(const std::string& lightName) {
    auto it = lightMap.find(lightName);
    if (it == lightMap.end()) {
        return; // Light not found
    }
    
    std::shared_ptr<Light> light = it->second;
    lightMap.erase(it);
    
    // Remove from appropriate collection
    switch (light->getType()) {
        case LightType::Directional: {
            auto it = std::find_if(directionalLights.begin(), directionalLights.end(),
                [&lightName](const std::shared_ptr<DirectionalLight>& dl) {
                    return dl->getName() == lightName;
                });
            if (it != directionalLights.end()) {
                directionalLights.erase(it);
            }
            break;
        }
        case LightType::Point: {
            auto it = std::find_if(pointLights.begin(), pointLights.end(),
                [&lightName](const std::shared_ptr<PointLight>& pl) {
                    return pl->getName() == lightName;
                });
            if (it != pointLights.end()) {
                pointLights.erase(it);
            }
            break;
        }
        case LightType::Ambient: {
            auto it = std::find_if(ambientLights.begin(), ambientLights.end(),
                [&lightName](const std::shared_ptr<AmbientLight>& al) {
                    return al->getName() == lightName;
                });
            if (it != ambientLights.end()) {
                ambientLights.erase(it);
            }
            break;
        }
    }
    
}

void LightManager::clearAllLights() {
    directionalLights.clear();
    pointLights.clear();
    ambientLights.clear();
    lightMap.clear();
}

std::shared_ptr<Light> LightManager::getLight(const std::string& lightName) {
    auto it = lightMap.find(lightName);
    return (it != lightMap.end()) ? it->second : nullptr;
}

std::shared_ptr<DirectionalLight> LightManager::getDirectionalLight(const std::string& name) {
    auto it = std::find_if(directionalLights.begin(), directionalLights.end(),
        [&name](const std::shared_ptr<DirectionalLight>& dl) {
            return dl->getName() == name;
        });
    return (it != directionalLights.end()) ? *it : nullptr;
}

std::shared_ptr<PointLight> LightManager::getPointLight(const std::string& name) {
    auto it = std::find_if(pointLights.begin(), pointLights.end(),
        [&name](const std::shared_ptr<PointLight>& pl) {
            return pl->getName() == name;
        });
    return (it != pointLights.end()) ? *it : nullptr;
}

std::shared_ptr<AmbientLight> LightManager::getAmbientLight(const std::string& name) {
    auto it = std::find_if(ambientLights.begin(), ambientLights.end(),
        [&name](const std::shared_ptr<AmbientLight>& al) {
            return al->getName() == name;
        });
    return (it != ambientLights.end()) ? *it : nullptr;
}

bool LightManager::hasLight(const std::string& lightName) const {
    return lightMap.find(lightName) != lightMap.end();
}

void LightManager::updateLight(const std::string& lightName, const Vec3& position, const Vec3& direction, const Vec3& color, float intensity) {
    auto light = getLight(lightName);
    if (light) {
        light->setPosition(position);
        light->setDirection(direction);
        light->setColor(color);
        light->setIntensity(intensity);
    }
}

void LightManager::setupDefaultLighting() {
    clearAllLights();
    
    // Add ambient light
    auto ambient = std::make_shared<AmbientLight>("DefaultAmbient", Vec3(0.1f, 0.1f, 0.2f), 0.3f);
    addLight(ambient);
    
    // Add directional light (sun)
    auto sun = std::make_shared<DirectionalLight>("Sun", Vec3(0.0f, -1.0f, 0.0f), Vec3(1.0f, 0.95f, 0.8f), 1.0f);
    addLight(sun);
    
}

void LightManager::setupDayLighting() {
    clearAllLights();
    
    // Bright ambient light for day
    auto ambient = std::make_shared<AmbientLight>("DayAmbient", Vec3(0.2f, 0.3f, 0.5f), 0.4f);
    addLight(ambient);
    
    // Bright sun
    auto sun = std::make_shared<DirectionalLight>("Sun", Vec3(0.0f, -1.0f, 0.0f), Vec3(1.0f, 0.95f, 0.8f), 1.2f);
    addLight(sun);
    
}

void LightManager::setupNightLighting() {
    clearAllLights();
    
    // Dark ambient light for night
    auto ambient = std::make_shared<AmbientLight>("NightAmbient", Vec3(0.05f, 0.05f, 0.1f), 0.1f);
    addLight(ambient);
    
    // Dim moon
    auto moon = std::make_shared<DirectionalLight>("Moon", Vec3(0.0f, -1.0f, 0.0f), Vec3(0.7f, 0.7f, 1.0f), 0.3f);
    addLight(moon);
    
}

void LightManager::setupIndoorLighting() {
    clearAllLights();
    
    // Low ambient light for indoor
    auto ambient = std::make_shared<AmbientLight>("IndoorAmbient", Vec3(0.1f, 0.1f, 0.1f), 0.2f);
    addLight(ambient);
    
    // Indoor point lights
    auto light1 = std::make_shared<PointLight>("CeilingLight1", Vec3(0.0f, 8.0f, 0.0f), Vec3(1.0f, 1.0f, 0.9f), 1.0f, 15.0f);
    auto light2 = std::make_shared<PointLight>("CeilingLight2", Vec3(5.0f, 8.0f, 5.0f), Vec3(1.0f, 1.0f, 0.9f), 0.8f, 12.0f);
    auto light3 = std::make_shared<PointLight>("CeilingLight3", Vec3(-5.0f, 8.0f, -5.0f), Vec3(1.0f, 1.0f, 0.9f), 0.8f, 12.0f);
    
    addLight(ambient);
    addLight(light1);
    addLight(light2);
    addLight(light3);
    
}

} // namespace Engine
