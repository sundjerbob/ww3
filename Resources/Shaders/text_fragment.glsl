#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture;
uniform vec3 textColor;
uniform float alpha;

void main()
{
    // Sample the alpha from the font texture
    float textAlpha = texture(textTexture, TexCoord).r;
    
    // Apply text color with alpha blending
    FragColor = vec4(textColor, textAlpha * alpha);
}
