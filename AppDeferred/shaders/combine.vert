#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 2) in vec2 vertTex;

out vec2 fragTex;

void main() {
    gl_Position = vec4(2.f * vertPos, 1.0);
    fragTex = vertTex;
}