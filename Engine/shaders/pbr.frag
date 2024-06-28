#include "alphaDiscard.glsl"
#include "shadowreceiver.glsl"
#include "pbr.glsl"
#include "ibl.glsl"
#include "color.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;
#ifdef TANGENTS
in vec4 fragTan;
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

#ifdef IBL
 layout(binding = 6) uniform samplerCube ibl;
 layout(binding = 7) uniform sampler2D dfgLUT;
 uniform int iblMips;
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

	mat3 TBN;
#ifdef TANGENTS
	vec3 tan = normalize(fragTan.xyz);
	vec3 biTan = normalize(cross(baseNormal, tan)) * fragTan.w;
	if (any(isnan(biTan))) {
		biTan = vec3(0, 1, 0); // uhhh
	}
	TBN = mat3(tan, biTan, baseNormal);

#else
	// http://www.thetenthplanet.de/archives/1180
	vec3 dp1 = dFdx( fragPos.xyz ); 
	vec3 dp2 = dFdy( fragPos.xyz ); 
	vec2 duv1 = dFdx( fragTex ); 
	vec2 duv2 = dFdy( fragTex );
	vec3 dp2perp = cross( dp2, baseNormal ); 
	vec3 dp1perp = cross( baseNormal, dp1 ); 
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x; 
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) ); 
	TBN = mat3( T * invmax, B * invmax, baseNormal );
#endif
	vec3 tangentNormal = texture(normalMap, fragTex).xyz * 2.0 - 1.0;
	return normalize(TBN * normalize(tangentNormal));
#endif
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
	L = normalize(lightDir);
	if (length(lightAtt) > 0) {
		float lightDistance = length(lightDir);
		attFactor = 1.f / (lightAtt.x + lightAtt.y * lightDistance + lightAtt.z * lightDistance * lightDistance);
	}
#endif

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = fAlbedo.rgb;
	pbrMaterial.N = fNorm;
	pbrMaterial.V = V;
	pbrMaterial.linearRoughness = fRoughness;
	pbrMaterial.metalness = fMetalness;
	pbrMaterial.F0 = mix(vec3(DIALECTRIC_REFLECTANCE), fAlbedo.rgb, vec3(fMetalness));

	PBRLight pbrLight;
	pbrLight.L = L;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a * attFactor;

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);

	brdf(pbrMaterial, pbrLight, pbrColor);

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

#ifdef IBL
	vec3 R = reflect(-V, fNorm);
	pbrColor.indirectSpecular = getIndirectSpecular(R, pbrMaterial, iblMips, dfgLUT, ibl);
#endif

	color.rgb = vec3(0)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
		+ pbrColor.indirectDiffuse
		+ pbrColor.indirectSpecular
		+ fEmissive;
	;

#ifdef DEBUG_ALBEDO
	color = vec4(fAlbedo.rgb, 1.0);
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
	vec3 _fNorm = fNorm * 0.5 + 0.5;
	color = vec4(_fNorm.x, _fNorm.y, _fNorm.z, 1);

	return;
#endif

#ifdef DEBUG_DIFFUSE
	color = vec4(pbrColor.directDiffuse + pbrColor.indirectDiffuse, 1.0);
	return;
#endif

#ifdef DEBUG_SPECULAR
	color = vec4(pbrColor.directSpecular + pbrColor.indirectSpecular, 1.0);
	return;
#endif


}

