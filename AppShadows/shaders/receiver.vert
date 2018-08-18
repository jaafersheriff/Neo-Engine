#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;

uniform mat4 P, V, M;
uniform mat4 lightP, lightV;
uniform mat3 N;

out vec4 fragPos;
out vec4 fragPosLight;
out vec3 fragNor;
out vec2 fragTex;

void main() {
    fragPos = M * vec4(vertPos, 1.0);
    fragNor = N * vertNor;
    gl_Position = P * V * fragPos;
    fragPosLight = lightP * lightV * fragPos;
    fragTex = vertTex;
}