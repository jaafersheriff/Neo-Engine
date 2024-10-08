#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"
#include "normal.glsl"

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

float getSingleShadow(vec4 shadowCoord, sampler2D _shadowMap, int lod) {
	if (shadowCoord.z < 0.0 || shadowCoord.z > 1.0) {
		return 0.0;
	}
	if (shadowCoord.x < 0.0 || shadowCoord.x > 1.0) {
		return 0.0;
	}
	if (shadowCoord.y < 0.0 || shadowCoord.y > 1.0) {
		return 0.0;
	}

	return saturate(shadowCoord.z) - 0.002 > textureLod(_shadowMap, saturate(shadowCoord.xy), 0).r ? 1.0 : 0.0;
}

float getShadow(
	vec4 mockTransform, 
	vec4 _shadowCoord0, 
	vec4 _shadowCoord1, 
	vec4 _shadowCoord2, 
	vec4 _shadowCoord3,
	sampler2D _shadowMap
) {
	float mockDepth = saturate(mockTransform.z / mockTransform.w); // TODO - use this to minimize texture samples
	float shadow = 0.0;
	shadow += getSingleShadow(_shadowCoord0, _shadowMap, 0);
	shadow += getSingleShadow(_shadowCoord1, _shadowMap, 1);
	shadow += getSingleShadow(_shadowCoord2, _shadowMap, 2);
	shadow += getSingleShadow(_shadowCoord3, _shadowMap, 3);

	return 1.0 - saturate(shadow);
}

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= texture(albedoMap, fragTex);
#endif

#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif

	vec3 N = normalize(fragNor);
#ifdef NORMAL_MAP
	N = getNormal(fNorm, texture(normalMap, fragTex).rgb, fragPos.xyz, fragTex);
#endif
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
	float visibility = getShadow(
		mockTransform,
		shadowCoord0,
		shadowCoord1,
		shadowCoord2,
		shadowCoord3,
		shadowMap
	);

	color *= vec4(vec3(visibility), 1.0);

	//float visibility = max(getShadowVisibility(0, shadowMap, shadowMapResolution, shadowCoord, 0.005), 0.2);
	//color.rgb *= visibility;

#endif

	color.a = 1.0;
#ifdef TRANSPARENT
	color.a = fAlbedo.a;
#endif
}

