#version 450

// Input from vertex shader
layout(location = 0) in vec2 fragOffset;

// Final output color
layout(location = 0) out vec4 outColor;

// Light data structure
struct PointLight 
{
    vec4 position; // xyz = world-space position
    vec4 color;    // rgb = color, a = intensity
};

// Global uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor; // rgb + intensity
    PointLight pointLights[10];
    int numLights;
} ubo;

// Push constants per light quad
layout(push_constant) uniform PushConstants 
{
    vec4 position;  // xyz = world position
    vec4 color;     // rgb + intensity
    float radius;   // visual radius
} push;

// Constant for cosine falloff
const float PI = 3.14159265;

void main() 
{
    // Compute distance from quad center (in screen-space)
    float offsetDist = length(fragOffset);

    // Discard fragments outside the unit circle (soft quad boundary)
    if (offsetDist >= 1.0) {
        discard;
    }

    // Cosine-based smooth radial falloff (range: 1.0 to 0.0)
    float falloff = 0.5 * (cos(offsetDist * PI) + 1.0);

    // Light visualization color with subtle intensity boost
    vec3 lightColor = push.color.rgb + 0.5 * falloff;

    outColor = vec4(lightColor, falloff);
}

