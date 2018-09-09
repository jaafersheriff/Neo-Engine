#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

out vec4 color;

void main() {
    vec4 textureColor = texture(inputFBO, fragTex);
    color = vec4(vec3(textureColor.r + textureColor.g + textureColor.b) / 3.f, 1.f);
}