#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // For minimap, we want to render in screen space
    // So we'll use the position directly as screen coordinates
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
