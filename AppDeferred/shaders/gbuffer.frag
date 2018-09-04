#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform bool useNormalMap;
uniform sampler2D normalMap;

uniform bool useDiffuseMap;
uniform sampler2D diffuseMap;
uniform vec3 diffuseMaterial;
uniform float ambient;

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gDiffuse;

void main() {
    gNormal = vec4(0.5 + 0.5 * (useNormalMap ? texture(normalMap, fragTex).rgb : normalize(fragNor)), 1.f);
    gDiffuse = vec4(useDiffuseMap ? texture(diffuseMap, fragTex).rgb : diffuseMaterial, 1.f);
}  