#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // For 2D arrow, we work directly in NDC space
    // Apply only the model matrix (rotation) and ignore view/projection
    vec4 pos = model * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0); // Force Z to 0 for 2D
}
