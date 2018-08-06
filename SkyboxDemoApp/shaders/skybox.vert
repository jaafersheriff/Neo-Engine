#version 330 core

layout (location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;

out vec3 fragTex;

void main() {
    mat4 skyV = V;
    skyV[3][0] = skyV[3][1] = skyV[3][2] = 0.0;
    gl_Position = P * skyV * vec4(vertPos, 1.0); 
    fragTex = vertPos;
}  