#include "shadowreceiver.glsl"
#include "pbr.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormalRoughness;
layout(binding = 2) uniform sampler2D gEmissiveMetalness;
layout(binding = 3) uniform sampler2D gDepth;

#ifdef ENABLE_SHADOWS
uniform vec2 shadowMapResolution;
uniform mat4 lightTransform;
layout(binding = 5) uniform sampler2D shadowMap;
#endif

uniform mat4 invP;
uniform mat4 invV;
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
	vec4 shadowCoord = lightTransform * vec4(worldPos, 1.0);
	float visibility = getShadowVisibility(2, shadowMap, shadowMapResolution, shadowCoord, 0.001);
	pbrColor.directDiffuse *= visibility;
	pbrColor.directSpecular *= visibility;
#endif

	
	color.rgb = vec3(0)
		+ calculateIndirectDiffuse(pbrMaterial.albedo, pbrMaterial.metalness, pbrLight.radiance, 0.0002)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
	;
	color.a = 1.0;

}

