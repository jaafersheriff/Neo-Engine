#version 330 core

in vec3 fragTex;

uniform samplerCube cubeMap;

out vec4 color;

void main() {
    color = texture(cubeMap, fragTex);
}