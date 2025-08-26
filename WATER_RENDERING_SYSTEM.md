# Water Rendering System for WW3 Engine

## Overview

This document describes the implementation of a realistic water rendering system for the WW3 engine, adapted from the CProceduralGame water system. The system provides reflection, refraction, wave animation, and realistic water effects.

## Architecture

### Core Components

1. **WaterRenderer** (`Source/Engine/Rendering/WaterRenderer.h/cpp`)
   - Specialized renderer for water effects
   - Handles reflection and refraction framebuffers
   - Manages water textures and shaders
   - Provides wave animation and distortion effects

2. **Water GameObject** (`Source/GameObjects/Water.h/cpp`)
   - Represents a water surface in the scene
   - Configurable water parameters
   - Integrates with the renderer factory system

3. **Water Shaders** (`Resources/Shaders/water_vertex.glsl`, `water_fragment.glsl`)
   - Vertex shader: Wave animation and data preparation
   - Fragment shader: Reflection, refraction, and lighting effects

4. **Water Textures** (`Resources/Images/`)
   - `water_du_dv.png`: DuDv map for wave distortion
   - `water_normals.png`: Normal map for surface detail

## Features

### Reflection and Refraction
- **Reflection Pass**: Renders scene to texture from water surface perspective
- **Refraction Pass**: Renders scene normally for underwater view
- **Fresnel Effect**: Blends reflection/refraction based on view angle
- **Depth-based Transparency**: Realistic water depth calculation

### Wave Animation
- **Sine Wave Animation**: Time-based wave movement
- **DuDv Distortion**: Texture-based wave distortion
- **Normal Mapping**: Surface detail for realistic lighting
- **Configurable Parameters**: Wave speed, amplitude, frequency

### Water Parameters
- **Wave Speed**: Controls animation speed (default: 0.03)
- **Distortion Scale**: Wave distortion intensity (default: 0.01)
- **Shine Damper**: Specular highlight sharpness (default: 20.0)
- **Reflectivity**: Reflection strength (default: 0.6)
- **Water Height**: Surface elevation in world space

## Integration

### Renderer Factory
The water renderer is integrated into the existing renderer factory system:

```cpp
enum class RendererType {
    // ... existing types ...
    Water,      // For water rendering with reflection/refraction
    // ... other types ...
};
```

### Game Loop Integration
Water rendering is integrated into the main game loop with multiple render passes:

1. **Reflection Pass**: Render scene to reflection framebuffer
2. **Refraction Pass**: Render scene to refraction framebuffer  
3. **Main Pass**: Render scene with water effects

### Scene Setup
Water surfaces are added to the scene like other GameObjects:

```cpp
auto waterSurface = std::make_unique<Water>("WaterSurface", -5.0f);
waterSurface->setPosition(Vec3(0.0f, -5.0f, 0.0f));
waterSurface->setScale(Vec3(2.0f, 1.0f, 2.0f));
scene->addGameObject(std::move(waterSurface));
```

## Usage

### Controls
- **W Key**: Display water statistics and parameters
- **T Key**: Display terrain statistics (existing)

### Configuration
Water parameters can be adjusted at runtime:

```cpp
waterSurface->setWaveSpeed(0.05f);        // Faster waves
waterSurface->setDistortionScale(0.02f);  // More distortion
waterSurface->setShineDamper(15.0f);      // Softer shine
waterSurface->setReflectivity(0.7f);      // More reflective
```

## Technical Details

### Framebuffers
- **Reflection FBO**: Captures scene reflection
- **Refraction FBO**: Captures scene refraction with depth
- **Texture Management**: Automatic cleanup and resizing

### Shader Uniforms
- `time`: Current time for animation
- `moveFactor`: Wave movement factor
- `distortionScale`: Wave distortion intensity
- `shineDamper`: Specular highlight control
- `reflectivity`: Reflection strength
- `waterHeight`: Water surface height

### Texture Bindings
- `duDvTexture` (GL_TEXTURE0): Wave distortion map
- `normalMap` (GL_TEXTURE1): Surface normal map
- `reflectionTexture` (GL_TEXTURE2): Reflection render target
- `refractionTexture` (GL_TEXTURE3): Refraction render target
- `depthMap` (GL_TEXTURE4): Depth information

## Performance Considerations

### Optimization
- **Framebuffer Resolution**: Matches window size for quality
- **Mesh Resolution**: 64x64 vertices for smooth water surface
- **Texture Compression**: Uses standard OpenGL texture formats
- **Shader Efficiency**: Optimized for real-time rendering

### Memory Usage
- **Framebuffers**: 2 color textures + 1 depth texture
- **Water Textures**: 2 additional textures (DuDv + Normal)
- **Mesh Data**: ~4KB for water surface geometry

## Future Enhancements

### Planned Features
1. **Dynamic Water Physics**: Real-time wave simulation
2. **Caustics**: Underwater light patterns
3. **Foam Effects**: Wave crest foam rendering
4. **Water Interaction**: Object-water collision effects
5. **Multiple Water Bodies**: Support for lakes, rivers, oceans

### Advanced Effects
1. **Screen Space Reflections**: More accurate reflections
2. **Planar Reflections**: Optimized for flat water surfaces
3. **Water Particles**: Splash and spray effects
4. **Underwater Fog**: Depth-based visibility

## Troubleshooting

### Common Issues
1. **Missing Textures**: Ensure water textures are in `Resources/Images/`
2. **Shader Compilation**: Check shader file paths and syntax
3. **Framebuffer Errors**: Verify OpenGL version supports required features
4. **Performance Issues**: Reduce water mesh resolution or disable effects

### Debug Information
Use the **W key** to display water statistics and verify system operation.

## Dependencies

### External Libraries
- **stb_image**: Image loading for water textures
- **OpenGL**: Core rendering functionality
- **GLFW**: Window and context management

### Internal Dependencies
- **Renderer Factory**: Renderer management system
- **Shader System**: Shader compilation and management
- **Mesh System**: Geometry handling
- **Math Library**: Matrix and vector operations

## Conclusion

The water rendering system provides a solid foundation for realistic water effects in the WW3 engine. The modular design allows for easy integration and future enhancements while maintaining good performance characteristics.
