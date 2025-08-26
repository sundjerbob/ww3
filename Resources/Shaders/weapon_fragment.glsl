/*
 * WEAPON FRAGMENT SHADER - Pixel Color Generation with Texture Support
 * 
 * PURPOSE:
 * Determines the final color of each pixel (fragment) on the rendered weapon surface.
 * Supports texture mapping with fallback to solid colors for backward compatibility.
 * 
 * FEATURES:
 * - Texture sampling for realistic weapon materials
 * - Fallback to solid colors when no texture is available
 * - Material color tinting support
 * - Proper alpha blending for weapon overlays
 * - White color transparency support
 */

#version 330 core
in vec3 outPos;     // Position from vertex shader
in vec2 TexCoord;   // Texture coordinates from vertex shader
out vec4 FragColor; // Output color for this fragment/pixel

uniform vec3 color;              // Per-object color (used when texture is disabled)
uniform int useTexture;          // 1 = enable texture sampling, 0 = use object color
uniform sampler2D weaponTexture; // Weapon material texture
uniform float textureStrength;   // How much to blend texture vs color (0.0 to 1.0)

// Function to check if a color is close to white and should be transparent
bool isWhiteColor(vec3 color, float threshold) {
    return color.r > threshold && color.g > threshold && color.b > threshold;
}

void main()
{
    if (useTexture == 1) {
        // Sample texture for realistic weapon materials
        vec4 texColor = texture(weaponTexture, TexCoord);
        
        // Check if texture color is white (close to 1.0) and make it transparent
        if (isWhiteColor(texColor.rgb, 0.98)) {
            discard; // Discard this fragment (make it transparent)
        }
        
        // Blend texture with object color based on texture strength
        vec3 finalColor = mix(color, texColor.rgb, textureStrength);
        
        // Use texture alpha or full opacity
        float alpha = texColor.a > 0.0 ? texColor.a : 1.0;
        FragColor = vec4(finalColor, alpha);
    } else {
        // Fallback to solid color (backward compatibility)
        // Check if the object color is white and make it transparent
        if (isWhiteColor(color, 0.98)) {
            discard; // Discard this fragment (make it transparent)
        }
        FragColor = vec4(color, 1.0f);
    }
}
