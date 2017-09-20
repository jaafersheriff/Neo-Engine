#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexNormal;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 fragNormal;
out vec3 worldPos;

void main() {
   gl_Position = P * V * M * vertexPos;
   fragNormal = normalize((V * M * vec4(vertexNormal, 0.0)).xyz);
   worldPos = vec3(V * M * vertexPos);
}
