#version 330 core

in vec3 fragPos;

out vec4 color;

void main() {
    color = vec4(fragPos,1);
}