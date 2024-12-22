#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "pbr.glsl"
#include "ibl.glsl"
#include "color.glsl"
#include "normal.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef TANGENTS
in vec4 fragTan;
#endif

uniform vec4 albedo;
#ifdef ALBEDO_MAP
layout(binding = 0) uniform sampler2D albedoMap;
#endif

#ifdef NORMAL_MAP
layout(binding = 1) uniform sampler2D normalMap;
uniform float normalMapScale;
#endif

uniform float metalness;
uniform float roughness;
#ifdef METAL_ROUGHNESS_MAP
layout(binding = 2) uniform sampler2D metalRoughnessMap;
#endif

#ifdef OCCLUSION_MAP
layout(binding = 3) uniform sampler2D occlusionMap; // Shouldn't be used for indirect lights
uniform float occlusionStrength;
#endif

uniform vec3 emissiveFactor;
#ifdef EMISSIVE
layout(binding = 4) uniform sampler2D emissiveMap;
#endif

#ifdef ENABLE_SHADOWS
uniform vec2 shadowMapResolution;
#	ifdef DIRECTIONAL_LIGHT
	in vec4 shadowCoord[3];
	uniform vec3 csmDepths;
	layout(binding = 5) uniform sampler2D shadowMap;
#	elif defined(POINT_LIGHT)
	layout(binding = 5) uniform samplerCube shadowMap;
	uniform float shadowRange;
#	endif
#endif

#ifdef IBL
layout(binding = 6) uniform samplerCube ibl;
layout(binding = 7) uniform sampler2D dfgLUT;
uniform int iblMips;
#endif

uniform vec4 lightRadiance;
#if defined(DIRECTIONAL_LIGHT)
uniform vec3 lightDir;
#endif
#if defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform float lightRadius;
#endif

uniform vec3 camPos;

out vec4 color;

void main() {
	vec4 fAlbedo = albedo;
#ifdef ALBEDO_MAP
	fAlbedo *= srgbToLinear(texture(albedoMap, fragTex));
#endif

#ifdef ALPHA_TEST
	alphaDiscard(fAlbedo.a);
#endif
#ifdef TRANSPARENT
	alphaDiscard(fAlbedo.a, 0.1);
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

	vec3 fNorm = normalize(fragNor);
#ifdef NORMAL_MAP
	#ifdef TANGENTS
	fNorm = getNormal(fNorm, texture(normalMap, fragTex).rgb, normalMapScale, fragTan);
	#else
	fNorm = getNormal(fNorm, texture(normalMap, fragTex).rgb, normalMapScale, fragPos.xyz, fragTex);
	#endif
#endif

	vec3 V = normalize(camPos - fragPos.xyz);

	float attFactor = 1;
	vec3 L = vec3(0, 0, 0);
#ifdef DIRECTIONAL_LIGHT
	L = normalize(lightDir);
#elif defined(POINT_LIGHT)
	vec3 lightDir = lightPos - fragPos.xyz;
	L = normalize(lightDir);
	float lightDistance = length(lightDir);
	if (lightDistance == 0.0 || lightDistance > lightRadius) {
		color = vec4(0, 0, 0, fAlbedo.a);
		return;
	}
	attFactor = lightDistance * lightDistance * lightDistance; // Not physically based, but better falloff :) 
#endif

	float ao = 1.f;
#ifdef OCCLUSION_MAP
	float ao = 1.0 + occlusionStrength * (texture(occlusionMap, fragTex).r - 1.0);
#endif

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = fAlbedo.rgb;
	pbrMaterial.N = fNorm;
	pbrMaterial.V = V;
	pbrMaterial.linearRoughness = fRoughness;
	pbrMaterial.metalness = fMetalness;
	pbrMaterial.F0 = calculateF0(fAlbedo.rgb, fMetalness);
	pbrMaterial.ao = ao;

	PBRLight pbrLight;
	pbrLight.L = L;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a / attFactor;

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);

	brdf(pbrMaterial, pbrLight, pbrColor);

	pbrColor.indirectDiffuse = calculateIndirectDiffuse(pbrMaterial.albedo, pbrMaterial.metalness, pbrLight.radiance);

#ifdef IBL
	pbrColor.indirectSpecular = getIndirectSpecular(pbrMaterial, iblMips, dfgLUT, ibl);
#endif


#ifdef ENABLE_SHADOWS
#	ifdef DIRECTIONAL_LIGHT
	float visibility = getCSMShadowVisibility(gl_FragCoord.z / gl_FragCoord.w, csmDepths, shadowCoord, shadowMap);
#	elif defined(POINT_LIGHT)
	float visibility = getShadowVisibility(1, shadowMap, fragPos.xyz - lightPos, shadowMapResolution.x, shadowRange, 0.001);
#	endif
	pbrColor.directDiffuse *= visibility;
	pbrColor.directSpecular *= visibility;
#endif

	color.rgb = vec3(0)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
		+ pbrColor.indirectDiffuse
		+ pbrColor.indirectSpecular
		+ fEmissive
	;
	color.a = 1.0;
#ifdef TRANSPARENT
	color.a = fAlbedo.a;
#endif
}

