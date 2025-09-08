#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float alpha;

void main()
{
    // Simple unlit color output - no lighting calculations
    FragColor = vec4(color, alpha);
}
