#include "alphaDiscard.glsl"
#include "phong.glsl"

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

uniform vec3 lightCol;
#if defined(DIRECTIONAL_LIGHT) || defined(ENABLE_SHADOWS)
uniform vec3 lightDir;
#elif defined(POINT_LIGHT)
uniform vec3 lightPos;
uniform vec3 lightAtt;
#endif

uniform vec3 camPos;

out vec4 color;

void main() {
	vec3 fAlbedo = albedo.rgb;
#ifdef ALBEDO_MAP
	vec4 albedoSample = texture(albedoMap, fragTex);
	fAlbedo *= albedoSample.rgb;

#endif

#ifdef DEBUG_METAL_ROUGHNESS
	float fMetalness = metalness;
	float fRoughness = roughness;
#ifdef METAL_ROUGHNESS_MAP
	vec3 metalRoughness = texture(metalRoughnessMap, fragTex).rgb;
	fMetalness *= metalRoughness.b;
	fRoughness *= metalRoughness.g;
#endif
	color = vec4(0.0, fRoughness, fMetalness, 1.0);
	return;
#endif


#ifdef DEBUG_EMISSIVE
#ifdef EMISSIVE
	color = 1vec4(emissiveFactor * texture(emissiveMap, fragTex).rgb, 1.0);
#else
	color = vec4(0, 0, 0, 1);
#endif
	return;
#endif

	// TODO - normal mapping
	vec3 N = normalize(fragNor);
#ifdef NORMAL_MAP
// 	N = normalize(texture(normalMap, fragTex).rgb);
#endif
	vec3 V = normalize(camPos - fragPos.xyz);

float attFactor = 1;
#ifdef DIRECTIONAL_LIGHT
	vec3 L = normalize(lightDir);
#elif defined(POINT_LIGHT)
	vec3 lightDir = lightPos - fragPos.xyz;
	vec3 L = normalize(lightDir);
	float lightDistance = length(lightDir);
	if (length(lightAtt) > 0) {
		attFactor = lightAtt.x + lightAtt.y*lightDistance + lightAtt.z*lightDistance*lightDistance;
	}
#else
	vec3 L = vec3(0, 0, 0);
#endif

	color.rgb = getPhong(V, N, L, fAlbedo.rgb * 0.2, fAlbedo.rgb, vec3(1.0), 13.0, lightCol, attFactor);
#ifdef OCCLUSION_MAP
	color.rgb *= texture(occlusionMap, fragTex).rgb;
#endif
	color.a = 1.0;
}

