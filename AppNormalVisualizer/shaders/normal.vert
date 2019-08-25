#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

out vec3 fragNor;

void main() {
    gl_Position = P * V * M * vec4(vertPos, 1.0);
    fragNor = vec3(P * vec4(N * vertNor, 0));
}