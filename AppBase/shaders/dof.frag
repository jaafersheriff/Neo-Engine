
#include "postprocess.glsl"

out vec4 color;

void main() {
    vec4 inColor = texture(inputFBO, fragTex);
    float inDepth = texture(inputDepth, fragTex).r;

    color.rgb = inColor.rgb;
    color.a = 1.f;
}