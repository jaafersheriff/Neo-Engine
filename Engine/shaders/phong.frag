#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#if defined(ALPHA_TEST) && defined(ALPHA_MAP)
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

#ifdef ENABLE_SHADOWS
in vec4 shadowCoord;
layout(binding = 4) uniform sampler2D shadowMap;
#endif

uniform vec3 lightCol;
#if defined(DIRECTIONAL_LIGHT) || defined(ENABLE_SHADOWS)
uniform vec3 lightDir;
#elif defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform vec3 lightAtt;
#endif

uniform vec3 ambientColor;

uniform vec3 camPos;

out vec4 color;

void main() {
    vec4 albedo = vec4(0,0,0,1);
#ifdef DIFFUSE_MAP
    albedo = texture(diffuseMap, fragTex);
#else
    albedo.rgb = diffuseColor;
#endif

    vec3 specular = vec3(0.0);
#ifdef SPECULAR_MAP
    specular = texture(specularMap, fragTex);
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

    // TODO - normal mapping
    vec3 N = normalize(fragNor);
    vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
#ifdef DIRECTIONAL_LIGHT
    vec3 L = normalize(lightDir);
#elif defined(POINT_LIGHT)
    vec3 lightDir = lightPos - fragPos.xyz;
    vec3 L = normalize(lightDir);
    float lightDistance = length(lightDir);
    if (length(lightAtt) > 0) {
        attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
    }
#else
    vec3 L = vec3(0, 0, 0);
#endif

    color.rgb = getPhong(V, N, L, ambientColor, albedo.rgb, specular, shine, lightCol, attFactor);

#ifdef ENABLE_SHADOWS
    float visibility = max(getShadowVisibility(1, shadowMap, shadowCoord, 0.002), 0.2);
    color.rgb *= visibility;
#endif

    color.a = 1.0;
}

