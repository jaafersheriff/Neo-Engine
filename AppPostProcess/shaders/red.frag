#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;

out vec4 color;

void main() {
    color = texture(inputFBO, fragTex);
    color.r += 0.5f;
}