#include "phong.glsl"
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec3 camPos;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform vec3 lightAtt;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shine;
uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

out vec4 color;

void main() {
    vec3 amb = diffuse * texture(ambientMap, fragTex).rgb;
    vec3 dif = texture(diffuseMap, fragTex).rgb;
    vec3 spec = specular * texture(specularMap, fragTex).rgb;
    vec3 n = texture(normalMap, fragTex).rgb; //normalize(fragNor); 
    vec3 phong = getPhong(n, fragPos.rgb, camPos, lightPos, lightAtt, lightCol, dif, spec, shine);

    color = vec4(dif, 1.0);
}