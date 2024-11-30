#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

#ifdef ENABLE_SHADOWS
in vec4 shadowCoord[4];
uniform vec4 csmDepths;
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

float getShadow(vec4 _csmDepths, vec4 _shadowCoord[4], sampler2D _shadowMap) {
	int lod = 0;
	if (sceneDepth < _csmDepths.x) {
		lod = 0;
	}
	else if (sceneDepth < _csmDepths.y) {
		lod = 1;
	}
	else if (sceneDepth < _csmDepths.z) {
		lod = 2;
	}
	else if (sceneDepth < _csmDepths.w) {
		lod = 3;
	}

	float shadow = getSingleShadow(_shadowCoord[lod], _shadowMap, lod);
	return 1.0 - saturate(shadow);
}

void main() {
	vec4 fAlbedo = albedo;
	vec3 N = normalize(fragNor);
	vec3 V = normalize(camPos - fragPos.xyz);

	float attFactor = 1;
	vec3 Ldir = normalize(lightDir);

	color.rgb = lambertianDiffuse(Ldir, N, fAlbedo.rgb, lightCol, attFactor);
	color.a = 1.0;

#ifdef ENABLE_SHADOWS
	float visibility = getShadow(csmDepths, shadowCoord, shadowMap);

	color *= vec4(vec3(max(visibility, 0.2)), 1.0);
#endif

#ifdef DEBUG_VIEW
	const float scale = 0.2;
	if (sceneDepth > 0) {
		if (sceneDepth <= csmDepths.x && validCascade(shadowCoord[0])) {
			color.yz *= scale;
		}
		else if (sceneDepth < csmDepths.y && validCascade(shadowCoord[1])) {
			color.xz *= scale;
		}
		else if (sceneDepth < csmDepths.z && validCascade(shadowCoord[2])) {
			color.xy *= scale;
		}
		else if (sceneDepth < csmDepths.w && validCascade(shadowCoord[3])) {
			color.z *= scale;
		}
	}
#endif

	color.a = 1.0;
}

