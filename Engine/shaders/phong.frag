#include "phongLighting.glsl"
#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"

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

#ifdef ENABLE_SHADOWS
in vec4 shadowCoord;
uniform vec3 lightDir;
layout(binding = 2) uniform sampler2D shadowMap;
#else
uniform vec3 lightPos;
#endif

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float shine;
uniform vec3 camPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

out vec4 color;

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

    // TODO - normal mapping
    vec3 nor = fragNor;

    float visibility = 1.0;
#ifdef ENABLE_SHADOWS
    visibility = max(getShadowVisibility(1, shadowMap, shadowCoord, 0.002), 0.2);
#endif

#ifdef ENABLE_SHADOWS
#else
    vec3 lightDir = lightPos - fragPos.xyz;
#endif

    color.rgb = albedo.rgb * ambientColor +
        getPhong(nor, fragPos.rgb, camPos, lightDir, lightAtt, lightCol, albedo.rgb, specularColor, shine)
        * visibility;
    color.a = 1.0;
}
