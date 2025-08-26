#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

out vec4 clipSpace;
out vec2 TexCoord;
out vec3 toCameraVector;
out vec3 fromLightVector;
out vec3 worldPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;
uniform float time;
uniform float waterHeight;

// Light direction (can be made uniform later)
vec3 lightDir = normalize(vec3(-1.0, -1.0, -1.0));

// Water parameters
const float tiling = 8.0;
const float waveAmplitude = 0.8;
const float waveFrequency = 0.2;

void main() 
{
    // First calculate the base world position without wave animation
    vec3 baseWorldPosition = vec3(model * vec4(position, 1.0));
    
    // Apply water height offset
    baseWorldPosition.y += waterHeight;
    
    // Apply wave animation in world space (using world X coordinate)
    float wave = sin(baseWorldPosition.x * waveFrequency + time) * waveAmplitude;
    worldPosition = baseWorldPosition;
    worldPosition.y += wave;
    
    // Calculate clip space position
    clipSpace = projection * view * vec4(worldPosition, 1.0);
    gl_Position = clipSpace;

    // Apply tiling to texture coordinates
    TexCoord = texCoord * tiling;
    
    // Calculate vectors for lighting and reflection
    toCameraVector = cameraPosition - worldPosition;
    fromLightVector = worldPosition - lightDir;
}
