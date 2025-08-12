#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D minimapTexture;

void main()
{
    // Sample the minimap texture
    vec4 texColor = texture(minimapTexture, TexCoord);
    
    // Add a border effect
    vec2 border = step(vec2(0.05), TexCoord) * step(vec2(0.05), 1.0 - TexCoord);
    float borderFactor = border.x * border.y;
    
    // Mix texture with border color
    vec3 borderColor = vec3(0.2, 0.2, 0.2);
    vec3 finalColor = mix(borderColor, texColor.rgb, borderFactor);
    
    FragColor = vec4(finalColor, 1.0);
}
