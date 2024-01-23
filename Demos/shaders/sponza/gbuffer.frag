#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef DIFFUSE_MAP
layout(binding = 0) uniform sampler2D diffuseMap;
#else
uniform vec3 diffuseColor;
#endif

#ifdef NORMAL_MAP
layout(binding = 1) uniform sampler2D normalMap;
#endif

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gNormal;

void main() {
    vec4 albedo = vec4(0,0,0,1);
#ifdef DIFFUSE_MAP
    albedo = texture(diffuseMap, fragTex);
#else
    albedo.rgb = diffuseColor;
#endif

#ifdef ALPHA_TEST
    alphaDiscard(albedo.a);
#endif

    gDiffuse = vec4(albedo.rgb, 1.f);
    gNormal = vec4(normalize(fragNor), 1.f);
}  