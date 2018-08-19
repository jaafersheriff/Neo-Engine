#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P, V, M;
uniform mat3 N;

uniform vec3 snowAngle;
uniform vec3 snowColor;
uniform float snowSize;
uniform float height;

out vec4 fragPos;
out vec3 fragNor;

void main() {
    vec3 vert = vertPos;
    if (dot(vertNor, snowAngle) >= snowSize) {
        vert += vertNor * height;
    }
    fragPos = M * vec4(vert, 1.0);
    fragNor = N * vertNor;
    gl_Position = P * V * fragPos;
}