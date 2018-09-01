#version 330 core

in vec2 fragTex;

out vec4 color;

void main() {
    color = vec4(normalize(vec3(fragTex,0)),1);
}