#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef ENABLE_SHADOWS
in vec4 mockTransform;
in vec4 shadowCoord0;
in vec4 shadowCoord1;
in vec4 shadowCoord2;
in vec4 shadowCoord3;
#endif

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
layout(binding = 1) uniform sampler2D normalMap;
#endif

#ifdef ENABLE_SHADOWS
uniform vec2 shadowMapResolution;
layout(binding = 2) uniform sampler2D shadowMap;
#endif

uniform vec3 lightCol;
#if defined(DIRECTIONAL_LIGHT) || defined(ENABLE_SHADOWS)
uniform vec3 lightDir;
#elif defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform float lightRadiance;
#endif

uniform vec3 camPos;

out vec4 color;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= texture(albedoMap, fragTex);
#endif

#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif

	// TODO - normal mapping
	vec3 N = normalize(fragNor);
	vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
#ifdef DIRECTIONAL_LIGHT
	vec3 Ldir = normalize(lightDir);
#elif defined(POINT_LIGHT)
	vec3 lightDir = lightPos - fragPos.xyz;
	float lightDistance = length(lightDir);
	vec3 Ldir = lightDir / lightDistance;

	attFactor = lightDistance / lightRadiance;
#else
	vec3 Ldir = vec3(0, 0, 0);
#endif

	color.rgb = lambertianDiffuse(Ldir, N, fAlbedo.rgb, lightCol, attFactor);

#ifdef ENABLE_SHADOWS
	float mockDepth = saturate(mockTransform.z / mockTransform.w);
	vec2 shadowTex;
	float L0Depth = shadowCoord0.z / shadowCoord0.w;
	float L1Depth = shadowCoord1.z / shadowCoord1.w;
	float L2Depth = shadowCoord2.z / shadowCoord2.w;
	float L3Depth = shadowCoord3.z / shadowCoord3.w;
	int layer = 0;
	if (L0Depth > 0.0 && L0Depth < 1.0 && shadowCoord0.x < 1.0 && shadowCoord0.x > 0.0 && shadowCoord0.y > 0.0 && shadowCoord0.y < 1.0) {
		layer = 0;
		shadowTex = shadowCoord0.xy;
	}
	else if (L1Depth > 0.0 && L1Depth < 1.0 && shadowCoord1.x < 1.0 && shadowCoord1.x > 0.0 && shadowCoord1.y > 0.0 && shadowCoord1.y < 1.0) {
		layer = 0;
		shadowTex = shadowCoord1.xy;
	}
	else if (L2Depth > 0.0 && L2Depth < 1.0 && shadowCoord2.x < 1.0 && shadowCoord2.x > 0.0 && shadowCoord2.y > 0.0 && shadowCoord2.y < 1.0) {
		layer = 2;
		shadowTex = shadowCoord2.xy;
	}
	else if (L3Depth > 0.0 && L3Depth < 1.0 && shadowCoord3.x < 1.0 && shadowCoord3.x > 0.0 && shadowCoord3.y > 0.0 && shadowCoord3.y < 1.0) {
		layer = 3;
		shadowTex = shadowCoord3.xy;
	}
	color *= vec4(vec3(texture(shadowMap, shadowTex, layer).r), 1.0);

	//float visibility = max(getShadowVisibility(0, shadowMap, shadowMapResolution, shadowCoord, 0.005), 0.2);
	//color.rgb *= visibility;

#endif

	color.a = 1.0;
#ifdef TRANSPARENT
	color.a = fAlbedo.a;
#endif
}

