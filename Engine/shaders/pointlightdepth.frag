
in vec4 fragPos;
in vec2 fragTex;

#include "alphaDiscard.glsl"

#ifdef ALPHA_TEST
layout (binding = 0) uniform sampler2D alphaMap;
#endif

uniform vec3 lightPos;
uniform float lightRange;


void main() {
#ifdef ALPHA_TEST
	vec4 fAlbedo = texture(alphaMap, fragTex);
	alphaDiscard(fAlbedo.a);
#endif

	// Write out in light units..?
	gl_FragDepth = length(fragPos.xyz - lightPos) / lightRange;
}
