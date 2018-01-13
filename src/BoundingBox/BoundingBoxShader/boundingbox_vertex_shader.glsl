#version 330 

layout(location = 0) in vec3 vertexPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform float size;

void main() {
    gl_Position = P * V * M * vec4(vertexPos, 1.0);
}
