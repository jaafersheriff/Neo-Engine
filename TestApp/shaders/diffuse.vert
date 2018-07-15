#version 330 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fragPos;
out vec3 fragNor;

void main() {
    vec4 worldPos = M * vec4(vertPos, 1.0);
    gl_Position = P * V * worldPos;
    fragPos = worldPos.xyz;
    fragNor = (M * vec4(vertNor, 1.0)).xyz;
}