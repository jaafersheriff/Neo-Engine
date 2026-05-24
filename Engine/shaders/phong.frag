#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef ENABLE_SHADOWS
in vec4 shadowCoord;
#endif

layout (std140, binding = 0) uniform UBO {
	vec4 camPos;
	vec4 lightCol;
	// DIRECTIONAL_LIGHT
	vec4 lightDir;
	// POINT_LIGHT
	vec4 lightPos;
	float lightRadiance;
	int pad0;
	int pad1;
	int pad2;
};

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 1) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
layout(binding = 2) uniform sampler2D normalMap;
#endif

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
	vec3 V = normalize(camPos.xyz - fragPos.xyz);

float attFactor = 1;
#ifdef DIRECTIONAL_LIGHT
	vec3 Ldir = normalize(lightDir.xyz);
#elif defined(POINT_LIGHT)
	vec3 pointLightDir = lightPos.xyz - fragPos.xyz;
	float lightDistance = length(pointLightDir);
	vec3 Ldir = pointLightDir / lightDistance;

	attFactor = lightDistance / lightRadiance;
#else
	vec3 Ldir = vec3(0, 0, 0);
#endif

	color.rgb = lambertianDiffuse(Ldir, N, fAlbedo.rgb, lightCol.rgb, attFactor);

	color.a = 1.0;
#ifdef TRANSPARENT
	color.a = fAlbedo.a;
#endif
}

