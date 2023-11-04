#include "phongLighting.glsl"
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef DIFFUSE_MAP
uniform sampler2D diffuseMap;
#else
uniform vec3 diffuseColor;
#endif

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform float shine;
uniform vec3 camPos;
uniform vec3 lightPos;
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

    color.rgb = albedo.rgb * ambientColor + 
                getPhong(fragNor, fragPos.rgb, camPos, lightPos - fragPos.xyz, lightAtt, lightCol, albedo.rgb, specularColor, shine);
    color.a = albedo.a;
}