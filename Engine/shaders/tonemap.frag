#include "color.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D inputTexture;

out vec4 color;

void main() {
    // Reinhard-Jodie
    vec3 v = texture(inputTexture, fragTex).rgb;
    float l = luminance(v);
    color = vec4(l);
    vec3 tv = v / (1.0f + v);
    color = vec4(mix(v / (1.0f + l), tv, tv), 1.0);
}