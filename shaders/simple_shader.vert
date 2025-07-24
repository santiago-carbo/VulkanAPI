#version 450

// Input vertex attributes (from vertex buffer)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV; // UV is unused but preserved

// Outputs to fragment shader
layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 worldPosition;
layout(location = 2) out vec3 worldNormal;

// Light data structure
struct PointLight 
{
    vec4 position; // xyz = position, w = unused
    vec4 color;    // rgb = color, a = intensity or unused
};

// Global uniform buffer object (shared data)
layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

// Push constants (object-specific data)
layout(push_constant) uniform PushConstants 
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() 
{
    // Transform vertex position to world space
    vec4 worldPos = push.modelMatrix * vec4(inPosition, 1.0);
    
    // Final vertex position in clip space
    gl_Position = ubo.projection * ubo.view * worldPos;

    // Pass transformed world-space normal to fragment shader
    worldNormal = normalize(mat3(push.normalMatrix) * inNormal);

    // Pass world-space position and vertex color to fragment shader
    worldPosition = worldPos.xyz;
    outColor = inColor;
}

