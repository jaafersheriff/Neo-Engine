#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "pbr/pbr.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef TANGENTS
in vec3 fragTan;
#endif
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

uniform float metalness;
uniform float roughness;
#ifdef METAL_ROUGHNESS_MAP
layout(binding = 2) uniform sampler2D metalRoughnessMap;
#endif

#ifdef OCCLUSION_MAP
layout(binding = 3) uniform sampler2D occlusionMap; // Shouldn't be used for indirect lights
#endif

uniform vec3 emissiveFactor;
#ifdef EMISSIVE
layout(binding = 4) uniform sampler2D emissiveMap;
#endif

#ifdef ENABLE_SHADOWS
uniform vec2 shadowMapResolution;
layout(binding = 5) uniform sampler2D shadowMap;
#endif

#ifdef SKYBOX
layout(binding = 6) uniform samplerCube skybox;
#endif

uniform vec4 lightRadiance;
#if defined(DIRECTIONAL_LIGHT) || defined(ENABLE_SHADOWS)
uniform vec3 lightDir;
#endif
#if defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform vec3 lightAtt;
#endif

uniform vec3 camPos;
uniform vec3 camDir;

out vec4 color;

vec3 getNormal() {
	vec3 baseNormal = normalize(fragNor);
#ifndef NORMAL_MAP
	return baseNormal;
#else

	vec3 T;
#ifdef TANGENTS
	T = fragTan;
#else
	vec3 q1 = dFdx(fragPos.xyz);
	vec3 q2 = dFdy(fragPos.xyz);
	vec2 st1 = dFdx(fragTex);
	vec2 st2 = dFdy(fragTex);
	T = normalize(q1 * st2.t - q2 * st1.t);
#endif

	vec3 _N = normalize(baseNormal);
	vec3 B = -normalize(cross(_N, T));
	mat3 TBN = mat3(T, B, _N);

	vec3 tangentNormal = texture(normalMap, fragTex).xyz * 2.0 - 1.0;
	return normalize(TBN * tangentNormal);
#endif
}

vec4 srgbToLinear(vec4 srgb) {
	return vec4(pow(srgb.xyz, vec3(2.2)), srgb.w);;
}

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= srgbToLinear(texture(albedoMap, fragTex));
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

	vec3 fEmissive = emissiveFactor;
#ifdef EMISSIVE
	fEmissive *= srgbToLinear(texture(emissiveMap, fragTex)).rgb;
#endif

	vec3 fNorm = getNormal();

	vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
	vec3 L = vec3(0, 0, 0);
#ifdef DIRECTIONAL_LIGHT
	L = normalize(lightDir);
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

	PBRLight pbrLight;
	pbrLight.L = L;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a;

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);

	brdf(pbrData, pbrLight, pbrColor);

#ifdef OCCLUSION_MAP
	float ao = texture(occlusionMap, fragTex).r;
	pbrColor.directDiffuse *= ao;
	pbrColor.directSpecular *= ao;
#endif

#ifdef ENABLE_SHADOWS
	float visibility = getShadowVisibility(1, shadowMap, shadowMapResolution, shadowCoord, 0.001);
	pbrColor.directDiffuse *= visibility;
	pbrColor.directSpecular *= visibility;
#endif

	pbrColor.indirectDiffuse = fAlbedo.rgb * 0.03 * (1.0 - fMetalness);

#ifdef SKYBOX
	vec3 R = reflect(-V, fNorm);
	//pbrColor.indirectSpecular += srgbToLinear(texture(skybox, R)).rgb;
#endif

	color.rgb = vec3(0)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
		+ pbrColor.indirectDiffuse
		+ pbrColor.indirectSpecular
	;
	color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
	color.rgb += fEmissive;
	color.a = 1.0;

#ifdef DEBUG_ALBEDO
	color = vec4(fAlbedo.rgb, 1.0);
	color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
	return;
#endif

#ifdef DEBUG_METAL_ROUGHNESS
	color = vec4(0.0, fRoughness, fMetalness, 1.0);
	return;
#endif

#ifdef DEBUG_EMISSIVE
	color = vec4(fEmissive, 1);
	return;
#endif

#ifdef DEBUG_NORMALS
	color = vec4(fNorm, 1);
	return;
#endif

#ifdef DEBUG_DIFFUSE
	color = vec4(pbrColor.directDiffuse, 1.0);
	color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
	return;
#endif

#ifdef DEBUG_SPECULAR
	color = vec4(pbrColor.directSpecular, 1.0);
	color.rgb = pow(color.rgb, vec3(1.0 / 2.2));
	return;
#endif


}
