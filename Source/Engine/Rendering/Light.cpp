/**
 * Light.cpp - Lighting System Implementation
 */

#include "Light.h"
#include <iostream>

namespace Engine {

// Base Light Class Implementation
Light::Light() 
    : type(LightType::Point), name("DefaultLight"), position(0.0f, 0.0f, 0.0f), 
      direction(0.0f, -1.0f, 0.0f), color(1.0f, 1.0f, 1.0f), intensity(1.0f),
      constant(1.0f), linear(0.09f), quadratic(0.032f), range(10.0f), isEnabled(true) {
}

Light::Light(const std::string& lightName, LightType lightType)
    : type(lightType), name(lightName), position(0.0f, 0.0f, 0.0f), 
      direction(0.0f, -1.0f, 0.0f), color(1.0f, 1.0f, 1.0f), intensity(1.0f),
      constant(1.0f), linear(0.09f), quadratic(0.032f), range(10.0f), isEnabled(true) {
}

void Light::calculateAttenuationFromRange(float lightRange) {
    range = lightRange;
    
    // Calculate attenuation coefficients based on range
    // These values provide good falloff over the specified range
    constant = 1.0f;
    linear = 2.0f / lightRange;
    quadratic = 1.0f / (lightRange * lightRange);
}

float Light::calculateAttenuation(float distance) const {
    if (type != LightType::Point) {
        return 1.0f; // No attenuation for directional/ambient lights
    }
    
    float attenuation = constant + linear * distance + quadratic * distance * distance;
    return 1.0f / attenuation;
}

// Directional Light Implementation
DirectionalLight::DirectionalLight(const std::string& name)
    : Light(name, LightType::Directional) {
    // Default sun-like directional light
    setDirection(Vec3(0.0f, -1.0f, 0.0f)); // Shining down
    setColor(Vec3(1.0f, 0.95f, 0.8f));     // Warm sunlight
    setIntensity(1.0f);
}

DirectionalLight::DirectionalLight(const std::string& name, const Vec3& direction, const Vec3& color, float intensity)
    : Light(name, LightType::Directional) {
    setDirection(direction);
    setColor(color);
    setIntensity(intensity);
}

// Point Light Implementation
PointLight::PointLight(const std::string& name)
    : Light(name, LightType::Point) {
    // Default point light
    setPosition(Vec3(0.0f, 5.0f, 0.0f));
    setColor(Vec3(1.0f, 1.0f, 1.0f));
    setIntensity(1.0f);
    setRange(10.0f);
    calculateAttenuationFromRange(10.0f);
}

PointLight::PointLight(const std::string& name, const Vec3& position, const Vec3& color, float intensity, float range)
    : Light(name, LightType::Point) {
    setPosition(position);
    setColor(color);
    setIntensity(intensity);
    setRange(range);
    calculateAttenuationFromRange(range);
}

// Ambient Light Implementation
AmbientLight::AmbientLight(const std::string& name)
    : Light(name, LightType::Ambient) {
    // Default ambient light
    setColor(Vec3(0.1f, 0.1f, 0.2f)); // Slight blue tint for sky
    setIntensity(0.3f);
}

AmbientLight::AmbientLight(const std::string& name, const Vec3& color, float intensity)
    : Light(name, LightType::Ambient) {
    setColor(color);
    setIntensity(intensity);
}

} // namespace Engine
