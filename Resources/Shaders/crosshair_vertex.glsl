#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // For 2D overlay, we ignore view and projection matrices
    // and work directly in NDC space
    gl_Position = vec4(aPos, 1.0);
}

