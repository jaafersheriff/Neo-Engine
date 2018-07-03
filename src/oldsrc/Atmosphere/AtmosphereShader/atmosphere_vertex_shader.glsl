#version 330 core

layout(location = 0) in vec3 vertexPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 vertPos;

void main() {
    vertPos = vertexPos;
    mat4 atmV = V;
    atmV[3][0] = atmV[3][1] = atmV[3][2] = 0.0;
    gl_Position = P * atmV * M * vec4(vertexPos, 1.0);
}