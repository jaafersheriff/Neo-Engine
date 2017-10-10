#version 330 core

layout(location = 0) in vec4 vertexPos;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexture;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 fragNormal;
out vec4 worldPos;
out vec2 pass_textureCoords;

void main() {
   vec4 worldPos = M * vertexPos;
   gl_Position = P * V * worldPos;
   fragNormal = vec3(M * vec4(vertexNormal, 0.0));
   pass_textureCoords = vertexTexture;
}
