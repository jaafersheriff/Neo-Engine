
#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec2 fragTex;

uniform sampler2D diffuseMap;
uniform bool useTexture;

out vec4 color;

void main() {
    if (useTexture) {
        alphaDiscard(texture(diffuseMap, fragTex).a);
    }

    color = vec4(vec3(0.0), 1.0);
}
