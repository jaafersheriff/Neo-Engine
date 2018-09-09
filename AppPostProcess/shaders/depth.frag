#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

out vec4 color;

void main() {
    float depthCol = texture(inputDepth, fragTex).r;
    color = vec4(vec3(depthCol), 1.f);
}