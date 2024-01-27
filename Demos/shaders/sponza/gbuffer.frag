#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef ALPHA_MAP
layout(binding = 0) uniform sampler2D alphaMap;
#endif

#ifdef DIFFUSE_MAP
layout(binding = 1) uniform sampler2D diffuseMap;
#else
uniform vec3 diffuseColor;
#endif

#ifdef SPECULAR_MAP
layout(binding = 2) uniform sampler2D specularMap;
#else
uniform vec3 specularColor;
#endif
uniform float shine;

#ifdef NORMAL_MAP
layout(binding = 3) uniform sampler2D normalMap;
#endif

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gSpecular;
layout (location = 2) out vec4 gWorld;
layout (location = 3) out vec4 gNormal;

void main() {
    vec4 albedo = vec4(0,0,0,1);
#ifdef DIFFUSE_MAP
    albedo = texture(diffuseMap, fragTex);
#else
    albedo.rgb = diffuseColor;
#endif

    vec3 specular= vec3(0.0);
#ifdef SPECULAR_MAP
    specular = texture(specularMap, fragTex).rgb;
#else
    specular = specularColor;
#endif


#ifdef ALPHA_TEST
    #ifdef ALPHA_MAP
    alphaDiscard(texture(alphaMap, fragTex).r);
    #else
    alphaDiscard(albedo.a);
    #endif
#endif

    gDiffuse = vec4(albedo.rgb, 1.f);
    gSpecular = vec4(specular, shine);
    gWorld = vec4(fragPos.rgb, 1.f);
    gNormal = vec4(normalize(fragNor) * 0.5 + 0.5, 1.f);
}  