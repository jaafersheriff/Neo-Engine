
in vec2 fragTex;

#include "alphaDiscard.glsl"

#ifdef ALPHA_TEST
layout (binding = 0) uniform sampler2D alphaMap;
#endif


void main() {
#ifdef ALPHA_TEST
	vec4 fAlbedo = texture(alphaMap, fragTex);
	alphaDiscard(fAlbedo.a);
#endif
}