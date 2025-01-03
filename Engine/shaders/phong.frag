#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef ENABLE_SHADOWS
in vec4 shadowCoord;
#endif


uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
layout(binding = 1) uniform sampler2D normalMap;
#endif

uniform vec3 lightCol;
#if defined(DIRECTIONAL_LIGHT)
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

	color.a = 1.0;
#ifdef TRANSPARENT
	color.a = fAlbedo.a;
#endif
}

