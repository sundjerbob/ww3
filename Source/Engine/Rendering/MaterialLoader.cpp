/**
 * MaterialLoader.cpp - Implementation of MTL File Parser
 */

#include "MaterialLoader.h"
#include <fstream>
#include <sstream>
#include <iostream                                                                                                                                                                                                                                 >
#include <algorithm>

namespace Engine {

MaterialLibrary MaterialLoader::loadMTL(const std::string& mtlFilePath) {
    MaterialLibrary materialLibrary;
    
    std::ifstream file(mtlFilePath);
    if (!file.is_open()) {
        return materialLibrary;
    }
    
    
    std::string line;
    Material currentMaterial;
    bool hasMaterial = false;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse the line
        if (line.substr(0, 6) == "newmtl") {
            // Save previous material if it exists
            if (hasMaterial) {
                materialLibrary.addMaterial(currentMaterial);
            }
            
            // Start new material
            std::string materialName = trim(line.substr(6));
            currentMaterial = Material(materialName);
            hasMaterial = true;
            
        } else if (hasMaterial) {
            parseMTLLine(line, currentMaterial, materialLibrary);
        }
    }
    
    // Add the last material
    if (hasMaterial) {
        materialLibrary.addMaterial(currentMaterial);
    }
    
    file.close();
    
    
    // Debug: Print all loaded materials
    for (const auto& materialName : materialLibrary.getMaterialNames()) {
        const Material* mat = materialLibrary.getMaterial(materialName);
        if (mat) {
            // std::cout << "Material '" << materialName << "' loaded with color ("
            //           << mat->diffuse.x << ", " << mat->diffuse.y << ", " << mat->diffuse.z << ")" << std::endl;
        }
    }
    
    return materialLibrary;
}

bool MaterialLoader::isValidMTLFile(const std::string& mtlFilePath) {
    std::ifstream file(mtlFilePath);
    return file.good();
}

std::string MaterialLoader::getMTLPathFromOBJ(const std::string& objFilePath) {
    // Replace .obj extension with .mtl
    size_t lastDot = objFilePath.find_last_of('.');
    if (lastDot != std::string::npos) {
        return objFilePath.substr(0, lastDot) + ".mtl";
    }
    return objFilePath + ".mtl";
}

void MaterialLoader::parseMTLLine(const std::string& line, Material& currentMaterial, MaterialLibrary& materialLibrary) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    
    if (command == "Ka") {
        // Ambient color
        std::string colorStr;
        std::getline(iss, colorStr);
        currentMaterial.ambient = parseColor(colorStr);
        
    } else if (command == "Kd") {
        // Diffuse color (main color)
        std::string colorStr;
        std::getline(iss, colorStr);
        currentMaterial.diffuse = parseColor(colorStr);
        
    } else if (command == "Ks") {
        // Specular color
        std::string colorStr;
        std::getline(iss, colorStr);
        currentMaterial.specular = parseColor(colorStr);
        
    } else if (command == "Ke") {
        // Emissive color
        std::string colorStr;
        std::getline(iss, colorStr);
        currentMaterial.emissive = parseColor(colorStr);
    } else if (command == "Ns") {
        // Shininess
        std::string valueStr;
        std::getline(iss, valueStr);
        currentMaterial.shininess = parseFloat(valueStr);
        
    } else if (command == "d") {
        // Alpha/transparency
        std::string valueStr;
        std::getline(iss, valueStr);
        currentMaterial.alpha = parseFloat(valueStr);
        
    } else if (command == "Ni") {
        // Refraction index
        std::string valueStr;
        std::getline(iss, valueStr);
        currentMaterial.refractionIndex = parseFloat(valueStr);
        
    } else if (command == "illum") {
        // Illumination model
        std::string valueStr;
        std::getline(iss, valueStr);
        currentMaterial.illuminationModel = parseInt(valueStr);
        
    } else if (command == "map_Kd") {
        // Diffuse texture map
        std::string textureStr;
        std::getline(iss, textureStr);
        currentMaterial.diffuseTexture = trim(textureStr);
        
    } else if (command == "map_Bump" || command == "bump") {
        // Normal/bump map
        std::string textureStr;
        std::getline(iss, textureStr);
        currentMaterial.normalTexture = trim(textureStr);
        
    } else if (command == "map_Ks") {
        // Specular texture map
        std::string textureStr;
        std::getline(iss, textureStr);
        currentMaterial.specularTexture = trim(textureStr);
    }
    // Ignore unrecognized commands
}

Vec3 MaterialLoader::parseColor(const std::string& colorStr) {
    std::istringstream iss(colorStr);
    float r = 0.0f, g = 0.0f, b = 0.0f;
    iss >> r >> g >> b;
    return Vec3(r, g, b);
}

float MaterialLoader::parseFloat(const std::string& valueStr) {
    try {
        return std::stof(trim(valueStr));
    } catch (const std::exception&) {
        return 0.0f;
    }
}

int MaterialLoader::parseInt(const std::string& valueStr) {
    try {
        return std::stoi(trim(valueStr));
    } catch (const std::exception&) {
        return 0;
    }
}

std::string MaterialLoader::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

} // namespace Engine
