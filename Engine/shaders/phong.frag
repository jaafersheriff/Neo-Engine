#include "phongLighting.glsl"
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
uniform sampler2D diffuseMap;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shine;
uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;
out vec4 color;
void main() {
    vec4 albedo = texture(diffuseMap, fragTex);
    albedo.rgb += diffuseColor;
    alphaDiscard(albedo.a);
    color.rgb = albedo.rgb * ambientColor + 
                getPhong(fragNor, fragPos.rgb, camPos, lightPos - fragPos.xyz, lightAtt, lightCol, albedo.rgb, specularColor, shine);
    color.a = albedo.a;
}