/*
 * VERTEX SHADER - 3D Transformation Pipeline
 * 
 * PURPOSE:
 * Transforms vertex positions from object space to clip space using MVP matrices.
 * This is the first stage of the graphics pipeline, processing each vertex individually.
 * 
 * MATHEMATICAL PROCESS:
 * 1. Input: Vertex position in object space (aPos)
 * 2. Convert to homogeneous coordinates: vec4(aPos, 1.0)
 * 3. Apply Model transformation: model * vertex
 * 4. Apply View transformation: view * (model * vertex)  
 * 5. Apply Projection: projection * (view * model * vertex)
 * 6. Output: gl_Position in clip space coordinates
 * 
 * MATRIX MULTIPLICATION ORDER:
 * projection * view * model * vertex
 * Right-to-left application: model first, then view, then projection
 * 
 * COORDINATE TRANSFORMATIONS:
 * Object Space → World Space → Camera Space → Clip Space
 * 
 * GPU PROCESSING:
 * After vertex shader, GPU performs perspective divide (x/w, y/w, z/w)
 * and viewport transformation to convert to screen coordinates.
 */

#version 330 core
layout (location = 0) in vec3 aPos; // Vertex position attribute from VBO

// Transformation matrices (uniforms are constant for all vertices in a draw call)
uniform mat4 model;      // Object-to-world transformation
uniform mat4 view;       // World-to-camera transformation  
uniform mat4 projection; // Camera-to-clip transformation
out vec3 outPos;

void main()
{
    // Apply complete MVP transformation pipeline
    // Note: Matrix multiplication is right-associative in GLSL
    
    // Calculate world position for fragment shader color logic
    vec4 worldPos = model * vec4(aPos, 1.0);
    outPos = worldPos.xyz;
    
    gl_Position = projection * view * worldPos;
}