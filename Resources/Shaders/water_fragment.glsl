#version 330 core

in vec4 clipSpace;
in vec2 TexCoord;
in vec3 toCameraVector;
in vec3 fromLightVector;
in vec3 worldPosition;

out vec4 FragColor;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture; 
uniform sampler2D duDvTexture; 
uniform sampler2D normalMap; 
uniform sampler2D depthMap; 

uniform float time;
uniform float moveFactor;
uniform float distortionScale;
uniform float shineDamper;
uniform float reflectivity;
uniform float waterHeight;

// Water parameters
const float WAVE_SPEED = 0.03;
const float SHINE_DAMPER = 20.0;
const float REFLECTIVITY = 0.6;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 waterColor = vec3(0.0, 0.3, 0.5);

void main()
{
    // Calculate texture coordinates for reflection/refraction
    vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;
    vec2 reflectionTexCoord = vec2(ndc.x, -ndc.y);
    vec2 refractionTexCoord = vec2(ndc.x, ndc.y);

    // Calculate depth for transparency
    float near = 0.01;
    float far = 1000.0;
    float depth = texture(depthMap, refractionTexCoord).r;
    float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    depth = gl_FragCoord.z;
    float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

    float waterDepth = floorDistance - waterDistance;

    // Apply distortion using DuDv map
    vec2 distortedTexCoords = texture(duDvTexture, vec2(TexCoord.x + moveFactor, TexCoord.y)).rg * 0.1;
    distortedTexCoords = TexCoord + vec2(distortedTexCoords.x, TexCoord.y + moveFactor);
    vec2 totalDistortion = (texture(duDvTexture, distortedTexCoords).rg * 2.0 - 1.0) * distortionScale;
    
    // Apply distortion to reflection and refraction coordinates
    reflectionTexCoord += totalDistortion;
    reflectionTexCoord.x = clamp(reflectionTexCoord.x, 0.001, 0.999);
    reflectionTexCoord.y = clamp(reflectionTexCoord.y, -0.999, -0.001);

    refractionTexCoord += totalDistortion;
    refractionTexCoord = clamp(refractionTexCoord, 0.001, 0.999);

    // Sample reflection and refraction textures
    vec4 reflectColor = texture(reflectionTexture, reflectionTexCoord);
    vec4 refractColor = texture(refractionTexture, refractionTexCoord);

    // Sample normal map and calculate normal
    vec4 normalMapColor = texture(normalMap, distortedTexCoords);
    vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
    normal = normalize(normal);

    // Calculate Fresnel effect (reflection vs refraction based on view angle)
    vec3 viewVector = normalize(toCameraVector);
    float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
    refractiveFactor = pow(refractiveFactor, 0.75);

    // Calculate specular highlights
    vec3 reflectedLight = reflect(normalize(fromLightVector), normal);
    float specular = max(dot(reflectedLight, viewVector), 0.0);
    specular = pow(specular, shineDamper);
    vec3 specularHighlights = lightColor * specular * reflectivity * clamp(waterDepth / 0.05, 0.0, 1.0);

    // Combine reflection and refraction
    FragColor = mix(reflectColor, refractColor, refractiveFactor);
    
    // Add water color tint and specular highlights
    FragColor = mix(FragColor, vec4(waterColor, 1.0), 0.2) + vec4(specularHighlights, 0.0);
    
    // Set alpha based on water depth
    FragColor.a = clamp(waterDepth / 0.05, 0.0, 1.0);
}
