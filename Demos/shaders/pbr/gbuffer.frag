#include "alphaDiscard.glsl"
#include "color.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef TANGENTS
in vec4 fragTan;
#endif

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
layout(binding = 1) uniform sampler2D normalMap;
#endif

uniform float metalness;
uniform float roughness;
#ifdef METAL_ROUGHNESS_MAP
layout(binding = 2) uniform sampler2D metalRoughnessMap;
#endif

#ifdef OCCLUSION_MAP
layout(binding = 3) uniform sampler2D occlusionMap; // Shouldn't be used for indirect lights
#endif

uniform vec3 emissiveFactor;
#ifdef EMISSIVE
layout(binding = 4) uniform sampler2D emissiveMap;
#endif


layout (location = 0) out vec4 gAlbedoAO;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gWorldRoughness;
layout (location = 3) out vec4 gEmissiveMetalness;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= srgbToLinear(texture(albedoMap, fragTex));
#endif
#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif
 	float ao = 1.0;
 #ifdef OCCLUSION_MAP
 	ao = texture(occlusionMap, fragTex).r;
 #endif
	gAlbedoAO = vec4(fAlbedo.rgb, ao);

	vec3 fNorm = normalize(fragNor);
#ifdef NORMAL_MAP
	#ifdef TANGENTS
	fNorm = getNormal(fNorm, texture(normalMap, fragTex).rgb, fragTan);
	#else
	fNorm = getNormal(fNorm, texture(normalMap, fragTex).rgb, fragPos.xyz, fragTex);
	#endif
#endif
 	gNormal= vec4(fNorm, 1.0);

	float fMetalness = metalness;
	float fRoughness = roughness;
#ifdef METAL_ROUGHNESS_MAP
	vec3 metalRoughness = texture(metalRoughnessMap, fragTex).rgb;
	fMetalness *= metalRoughness.b;
	fRoughness *= metalRoughness.g;
#endif
	gWorldRoughness = vec4(fragPos.xyz, fRoughness);

	vec3 fEmissive = emissiveFactor;
#ifdef EMISSIVE
	fEmissive *= srgbToLinear(texture(emissiveMap, fragTex)).rgb;
#endif
	gEmissiveMetalness = vec4(fEmissive, fMetalness);

}  