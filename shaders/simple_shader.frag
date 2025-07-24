#version 450

// Inputs from vertex shader
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec3 worldNormal;

// Output to framebuffer
layout(location = 0) out vec4 outColor;

// Point light definition
struct PointLight 
{
    vec4 position; // xyz = light position, w = unused
    vec4 color;    // rgb = color, a = intensity
};

// Global uniform buffer (scene-wide data)
layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;     // rgb = color, a = intensity
    PointLight pointLights[10];
    int numLights;
} ubo;

// Push constants (per-object data)
layout(push_constant) uniform PushConstants 
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() 
{
    // Compute ambient lighting component
    vec3 ambient = ubo.ambientLightColor.rgb * ubo.ambientLightColor.a;

    // Initialize diffuse and specular lighting
    vec3 diffuse = ambient;
    vec3 specular = vec3(0.0);

    // Normalize the surface normal in world space
    vec3 normal = normalize(worldNormal);

    // Reconstruct camera position from inverse view matrix
    vec3 cameraPos = ubo.invView[3].xyz;

    // Direction from fragment to camera
    vec3 viewDir = normalize(cameraPos - worldPos);

    // Loop through all active point lights
    for (int i = 0; i < ubo.numLights; ++i) 
    {
        PointLight light = ubo.pointLights[i];

        // Vector from fragment to light
        vec3 lightDir = light.position.xyz - worldPos;
        float distanceSq = dot(lightDir, lightDir);
        lightDir = normalize(lightDir);

        // Attenuation factor (inverse-square falloff)
        float attenuation = 1.0 / distanceSq;

        // Compute diffuse intensity
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 lightIntensity = light.color.rgb * light.color.a * attenuation;
        diffuse += lightIntensity * NdotL;

        // Compute Blinn-Phong specular term
        vec3 halfVector = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfVector), 0.0);
        float shininess = 512.0; // High shininess = tight specular highlight
        float specFactor = pow(NdotH, shininess);
        specular += lightIntensity * specFactor;
    }

    // Combine lighting contributions with the fragment's base color
    vec3 finalColor = diffuse * inColor + specular * inColor;
    outColor = vec4(finalColor, 1.0);
}

