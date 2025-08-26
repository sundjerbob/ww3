/*
 * LIGHTING VERTEX SHADER - 3D Transformation Pipeline with Lighting and Shadow Support
 * 
 * PURPOSE:
 * Transforms vertex positions from object space to clip space using MVP matrices.
 * Calculates and passes lighting-relevant data and shadow coordinates to fragment shader.
 * 
 * FEATURES:
 * - Position transformation (MVP pipeline)
 * - Normal transformation for lighting calculations
 * - World space position and normal output
 * - Shadow coordinate calculation for shadow mapping
 * - Support for multiple light types
 * 
 * MATHEMATICAL PROCESS:
 * 1. Input: Vertex position (aPos) and normal (aNormal) in object space
 * 2. Transform position: MVP * position
 * 3. Transform normal: (M^-1)^T * normal (normal matrix)
 * 4. Calculate shadow coordinates: lightSpaceMatrix * worldPosition
 * 5. Output: Clip space position, world space position, normal, and shadow coordinates
 */

#version 330 core
layout (location = 0) in vec3 aPos;    // Vertex position attribute
layout (location = 1) in vec3 aNormal; // Vertex normal attribute

// Transformation matrices
uniform mat4 model;      // Object-to-world transformation
uniform mat4 view;       // World-to-camera transformation  
uniform mat4 projection; // Camera-to-clip transformation
uniform mat3 normalMatrix; // Normal transformation matrix

// Light space matrices for shadow mapping
uniform mat4 lightSpaceMatrix[4]; // Support up to 4 light sources
uniform int numLightSpaceMatrices; // Number of active light space matrices

// Output to fragment shader
out vec3 FragPos;        // World space position
out vec3 Normal;         // World space normal
out vec4 ShadowCoords[4]; // Shadow coordinates for each light

void main()
{
    // Transform position to world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal to world space using normal matrix
    Normal = normalMatrix * aNormal;
    
    // Transform to clip space
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
    // Calculate shadow coordinates for each light
    for (int i = 0; i < numLightSpaceMatrices && i < 4; i++) {
        ShadowCoords[i] = lightSpaceMatrix[i] * vec4(FragPos, 1.0);
    }
}
