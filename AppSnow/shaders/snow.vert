#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P, V, M;
uniform mat3 N;

out vec4 fragPos;
out vec3 fragNor;

void main() {
    fragPos = M * vec4(vertPos, 1.0);
    fragNor = N * vertNor;
    gl_Position = P * V * fragPos;
}