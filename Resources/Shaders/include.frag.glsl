#define MAX_LIGHTS 100
struct DirectionalLight{
    vec3 direction;
    float intensity;
    vec4 color;
};
struct AmbientLight{
    float intensity;
    vec4 color;
};
struct PointLight{
    vec3 position;
    vec4 color;
    float intensity;
    float range;
};

struct SpotLight{
    vec3 position;
    vec3 direction;
    vec4 color;
    float intensity;
    float range;
    float angle;
};
//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inPos;
