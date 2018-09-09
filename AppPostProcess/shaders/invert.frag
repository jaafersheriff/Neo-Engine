#version 330 core

in vec2 fragTex;

uniform sampler2D inputFBO;
uniform sampler2D inputDepth;

out vec4 color;

void main() {
    vec4 textureColor = texture(inputFBO, fragTex);
    color = vec4(1.0 - textureColor.r,1.0 -textureColor.g,1.0 -textureColor.b,1);
}