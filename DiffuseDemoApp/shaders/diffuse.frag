#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform sampler2D diffuseMap;
uniform vec3 diffuseContribution;
uniform vec3 specularColor;
uniform float shine;

uniform vec3 camPos;
uniform vec3 lightPos;

out vec4 color;

void main() {
    color = texture(diffuseMap, fragTex);
}