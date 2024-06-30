#include "pbr.glsl"

in vec4 fragPos;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gWorldRoughness;
layout(binding = 3) uniform sampler2D gEmissiveMetalness;
layout(binding = 4) uniform sampler2D gDepth;

uniform vec3 camPos;
uniform vec2 resolution;

uniform vec4 lightRadiance;
uniform vec3 lightPos;
uniform float lightRadius;

#ifdef SHOW_LIGHTS
uniform float debugRadius;
#endif

out vec4 color;

float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr)
{
	// https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
	// - r0: ray origin
	// - rd: normalized ray direction
	// - s0: sphere center
	// - sr: sphere radius
	// - Returns distance from r0 to first intersecion with sphere,
	//   or -1.0 if no intersection.
	float a = dot(rd, rd);
	vec3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sr * sr);
	if (b*b - 4.0*a*c < 0.0)
	{
		return -1.0;
	}
	return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}



void main() {

#ifdef SHOW_LIGHTS
	float rayDist = raySphereIntersect(camPos, normalize(fragPos.xyz - camPos), lightPos, debugRadius);
	if (rayDist > 0.0 && rayDist < length(fragPos.xyz - camPos)) {
		color = vec4(lightRadiance.rgb * lightRadiance.a, 1.f);
		return;
	}
#endif

	vec2 uv = gl_FragCoord.xy / resolution.xy;
	vec4 albedoAO = texture(gAlbedoAO, uv);
	vec3 normal = texture(gNormal, uv).rgb;
	vec4 worldRoughness = texture(gWorldRoughness, uv);
	vec4 emissiveMetalness = texture(gEmissiveMetalness, uv);

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = albedoAO.rgb;
	pbrMaterial.N = normalize(normal.xyz * 2.0 - 1.0);
	pbrMaterial.V = normalize(camPos - worldRoughness.xyz); // TODO - negative world pos is clamped to 0?
	pbrMaterial.linearRoughness = worldRoughness.a;
	pbrMaterial.metalness = emissiveMetalness.a;
	pbrMaterial.F0 = calculateF0(albedoAO.rgb, emissiveMetalness.a);
	pbrMaterial.ao = albedoAO.a;

	PBRLight pbrLight;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a;
	vec3 lightDir = lightPos - worldRoughness.xyz; // TODO - might break with negative world pos
	float lightDistance = length(lightDir);
	pbrLight.L = normalize(lightDir / lightDistance);
	if (lightDistance > lightRadius) {
		return;
	}
	float att = 1 / pow(1.0 + lightDistance / lightRadius, 2);
	pbrLight.radiance *= att;

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);
	brdf(pbrMaterial, pbrLight, pbrColor);

	color.rgb = vec3(0)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
		//+ emissiveMetalness.rgb TODO - move to indirect light pass
		;

}

