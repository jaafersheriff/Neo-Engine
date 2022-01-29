
#include "postprocess.glsl"

out vec4 color;

void main() {
    float depthCol = texture(inputDepth, fragTex).r;
    color = vec4(vec3(depthCol), 1.f);
}