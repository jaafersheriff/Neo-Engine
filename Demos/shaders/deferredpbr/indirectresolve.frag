#include "pbr.glsl"
#include "ibl.glsl"

in vec2 fragTex;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 2) uniform sampler2D gNormalRoughness;
layout(binding = 3) uniform sampler2D gEmissiveMetalness;
layout(binding = 4) uniform sampler2D gDepth;

uniform mat4 invP;
uniform mat4 invV;
uniform vec3 camPos;

#ifdef IBL
 layout(binding = 5) uniform samplerCube ibl;
 layout(binding = 6) uniform sampler2D dfgLUT;
 uniform int iblMips;
#endif

out vec4 color;

void main() {

	vec4 albedoAO = texture(gAlbedoAO, fragTex);
	vec4 normalRoughness = texture(gNormalRoughness, fragTex);
	vec4 emissiveMetalness = texture(gEmissiveMetalness, fragTex);
	float depth = texture(gDepth, fragTex).r;
	if (depth >= 1.0) {
		color = vec4(0, 0, 0, 1);
		return;
	}
	vec3 worldPos = reconstructWorldPos(fragTex, depth, invP, invV);

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = albedoAO.rgb;
	pbrMaterial.N = normalize(normalRoughness.xyz * 2.0 - 1.0);
	pbrMaterial.V = normalize(camPos - worldPos);
	pbrMaterial.linearRoughness = normalRoughness.a;
	pbrMaterial.metalness = emissiveMetalness.a;
	pbrMaterial.F0 = calculateF0(albedoAO.rgb, emissiveMetalness.a);
	pbrMaterial.ao = albedoAO.a;

	color.rgb = vec3(0)
		+ calculateIndirectDiffuse(pbrMaterial.albedo, pbrMaterial.metalness, 0.1)
#ifdef IBL
		+ getIndirectSpecular(pbrMaterial, iblMips, dfgLUT, ibl)
#endif
		+ emissiveMetalness.rgb
	;
	color.a = 1.0;

}

