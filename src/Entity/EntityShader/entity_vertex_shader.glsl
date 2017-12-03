#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexture;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec4 worldPos;
out vec3 fragNormal;
out vec4 viewDir;
out vec2 textureCoords;

void main() {
    worldPos = M * vertexPos;
    gl_Position = P * V * worldPos;

    viewDir = (inverse(V) * vec4(0.0, 0.0, 0.0, 1.0)) - worldPos;
    fragNormal = vec3(M * vec4(vertexNormal, 0.0));
    textureCoords = vertexTexture;
}
