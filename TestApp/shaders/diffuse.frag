#version 330 core

in vec3 fragPos;

uniform bool useOutline;

out vec4 color;

void main() {
    if (useOutline) {
        color = vec4(0.5, 0.5, 0.5, 1.0);
    }
    else {
        color = vec4(fragPos,1);
    }
}