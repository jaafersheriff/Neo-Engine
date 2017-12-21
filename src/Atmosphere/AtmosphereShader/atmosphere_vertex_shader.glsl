#version 330 core

layout(location = 0) in vec3 vertexPos;

uniform mat4 P;
uniform mat4 V;

out vec3 worldPos;

void main() {
    worldPos = vertexPos;   // TODO : model matrix
    gl_Position = P * V * vec4(vertexPos, 1.0);
}