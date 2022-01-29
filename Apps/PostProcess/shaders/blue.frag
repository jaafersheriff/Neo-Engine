
#include "postprocess.glsl"

out vec4 color;

void main() {
    color = texture(inputFBO, fragTex);
    color.b += 0.5f;
}