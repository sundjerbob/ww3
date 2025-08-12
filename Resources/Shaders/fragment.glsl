/*
 * FRAGMENT SHADER - Pixel Color Generation
 * 
 * PURPOSE:
 * Determines the final color of each pixel (fragment) on the rendered surface.
 * This is the final programmable stage before the fragment is written to the frame buffer.
 * 
 * MATHEMATICAL PROCESS:
 * 1. Input: Interpolated values from vertex shader (position, if any)
 * 2. Calculate: Fragment color based on lighting, materials, textures, etc.
 * 3. Output: Final RGBA color value for this pixel
 * 
 * COLOR REPRESENTATION:
 * RGBA values in range [0.0, 1.0] where:
 * - R (Red): 0.8 = 80% red intensity
 * - G (Green): 0.4 = 40% green intensity  
 * - B (Blue): 0.2 = 20% blue intensity
 * - A (Alpha): 1.0 = 100% opacity (no transparency)
 * 
 * GPU PROCESSING:
 * After fragment shader, GPU performs:
 * - Depth testing (Z-buffer comparison)
 * - Alpha blending (if transparency is used)
 * - Frame buffer writing (final pixel color storage)
 * 
 * RENDERING PIPELINE POSITION:
 * Vertex Shader → Rasterization → Fragment Shader → Frame Buffer
 */

#version 330 core
in vec3 outPos;     // Position from vertex shader
out vec4 FragColor; // Output color for this fragment/pixel

void main()
{
    // Dynamic color based on world position:
    // Ground plane (y <= 0): Green color for floor/ground
    // Objects above ground (y > 0): Orange color for cube and other objects
    
    if (outPos.y <= -1.0)
        FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green for ground
    else     
        FragColor = vec4(0.8f, 0.4f, 0.2f, 1.0f); // Orange for objects above ground
}