/**
 * MaterialLoader.h - MTL File Parser for Material Loading
 * 
 * OVERVIEW:
 * Parses .mtl (Material Template Library) files and loads material properties.
 * Supports standard MTL format used by OBJ files.
 * 
 * FEATURES:
 * - MTL file parsing
 * - Material property extraction
 * - Error handling and validation
 * - Multiple materials per file support
 */

#pragma once
#include "Material.h"
#include <string>

namespace Engine {

/**
 * MaterialLoader Class - MTL File Parser
 * 
 * Loads material definitions from .mtl files and creates Material objects.
 * Supports standard MTL format with common material properties.
 */
class MaterialLoader {
public:
    /**
     * Load materials from MTL file
     * 
     * @param mtlFilePath Path to the .mtl file
     * @return MaterialLibrary containing all loaded materials
     */
    static MaterialLibrary loadMTL(const std::string& mtlFilePath);
    
    /**
     * Check if MTL file exists and is readable
     * 
     * @param mtlFilePath Path to the .mtl file
     * @return true if file exists and can be read
     */
    static bool isValidMTLFile(const std::string& mtlFilePath);
    
    /**
     * Get MTL file path from OBJ file path
     * 
     * @param objFilePath Path to the .obj file
     * @return Expected path to the .mtl file (same name, .mtl extension)
     */
    static std::string getMTLPathFromOBJ(const std::string& objFilePath);

private:
    /**
     * Parse a single line from MTL file
     * 
     * @param line Line to parse
     * @param currentMaterial Current material being processed
     * @param materialLibrary Library to add completed materials to
     */
    static void parseMTLLine(const std::string& line, Material& currentMaterial, MaterialLibrary& materialLibrary);
    
    /**
     * Parse Vec3 color from string (e.g., "0.8 0.2 0.1")
     * 
     * @param colorStr String containing 3 float values
     * @return Vec3 color
     */
    static Vec3 parseColor(const std::string& colorStr);
    
    /**
     * Parse float value from string
     * 
     * @param valueStr String containing float value
     * @return Parsed float value
     */
    static float parseFloat(const std::string& valueStr);
    
    /**
     * Parse integer value from string
     * 
     * @param valueStr String containing integer value
     * @return Parsed integer value
     */
    static int parseInt(const std::string& valueStr);
    
    /**
     * Trim whitespace from string
     * 
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);
};

} // namespace Engine
