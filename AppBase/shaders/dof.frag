
#include "postprocess.glsl"

in vec2 fragColorTex0;
in vec2 fragColorTex1;
in vec2 fragDepthTex0;
in vec2 fragDepthTex1;
in vec2 fragDepthTex2;
in vec2 fragDepthTex3;

out vec4 color;

void main() {
    vec4 inColor = texture(inputFBO, fragTex);
    float inDepth = texture(inputDepth, fragTex).r;

    color.rgb = inColor.rgb;
    color.a = 1.f;
}