#version 330 core

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform bool useNormalMap;
uniform sampler2D normalMap;

uniform bool useDiffuseMap;
uniform sampler2D diffuseMap;
uniform vec3 diffuseMaterial;

uniform bool useSpecularMap;
uniform sampler2D specularMap;
uniform vec3 specularMaterial;
uniform float shine;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gSpecular;

void main() {
    gPosition = vec4(fragPos.xyz, 1);
    gNormal = vec4(useNormalMap ? texture(normalMap, fragTex).rgb : normalize(fragNor), 1.f);
    gDiffuse = vec4(useDiffuseMap ? texture(diffuseMap, fragTex).rgb : diffuseMaterial, 1.f);
    gSpecular = useSpecularMap ? texture(specularMap, fragTex) : vec4(specularMaterial, shine);
}  