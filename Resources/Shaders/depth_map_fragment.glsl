/*
 * DEPTH MAP FRAGMENT SHADER - Shadow Mapping Depth Output
 * 
 * PURPOSE:
 * Outputs depth values for shadow map generation.
 * This shader is intentionally empty as depth is automatically written to depth buffer.
 * 
 * FEATURES:
 * - Automatic depth output
 * - No color calculations needed
 * - Optimized for depth-only rendering
 */

#version 330 core

void main()
{
    // Fragment shader is intentionally empty
    // Depth is automatically written to the depth buffer
    // This is the most efficient way to generate depth maps
}
