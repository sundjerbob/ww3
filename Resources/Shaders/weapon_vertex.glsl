/*
 * WEAPON VERTEX SHADER - 3D Transformation Pipeline with Texture Support
 * 
 * PURPOSE:
 * Transforms vertex positions from object space to clip space using MVP matrices.
 * Supports texture coordinates for weapon material rendering.
 * 
 * MATHEMATICAL PROCESS:
 * 1. Input: Vertex position in object space (aPos) and texture coordinates (aTexCoord)
 * 2. Convert to homogeneous coordinates: vec4(aPos, 1.0)
 * 3. Apply Model transformation: model * vertex
 * 4. Apply View transformation: view * (model * vertex)  
 * 5. Apply Projection: projection * (view * model * vertex)
 * 6. Output: gl_Position in clip space coordinates and TexCoord for fragment shader
 * 
 * MATRIX MULTIPLICATION ORDER:
 * projection * view * model * vertex
 * Right-to-left application: model first, then view, then projection
 * 
 * COORDINATE TRANSFORMATIONS:
 * Object Space → World Space → Camera Space → Clip Space
 */

#version 330 core
layout (location = 0) in vec3 aPos;        // Vertex position attribute from VBO
layout (location = 1) in vec2 aTexCoord;   // Texture coordinate attribute from VBO

// Transformation matrices (uniforms are constant for all vertices in a draw call)
uniform mat4 model;      // Object-to-world transformation
uniform mat4 view;       // World-to-camera transformation  
uniform mat4 projection; // Camera-to-clip transformation

out vec3 outPos;         // World position for fragment shader
out vec2 TexCoord;       // Texture coordinates for fragment shader

void main()
{
    // Apply complete MVP transformation pipeline
    // Note: Matrix multiplication is right-associative in GLSL
    
    // Calculate world position for fragment shader color logic
    vec4 worldPos = model * vec4(aPos, 1.0);
    outPos = worldPos.xyz;
    
    // Pass texture coordinates to fragment shader
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * worldPos;
}
