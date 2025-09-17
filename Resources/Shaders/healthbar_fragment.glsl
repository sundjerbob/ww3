#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

// Health bar uniforms
uniform float healthPercentage;  // 0.0 to 1.0
uniform vec3 backgroundColor;    // Background color
uniform vec3 healthColor;        // Health fill color
uniform vec3 borderColor;        // Border color
uniform float alpha;             // Overall alpha

void main()
{
    // Create health bar effect based on UV coordinates
    vec3 finalColor;
    
    // Simple border check
    if (TexCoord.x < 0.05 || TexCoord.x > 0.95 || TexCoord.y < 0.05 || TexCoord.y > 0.95) {
        // Border area - use border color
        finalColor = borderColor;
    } else {
        // Inside health bar - check health percentage
        if (TexCoord.x <= healthPercentage) {
            // Health area - use health color
            finalColor = healthColor;
        } else {
            // Empty health area - use background color
            finalColor = backgroundColor;
        }
    }
    
    FragColor = vec4(finalColor, alpha);
}
