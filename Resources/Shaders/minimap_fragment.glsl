#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D minimapTexture;
uniform float terrainHeight;  // Height data for terrain coloring
uniform bool isTerrain;       // Flag to indicate if this is terrain rendering

void main()
{
    // Sample the minimap texture
    vec4 texColor = texture(minimapTexture, TexCoord);
    
    // Apply height-based coloring for terrain
    if (isTerrain) {
        // Create height-based color gradient for terrain with more sensitive ranges
        vec3 terrainColor;
        if (terrainHeight < -16.0) {
            // Deep underground - very dark brown/black
            terrainColor = vec3(0.02, 0.01, 0.0);
        } else if (terrainHeight < -14.0) {
            // Deep underground - dark brown
            terrainColor = vec3(0.05, 0.02, 0.0);
        } else if (terrainHeight < -12.0) {
            // Underground - brown
            terrainColor = vec3(0.1, 0.05, 0.0);
        } else if (terrainHeight < -10.0) {
            // Underground - lighter brown
            terrainColor = vec3(0.2, 0.1, 0.05);
        } else if (terrainHeight < -8.0) {
            // Low terrain - dark sand
            terrainColor = vec3(0.4, 0.3, 0.2);
        } else if (terrainHeight < -6.0) {
            // Low terrain - sand
            terrainColor = vec3(0.6, 0.5, 0.3);
        } else if (terrainHeight < -4.0) {
            // Low terrain - light sand
            terrainColor = vec3(0.8, 0.7, 0.5);
        } else if (terrainHeight < -2.0) {
            // Ground level - beige
            terrainColor = vec3(0.9, 0.8, 0.6);
        } else if (terrainHeight < 0.0) {
            // Slightly above ground - light beige
            terrainColor = vec3(0.8, 0.9, 0.7);
        } else if (terrainHeight < 2.0) {
            // Low grass - light green
            terrainColor = vec3(0.4, 0.8, 0.4);
        } else if (terrainHeight < 4.0) {
            // Grass - green
            terrainColor = vec3(0.3, 0.7, 0.3);
        } else if (terrainHeight < 6.0) {
            // Dark grass - darker green
            terrainColor = vec3(0.2, 0.6, 0.2);
        } else if (terrainHeight < 8.0) {
            // Forest - dark green
            terrainColor = vec3(0.15, 0.5, 0.15);
        } else if (terrainHeight < 10.0) {
            // Mountain base - gray-brown
            terrainColor = vec3(0.4, 0.4, 0.3);
        } else if (terrainHeight < 12.0) {
            // Mountain - gray
            terrainColor = vec3(0.5, 0.5, 0.5);
        } else if (terrainHeight < 14.0) {
            // High mountain - light gray
            terrainColor = vec3(0.6, 0.6, 0.6);
        } else {
            // Peak - white/snow
            terrainColor = vec3(0.9, 0.9, 0.9);
        }
        
        texColor.rgb = terrainColor;
    }
    
    // Add a border effect
    vec2 border = step(vec2(0.05), TexCoord) * step(vec2(0.05), 1.0 - TexCoord);
    float borderFactor = border.x * border.y;
    
    // Mix texture with border color
    vec3 borderColor = vec3(0.2, 0.2, 0.2);
    vec3 finalColor = mix(borderColor, texColor.rgb, borderFactor);
    
    FragColor = vec4(finalColor, 1.0);
}
