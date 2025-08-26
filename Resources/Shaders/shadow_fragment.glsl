/*
 * SHADOW FRAGMENT SHADER - Lighting with Shadow Mapping
 * 
 * PURPOSE:
 * Calculates final fragment color with lighting and shadow mapping.
 * Combines ambient, diffuse, and specular lighting with shadow calculations.
 * 
 * FEATURES:
 * - Phong lighting model
 * - Shadow mapping with PCF filtering
 * - Support for multiple light sources
 * - Shadow bias to prevent shadow acne
 */

#version 330 core
in vec3 FragPos;        // World space position
in vec3 Normal;         // World space normal
in vec4 ShadowCoords[4]; // Shadow coordinates for each light

out vec4 FragColor; // Final fragment color

// Material properties
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// Light properties
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool isEnabled;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
    bool isEnabled;
};

struct AmbientLight {
    vec3 color;
    float intensity;
    bool isEnabled;
};

// Uniforms
uniform Material material;
uniform vec3 viewPos;  // Camera position

// Light arrays
uniform DirectionalLight directionalLights[4];
uniform PointLight pointLights[16];
uniform AmbientLight ambientLights[4];

// Light counts
uniform int numDirectionalLights;
uniform int numPointLights;
uniform int numAmbientLights;

// Shadow mapping uniforms
uniform sampler2D shadowMap;
uniform float shadowBias;
uniform float shadowBiasMin;
uniform float shadowBiasMax;
uniform int numLightSpaceMatrices;

// Shadow calculation function
float calculateShadow(vec4 fragPosLightSpace, int lightIndex) {
    if (lightIndex >= numLightSpaceMatrices) return 0.0;
    
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light frustum
    if (projCoords.z > 1.0) return 0.0;
    
    // Get closest depth from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get current depth
    float currentDepth = projCoords.z;
    
    // Calculate shadow bias based on surface normal
    float bias = max(shadowBiasMax * (1.0 - dot(normalize(Normal), normalize(directionalLights[lightIndex].direction))), shadowBiasMin);
    
    // Check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

// Lighting calculation functions
vec3 calculateAmbientLight(AmbientLight light, Material mat) {
    if (!light.isEnabled) return vec3(0.0);
    return light.color * light.intensity * mat.ambient;
}

vec3 calculateDirectionalLight(DirectionalLight light, Material mat, vec3 normal, vec3 viewDir, int lightIndex) {
    if (!light.isEnabled) return vec3(0.0);
    
    // Ambient
    vec3 ambient = light.color * light.intensity * mat.ambient;
    
    // Diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * diff * mat.diffuse;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = light.color * light.intensity * spec * mat.specular;
    
    // Calculate shadow
    float shadow = calculateShadow(ShadowCoords[lightIndex], lightIndex);
    
    // Combine lighting with shadow
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 calculatePointLight(PointLight light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (!light.isEnabled) return vec3(0.0);
    
    // Ambient
    vec3 ambient = light.color * light.intensity * mat.ambient;
    
    // Diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color * light.intensity * diff * mat.diffuse;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = light.color * light.intensity * spec * mat.specular;
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main()
{
    // Normalize normal vector
    vec3 norm = normalize(Normal);
    
    // Calculate view direction
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Initialize result color
    vec3 result = vec3(0.0);
    
    // Calculate ambient lighting
    for (int i = 0; i < numAmbientLights && i < 4; i++) {
        result += calculateAmbientLight(ambientLights[i], material);
    }
    
    // Calculate directional lighting with shadows
    for (int i = 0; i < numDirectionalLights && i < 4; i++) {
        result += calculateDirectionalLight(directionalLights[i], material, norm, viewDir, i);
    }
    
    // Calculate point lighting
    for (int i = 0; i < numPointLights && i < 16; i++) {
        result += calculatePointLight(pointLights[i], material, norm, FragPos, viewDir);
    }
    
    // Output final color
    FragColor = vec4(result, 1.0);
}
