#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float alpha;

void main()
{
    // Output the specified color with alpha support
    FragColor = vec4(color, alpha);
}

