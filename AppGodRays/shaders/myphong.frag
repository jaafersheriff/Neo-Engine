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
uniform vec3 shine;
uniform sampler2D ambientMap;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;

out vec4 color;

void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
}