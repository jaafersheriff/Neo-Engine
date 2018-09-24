#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform int blurAmount;

out vec4 color;

void main() {
    color = texture(inputFBO, fragTex);
    color.a= 1.f;
}