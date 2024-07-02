#include "pbr.glsl"
#include "ibl.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gWorldRoughness;
layout(binding = 3) uniform sampler2D gEmissiveMetalness;
layout(binding = 4) uniform sampler2D gDepth;

uniform vec3 camPos;

#ifdef IBL
 layout(binding = 5) uniform samplerCube ibl;
 layout(binding = 6) uniform sampler2D dfgLUT;
 uniform int iblMips;
#endif

out vec4 color;

void main() {

	vec4 albedoAO = texture(gAlbedoAO, fragTex);
	vec3 normal = texture(gNormal, fragTex).rgb;
	vec4 worldRoughness = texture(gWorldRoughness, fragTex);
	vec4 emissiveMetalness = texture(gEmissiveMetalness, fragTex);

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = albedoAO.rgb;
	pbrMaterial.N = normalize(normal.xyz * 2.0 - 1.0);
	pbrMaterial.V = normalize(camPos - worldRoughness.xyz); // TODO - negative world pos is clamped to 0?
	pbrMaterial.linearRoughness = worldRoughness.a;
	pbrMaterial.metalness = emissiveMetalness.a;
	pbrMaterial.F0 = calculateF0(albedoAO.rgb, emissiveMetalness.a);
	pbrMaterial.ao = albedoAO.a;

	color.rgb = vec3(0)
		+ calculateIndirectDiffuse(pbrMaterial.albedo, pbrMaterial.metalness, 0.08)
#ifdef IBL
		+ getIndirectSpecular(pbrMaterial, iblMips, dfgLUT, ibl)
#endif
		+ emissiveMetalness.rgb
	;
	color.a = 1.0;

}

