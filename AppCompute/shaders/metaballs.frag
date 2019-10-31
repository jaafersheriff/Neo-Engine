#include "phong.glsl"

in vec3 fragPos;
in vec3 fragNor;

out vec4 color;

uniform bool wf;
uniform vec3 lightPos;

void main() {
    if (wf) {
        color = vec4(1);
    }
    else {
        vec3 c = normalize(fragNor);
        color = vec4(
            c
            , 1.0);
    }
}