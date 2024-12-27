#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec4 albedo;

in vec4 shadowCoord[3];
layout(binding = 2) uniform sampler2D shadowMap;
uniform float shadowMapResolution;

uniform vec3 lightCol;
uniform vec3 lightDir;

uniform vec3 camPos;

out vec4 color;


void main() {
	vec4 fAlbedo = albedo;
	vec3 N = normalize(fragNor);
	vec3 V = normalize(camPos - fragPos.xyz);

	float attFactor = 1;
	vec3 Ldir = normalize(lightDir);

	color.rgb = lambertianDiffuse(Ldir, N, fAlbedo.rgb, lightCol, attFactor);
	color.a = 1.0;

	float visibility = getCSMShadowVisibility(1, shadowCoord, shadowMap, shadowMapResolution, 0.0001);
	color *= vec4(vec3(max(visibility, 0.2)), 1.0);

#if defined(DEBUG_VIEW)
	const float scale = 0.2;
	if (validCascade(shadowCoord[0])) {
		color.yz *= scale;
	}
	else if (validCascade(shadowCoord[1])) {
		color.xz *= scale;
	}
	else if (validCascade(shadowCoord[2])) {
		color.xy *= scale;
	}

#endif

	color.a = 1.0;
}

