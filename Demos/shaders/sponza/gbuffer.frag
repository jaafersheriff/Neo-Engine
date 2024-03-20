#include "alphaDiscard.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
// TODO - unused
layout(binding = 1) uniform sampler2D normalMap;
#endif

layout (location = 0) out vec4 gDiffuse;
layout (location = 1) out vec4 gWorld;
layout (location = 2) out vec4 gNormal;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= texture(albedoMap, fragTex);
#endif

#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif

	gDiffuse = vec4(fAlbedo.rgb, 1.f);
	gWorld = vec4(fragPos.rgb, 1.f);
	gNormal = vec4(normalize(fragNor), 1.f);
}  