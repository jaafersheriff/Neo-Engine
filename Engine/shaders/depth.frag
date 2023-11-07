
in vec2 fragTex;

#ifdef ALPHA_TEST
#include "alphaDiscard.glsl"
layout (binding = 0) uniform sampler2D alphaMap;
#endif


void main() {
#ifdef ALPHA_TEST
	alphaDiscard(texture(alphaMap, fragTex).a);
#endif
}