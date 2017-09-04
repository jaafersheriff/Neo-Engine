#version 400 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 mvp;

out vec3 col;

void main() {
   gl_Position = mvp * vec4(vPos, 0.0, 1.0);
   col = vCol;
}
