/**
 * LightingMaterial.cpp - Material Properties Implementation
 */

#include "LightingMaterial.h"

namespace Engine {

// Constructor implementations
LightingMaterial::LightingMaterial()
    : name("Default"), ambient(0.1f, 0.1f, 0.1f), diffuse(0.7f, 0.7f, 0.7f), 
      specular(0.5f, 0.5f, 0.5f), shininess(32.0f) {
}

LightingMaterial::LightingMaterial(const std::string& materialName)
    : name(materialName), ambient(0.1f, 0.1f, 0.1f), diffuse(0.7f, 0.7f, 0.7f), 
      specular(0.5f, 0.5f, 0.5f), shininess(32.0f) {
}

LightingMaterial::LightingMaterial(const std::string& materialName, const Vec3& amb, const Vec3& diff, const Vec3& spec, float shine)
    : name(materialName), ambient(amb), diffuse(diff), specular(spec), shininess(shine) {
}

// Material preset implementations
LightingMaterial LightingMaterial::createDefault() {
    return LightingMaterial("Default", Vec3(0.1f, 0.1f, 0.1f), Vec3(0.7f, 0.7f, 0.7f), Vec3(0.5f, 0.5f, 0.5f), 32.0f);
}

LightingMaterial LightingMaterial::createMetal() {
    return LightingMaterial("Metal", Vec3(0.2f, 0.2f, 0.2f), Vec3(0.8f, 0.8f, 0.8f), Vec3(1.0f, 1.0f, 1.0f), 128.0f);
}

LightingMaterial LightingMaterial::createPlastic() {
    return LightingMaterial("Plastic", Vec3(0.0f, 0.0f, 0.0f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.7f, 0.7f, 0.7f), 32.0f);
}

LightingMaterial LightingMaterial::createWood() {
    return LightingMaterial("Wood", Vec3(0.1f, 0.05f, 0.02f), Vec3(0.4f, 0.2f, 0.1f), Vec3(0.1f, 0.05f, 0.02f), 8.0f);
}

LightingMaterial LightingMaterial::createStone() {
    return LightingMaterial("Stone", Vec3(0.2f, 0.2f, 0.2f), Vec3(0.6f, 0.6f, 0.6f), Vec3(0.3f, 0.3f, 0.3f), 16.0f);
}

LightingMaterial LightingMaterial::createGlass() {
    return LightingMaterial("Glass", Vec3(0.0f, 0.0f, 0.0f), Vec3(0.1f, 0.1f, 0.1f), Vec3(1.0f, 1.0f, 1.0f), 256.0f);
}

LightingMaterial LightingMaterial::createRubber() {
    return LightingMaterial("Rubber", Vec3(0.02f, 0.02f, 0.02f), Vec3(0.1f, 0.1f, 0.1f), Vec3(0.4f, 0.4f, 0.4f), 10.0f);
}

LightingMaterial LightingMaterial::createFabric() {
    return LightingMaterial("Fabric", Vec3(0.0f, 0.0f, 0.0f), Vec3(0.6f, 0.6f, 0.6f), Vec3(0.0f, 0.0f, 0.0f), 4.0f);
}

LightingMaterial LightingMaterial::createLeather() {
    return LightingMaterial("Leather", Vec3(0.05f, 0.05f, 0.05f), Vec3(0.3f, 0.2f, 0.1f), Vec3(0.1f, 0.1f, 0.1f), 16.0f);
}

LightingMaterial LightingMaterial::createCeramic() {
    return LightingMaterial("Ceramic", Vec3(0.1f, 0.1f, 0.1f), Vec3(0.7f, 0.7f, 0.7f), Vec3(0.8f, 0.8f, 0.8f), 64.0f);
}

} // namespace Engine
