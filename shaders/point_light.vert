#version 450

// Predefined screen-space offsets to form a quad
const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

// Output to fragment shader
layout(location = 0) out vec2 fragOffset;

// Point light data structure
struct PointLight 
{
    vec4 position; // xyz = world position
    vec4 color;    // rgb = color, a = intensity
};

// Global uniform buffer
layout(set = 0, binding = 0) uniform GlobalUbo 
{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor; // rgb + intensity (w)
    PointLight pointLights[10];
    int numLights;
} ubo;

// Push constants for individual light
layout(push_constant) uniform PushConstants 
{
    vec4 position;  // xyz = world position
    vec4 color;     // rgb + intensity
    float radius;   // light radius
} push;

void main() 
{
    // Send vertex offset to fragment shader for screen-space calculations
    fragOffset = OFFSETS[gl_VertexIndex];

    // Extract right and up vectors from view matrix (camera orientation)
    vec3 cameraRight = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 cameraUp    = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    // Compute the quad vertex world position based on offset and radius
    vec3 worldPos = push.position.xyz 
                    + push.radius * fragOffset.x * cameraRight 
                    + push.radius * fragOffset.y * cameraUp;

    // Transform to clip space
    gl_Position = ubo.projection * ubo.view * vec4(worldPos, 1.0);
}
