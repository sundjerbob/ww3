/*
 * DEPTH MAP VERTEX SHADER - Shadow Mapping Depth Generation
 * 
 * PURPOSE:
 * Transforms vertices to light space for depth map generation.
 * Used in the first pass of shadow mapping to create depth textures.
 * 
 * FEATURES:
 * - Light space transformation
 * - Depth-only rendering
 * - Support for multiple light sources
 */

#version 330 core
layout (location = 0) in vec3 aPos;    // Vertex position attribute
layout (location = 1) in vec3 aNormal; // Vertex normal attribute (unused in depth pass)

// Light space transformation matrix
uniform mat4 lightSpaceMatrix;

void main()
{
    // Transform vertex to light space for depth calculation
    gl_Position = lightSpaceMatrix * vec4(aPos, 1.0);
}
