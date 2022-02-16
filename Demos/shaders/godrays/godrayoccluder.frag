
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec2 fragTex;

uniform sampler2D alphaMap;
uniform bool useTexture;

out vec4 color;

void main() {
    float alpha = texture(alphaMap, fragTex).a;
    alphaDiscard(alpha);
    color = vec4(vec3(0.0), alpha);
}
