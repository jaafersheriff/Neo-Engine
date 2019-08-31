
#include "postprocess.glsl"

uniform float gamma;

out vec4 color;

void main() {
    color = texture(inputFBO, fragTex);
    color.rgb = pow(color.rgb, vec3(1/gamma));
}