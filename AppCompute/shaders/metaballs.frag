#include "phong.glsl"

in vec3 fragPos;
in vec3 fragNor;

out vec4 color;

uniform bool wf;

void main() {
    if (wf) {
        color = vec4(1);
    }
    else {
        color = vec4(fragPos, 1.0);
    }
}