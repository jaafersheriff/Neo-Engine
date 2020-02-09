
#include "postprocess.glsl"

out vec4 color;

void main() {
    color = vec4(texture(inputFBO, fragTex).rgb, 1.0);
}
