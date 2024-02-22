#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "pbr/pbr.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

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

#ifdef EMISSIVE
layout(binding = 4) uniform sampler2D emissiveMap;
uniform vec3 emissiveFactor;
#endif

#ifdef ENABLE_SHADOWS
uniform mat4 L;
layout(binding = 5) uniform sampler2D shadowMap;
#endif

uniform vec3 lightCol;
#if defined(DIRECTIONAL_LIGHT) || defined(ENABLE_SHADOWS)
uniform vec3 lightDir;
#endif
#if defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform vec3 lightAtt;
#endif

uniform mat3 N;

uniform vec3 camPos;
uniform vec3 camDir;

out vec4 color;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= texture(albedoMap, fragTex);
#endif

#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif

	float fMetalness = metalness;
	float fRoughness = roughness;
#ifdef METAL_ROUGHNESS_MAP
	vec3 metalRoughness = texture(metalRoughnessMap, fragTex).rgb;
	fMetalness *= metalRoughness.b;
	fRoughness *= metalRoughness.g;
#endif
#ifdef DEBUG_METAL_ROUGHNESS
	color = vec4(0.0, fRoughness, fMetalness, 1.0);
	return;
#endif

	vec3 fEmissive = vec3(0.0);
#ifdef EMISSIVE
	fEmissive = emissiveFactor * texture(emissiveMap, fragTex).rgb;
#endif
#ifdef DEBUG_EMISSIVE
	color = vec4(fEmissive, 1);
	return;
#endif

	// TODO - normal mapping
	vec3 fNorm = normalize(N * fragNor);
#ifdef NORMAL_MAP
	//fNorm = normalize(texture(normalMap, fragTex).rgb) * 2.0 - 1.0;
	//fNorm = N * fNorm;
#endif
	vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
	vec3 Ldir = vec3(0, 0, 0);
#ifdef DIRECTIONAL_LIGHT
	Ldir = normalize(lightDir);
#endif
#if defined(POINT_LIGHT)
	vec3 lightDir = lightPos - fragPos.xyz;
	float lightDistance = length(lightDir);
	= lightDir / lightDistance;
	if (length(lightAtt) > 0) {
		attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
	}
#endif

	PBRMaterial pbrData;
	pbrData.albedo = fAlbedo.rgb;
	pbrData.N = fNorm;
	pbrData.V = V;
	pbrData.linearRoughness = fRoughness;
	pbrData.metalness = fMetalness;
	pbrData.emissive = fEmissive;

	PBRLight pbrLight;
	pbrLight.L = Ldir;
	pbrLight.radiance = lightCol;
	color.rgb = doPBR(pbrData, pbrLight);

#ifdef OCCLUSION_MAP
	color.rgb *= texture(occlusionMap, fragTex).r;
#endif

#ifdef ENABLE_SHADOWS
	vec4 shadowCoord = L * fragPos;
	float visibility = max(getShadowVisibility(1, shadowMap, shadowCoord, 0.005), 0.3);
	color.rgb *= visibility;
#endif
	color.a = 1.0;
}

