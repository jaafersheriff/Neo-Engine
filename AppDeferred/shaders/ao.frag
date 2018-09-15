#version 330 core

in vec2 fragTex;

uniform sampler2D gNormal;

out vec4 color;

void main() {
    color = texture(gNormal, fragTex);
}