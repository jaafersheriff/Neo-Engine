#include "shadowreceiver.glsl"
#include "pbr.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormalRoughness;
layout(binding = 2) uniform sampler2D gEmissiveMetalness;
layout(binding = 3) uniform sampler2D gDepth;

#ifdef ENABLE_SHADOWS
uniform mat4 L0;
uniform mat4 L1;
uniform mat4 L2;
uniform vec3 csmDepths;
layout(binding = 5) uniform sampler2D shadowMap;
#endif

uniform mat4 invP;
uniform mat4 invV;
uniform mat4 P;
uniform mat4 V;
uniform vec3 camPos;

uniform vec4 lightRadiance;
uniform vec3 lightDir;

out vec4 color;

void main() {
	vec4 albedoAO = texture(gAlbedoAO, fragTex);
	vec4 normalRoughness = texture(gNormalRoughness, fragTex);
	vec4 emissiveMetalness = texture(gEmissiveMetalness, fragTex);
	float depth = texture(gDepth, fragTex).r;
	vec3 worldPos = reconstructWorldPos(fragTex, depth, invP, invV);

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = albedoAO.rgb;
	pbrMaterial.N = normalize(normalRoughness.xyz * 2.0 - 1.0);
	pbrMaterial.V = normalize(camPos - worldPos);
	pbrMaterial.linearRoughness = normalRoughness.a;
	pbrMaterial.metalness = emissiveMetalness.a;
	pbrMaterial.F0 = calculateF0(albedoAO.rgb, emissiveMetalness.a);
	pbrMaterial.ao = albedoAO.a;

	PBRLight pbrLight;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a;
	pbrLight.L = normalize(lightDir);

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);
	brdf(pbrMaterial, pbrLight, pbrColor);


#ifdef ENABLE_SHADOWS
	vec4 shadowCoord[3];
	shadowCoord[0] = L0 * vec4(worldPos, 1.0);
	shadowCoord[1] = L1 * vec4(worldPos, 1.0);
	shadowCoord[2] = L2 * vec4(worldPos, 1.0);
	vec4 _d = P * V * vec4(worldPos, 1.0);
	float visibility = getCSMShadowVisibility(_d.z, csmDepths, shadowCoord, shadowMap);
	pbrColor.directDiffuse *= visibility;
	pbrColor.directSpecular *= visibility;
#endif
	
	color.rgb = vec3(0)
		+ calculateIndirectDiffuse(pbrMaterial.albedo, pbrMaterial.metalness, pbrLight.radiance, 0.0002)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
	;
	color.a = 1.0;

#if defined(ENABLE_SHADOWS) && 0
	 if (_d.z < csmDepths.x) {
	 	color.rgb *= vec3(1, 0, 0); 
	 }
	 else if (_d.z < csmDepths.y) {
	 	color.rgb *= vec3(0, 1, 0); 
	 }
	 else if (_d.z < csmDepths.z) {
	 	color.rgb *= vec3(0, 0, 1); 
	 }
#endif

}

