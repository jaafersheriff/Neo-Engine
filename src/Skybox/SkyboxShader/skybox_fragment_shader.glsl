#version 330 core

in vec3 textureCoords;

uniform samplerCube cubeMap;

out vec4 color;

void main() {
    color = texture(cubeMap, textureCoords);
}
