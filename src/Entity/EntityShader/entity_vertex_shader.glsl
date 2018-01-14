#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexture;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 cameraPos;

out vec4 worldPos;
out vec3 fragNormal;
out vec3 viewDir;
out vec2 textureCoords;

void main() {
    worldPos = M * vertexPos;
    gl_Position = P * V * worldPos;

    viewDir = cameraPos - worldPos.xyz;
    fragNormal = vec3(M * vec4(vertexNormal, 0.0));
    textureCoords = vertexTexture;
}
