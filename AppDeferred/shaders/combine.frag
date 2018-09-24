#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

uniform sampler2D lightOutput;

out vec4 color;

void main() {
    color.rgb = vec3(texture(inputFBO, fragTex).r);
    color.a = 1.f;
}