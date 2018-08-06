#version 330 core

in vec3 fragTex;

uniform samplerCube cube;

out vec4 color;

void main() {
    color = texture(cube, fragTex);
}