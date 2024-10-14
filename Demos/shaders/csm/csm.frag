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

bool validCascade(vec4 shadowCoord) {
	return true
		&& shadowCoord.x > 0.0 && shadowCoord.x < 1.0
		&& shadowCoord.y > 0.0 && shadowCoord.y < 1.0
		&& shadowCoord.z > 0.0 && shadowCoord.z < 1.0
	;
}

float getSingleShadow(vec4 shadowCoord, sampler2D _shadowMap, int lod) {
	if (validCascade(shadowCoord)) {
		return saturate(shadowCoord.z) - 0.002 > textureLod(_shadowMap, saturate(shadowCoord.xy), lod).r ? 1.0 : 0.0;
	}
	return 0.f;
}

float getShadow(
	float _depth0,
	float _depth1,
	float _depth2,
	float _depth3,
	vec4 _shadowCoord0, 
	vec4 _shadowCoord1, 
	vec4 _shadowCoord2, 
	vec4 _shadowCoord3,
	sampler2D _shadowMap
) {
	int lod = 0;
	if (sceneDepth < _depth0) {
		lod = 0;
	}
	else if (sceneDepth < _depth1) {
		lod = 1;
	}
	else if (sceneDepth < _depth2) {
		lod = 2;
	}
	else if (sceneDepth < _depth3) {
		lod = 3;
	}

	float shadow = 0.0;

	// TODO This should just be array access 
	if (lod == 0) {
		shadow = getSingleShadow(_shadowCoord0, _shadowMap, lod);
	}
	if (lod == 1) {
		shadow = getSingleShadow(_shadowCoord1, _shadowMap, lod);
	}
	if (lod == 2) {
		shadow = getSingleShadow(_shadowCoord2, _shadowMap, lod);
	}
	if (lod == 3) {
		shadow = getSingleShadow(_shadowCoord3, _shadowMap, lod);
	}

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
		depth0,
		depth1,
		depth2,
		depth3,
		shadowCoord0,
		shadowCoord1,
		shadowCoord2,
		shadowCoord3,
		shadowMap
	);

	color *= vec4(vec3(max(visibility, 0.2)), 1.0);
#endif

#ifdef DEBUG_VIEW
	const float scale = 0.2;
	if (sceneDepth > 0) {
		if (sceneDepth <= depth0 && validCascade(shadowCoord0)) {
			color.yz *= scale;
		}
		else if (sceneDepth < depth1 && validCascade(shadowCoord1)) {
			color.xz *= scale;
		}
		else if (sceneDepth < depth2 && validCascade(shadowCoord2)) {
			color.xy *= scale;
		}
		else if (sceneDepth < depth3 && validCascade(shadowCoord3)) {
			color.z *= scale;
		}
	}
#endif

	color.a = 1.0;
}

