#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef ENABLE_SHADOWS
in vec4 shadowCoord0;
in vec4 shadowCoord1;
in vec4 shadowCoord2;
in vec4 shadowCoord3;
uniform float depth0;
uniform float depth1;
uniform float depth2;
uniform float depth3;


in float sceneDepth;
#endif

uniform vec4 albedo;

#ifdef ENABLE_SHADOWS
layout(binding = 2) uniform sampler2D shadowMap;
#endif

uniform vec3 lightCol;
uniform vec3 lightDir;

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

	return saturate(shadowCoord.z) - 0.002 > textureLod(_shadowMap, saturate(shadowCoord.xy), lod).r ? 1.0 : 0.0;
}

float getShadow(
	vec4 _shadowCoord0, 
	vec4 _shadowCoord1, 
	vec4 _shadowCoord2, 
	vec4 _shadowCoord3,
	sampler2D _shadowMap
) {
	float shadow = 0.0;
	shadow += getSingleShadow(_shadowCoord0, _shadowMap, 0);
	shadow += getSingleShadow(_shadowCoord1, _shadowMap, 1);
	shadow += getSingleShadow(_shadowCoord2, _shadowMap, 2);
	shadow += getSingleShadow(_shadowCoord3, _shadowMap, 3);

	return 1.0 - saturate(shadow);
}

void main() {
	vec4 fAlbedo = albedo;
	vec3 N = normalize(fragNor);
	vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
	vec3 Ldir = normalize(lightDir);

	color.rgb = lambertianDiffuse(Ldir, N, fAlbedo.rgb, lightCol, attFactor);

#ifdef ENABLE_SHADOWS
	float visibility = getShadow(
		shadowCoord0,
		shadowCoord1,
		shadowCoord2,
		shadowCoord3,
		shadowMap
	);

	color *= vec4(vec3(max(visibility, 0.2)), 1.0);
#endif

#ifdef DEBUG_VIEW
	if (sceneDepth > 0) {
		if (sceneDepth < depth0) {
			color.yz *= 0.3;
		}
		else if (sceneDepth < depth1) {
			color.xz *= 0.3;
		}
		else if (sceneDepth < depth2) {
			color.xy *= 0.3;
		}
		else if (sceneDepth < depth3) {
			color.z *= 0.3;
		}
	}
#endif

	color.a = 1.0;
}

