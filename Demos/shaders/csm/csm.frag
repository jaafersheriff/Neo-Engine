#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec4 albedo;

#ifdef ENABLE_SHADOWS
in vec4 shadowCoord[3];
uniform vec3 csmDepths;
in float sceneDepth;

layout(binding = 2) uniform sampler2D shadowMap;
#endif

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

#ifdef ENABLE_SHADOWS
	float visibility = getCSMShadowVisibility(sceneDepth, csmDepths, shadowCoord, shadowMap);
	color *= vec4(vec3(max(visibility, 0.2)), 1.0);
#endif

#ifdef DEBUG_VIEW
	const float scale = 0.2;
	if (sceneDepth > 0) {
		if (sceneDepth <= csmDepths.x && validCascade(shadowCoord[0])) {
			color.yz *= scale;
		}
		else if (sceneDepth <= csmDepths.y && validCascade(shadowCoord[1])) {
			color.xz *= scale;
		}
		else if (sceneDepth <= csmDepths.z && validCascade(shadowCoord[2])) {
			color.xy *= scale;
		}
	}
#endif

	color.a = 1.0;
}

