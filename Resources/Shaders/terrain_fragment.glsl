/*
 * TERRAIN FRAGMENT SHADER - Height-Based Terrain Coloring with Lighting
 * 
 * PURPOSE:
 * Determines the final color of each pixel (fragment) on the terrain surface.
 * Uses height-based coloring with proper normal-based lighting.
 * 
 * MATHEMATICAL PROCESS:
 * 1. Input: Interpolated world position and normal from vertex shader
 * 2. Calculate: Fragment color based on height and lighting
 * 3. Output: Final RGBA color value for this pixel
 * 
 * COLOR REPRESENTATION:
 * RGBA values in range [0.0, 1.0] where:
 * - R (Red): 0.8 = 80% red intensity
 * - G (Green): 0.4 = 40% green intensity  
 * - B (Blue): 0.2 = 20% blue intensity
 * - A (Alpha): 1.0 = 100% opacity (no transparency)
 * 
 * GPU PROCESSING:
 * After fragment shader, GPU performs:
 * - Depth testing (Z-buffer comparison)
 * - Alpha blending (if transparency is used)
 * - Frame buffer writing (final pixel color storage)
 * 
 * RENDERING PIPELINE POSITION:
 * Vertex Shader → Rasterization → Fragment Shader → Frame Buffer
 */

#version 330 core
in vec3 outPos;     // World position from vertex shader
in vec3 outNormal;  // World normal from vertex shader
out vec4 FragColor; // Output color for this fragment/pixel

uniform vec3 color;              // Per-object color (used when height coloring is disabled)
uniform int useHeightColoring;   // 1 = enable height-based coloring (terrain), 0 = use object color

// Lighting uniforms
uniform vec3 lightDirection;     // Direction of the light (should be normalized)
uniform vec3 lightColor;         // Color of the light
uniform vec3 ambientColor;       // Ambient light color
uniform float ambientStrength;   // Ambient light strength
uniform float diffuseStrength;   // Diffuse light strength

void main()
{
    vec3 baseColor;
    
    if (useHeightColoring == 1) {
        // Dynamic color based on world position (used for terrain rendering):
        // Updated for more sensitive height ranges to show terrain variation
        float height = outPos.y;
        
        // More sensitive height ranges for brighter terrain visualization
        if (height <= -16.0) {
            // Deep underground - very dark brown/black
            baseColor = vec3(0.02f, 0.01f, 0.0f);
        } else if (height <= -14.0) {
            // Deep underground - dark brown
            baseColor = vec3(0.05f, 0.02f, 0.0f);
        } else if (height <= -12.0) {
            // Underground - brown
            baseColor = vec3(0.1f, 0.05f, 0.0f);
        } else if (height <= -10.0) {
            // Underground - lighter brown
            baseColor = vec3(0.2f, 0.1f, 0.05f);
        } else if (height <= -8.0) {
            // Low terrain - dark sand
            baseColor = vec3(0.4f, 0.3f, 0.2f);
        } else if (height <= -6.0) {
            // Low terrain - sand
            baseColor = vec3(0.6f, 0.5f, 0.3f);
        } else if (height <= -4.0) {
            // Low terrain - light sand
            baseColor = vec3(0.8f, 0.7f, 0.5f);
        } else if (height <= -2.0) {
            // Ground level - beige
            baseColor = vec3(0.9f, 0.8f, 0.6f);
        } else if (height <= 0.0) {
            // Slightly above ground - light beige
            baseColor = vec3(0.8f, 0.9f, 0.7f);
        } else if (height <= 2.0) {
            // Low grass - light green
            baseColor = vec3(0.4f, 0.8f, 0.4f);
        } else if (height <= 4.0) {
            // Grass - green
            baseColor = vec3(0.3f, 0.7f, 0.3f);
        } else if (height <= 6.0) {
            // Dark grass - darker green
            baseColor = vec3(0.2f, 0.6f, 0.2f);
        } else if (height <= 8.0) {
            // Forest - dark green
            baseColor = vec3(0.15f, 0.5f, 0.15f);
        } else if (height <= 10.0) {
            // Mountain base - gray-brown
            baseColor = vec3(0.4f, 0.4f, 0.3f);
        } else if (height <= 12.0) {
            // Mountain - gray
            baseColor = vec3(0.5f, 0.5f, 0.5f);
        } else if (height <= 14.0) {
            // High mountain - light gray
            baseColor = vec3(0.6f, 0.6f, 0.6f);
        } else {
            // Peak - white/snow
            baseColor = vec3(0.9f, 0.9f, 0.9f);
        }
        
        // Apply directional lighting to terrain using proper normals
        vec3 normal = normalize(outNormal);
        vec3 lightDir = normalize(lightDirection);
        
        // Ambient lighting
        vec3 ambient = ambientStrength * ambientColor;
        
        // Diffuse lighting using proper normal
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseStrength * diff * lightColor;
        
        // Combine lighting with base color
        vec3 result = (ambient + diffuse) * baseColor;
        
        FragColor = vec4(result, 1.0f);
    } else {
        // Per-object solid color (used for non-terrain objects)
        FragColor = vec4(color, 1.0f);
    }
}
