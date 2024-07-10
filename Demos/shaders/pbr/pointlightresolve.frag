#include "pbr.glsl"
#include "shadowreceiver.glsl"

in vec4 fragPos;

layout(binding = 0) uniform sampler2D gAlbedoAO;
layout(binding = 1) uniform sampler2D gNormalRoughness;
layout(binding = 2) uniform sampler2D gEmissiveMetalness;
layout(binding = 3) uniform sampler2D gDepth;

uniform mat4 invP;
uniform mat4 invV;
uniform vec3 camPos;
uniform vec2 resolution;

uniform vec4 lightRadiance;
uniform vec3 lightPos;

#ifdef ENABLE_SHADOWS
layout(binding = 4) uniform samplerCube shadowCube;
uniform float shadowRange;
#endif

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
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	vec4 albedoAO = texture(gAlbedoAO, uv);
	vec4 normalRoughness = texture(gNormalRoughness, uv);
	vec4 emissiveMetalness = texture(gEmissiveMetalness, uv);
	float depth = texture(gDepth, uv).r;
	vec3 worldPos = reconstructWorldPos(uv, depth, invP, invV);

#ifdef SHOW_LIGHTS
	float rayDist = raySphereIntersect(camPos, normalize(fragPos.xyz - camPos), lightPos, debugRadius);
	if (rayDist > 0.0 && distance(camPos, worldPos) > rayDist) {
		color = vec4(lightRadiance.rgb * lightRadiance.a, 1.f);
		return;
	}
#endif

	PBRMaterial pbrMaterial;
	pbrMaterial.albedo = albedoAO.rgb;
	pbrMaterial.N = normalize(normalRoughness.xyz * 2.0 - 1.0);
	pbrMaterial.V = normalize(camPos - worldPos);
	pbrMaterial.linearRoughness = normalRoughness.a;
	pbrMaterial.metalness = emissiveMetalness.a;
	pbrMaterial.F0 = calculateF0(albedoAO.rgb, emissiveMetalness.a);
	pbrMaterial.ao = albedoAO.a;

	vec3 lightDir = lightPos - worldPos;
	float lightDistance = length(lightDir) + EP;
	PBRLight pbrLight;
	pbrLight.radiance = lightRadiance.rgb * lightRadiance.a / (lightDistance * lightDistance);
	pbrLight.L = normalize(lightDir / lightDistance);

	PBRColor pbrColor;
	pbrColor.directDiffuse = vec3(0);
	pbrColor.directSpecular = vec3(0);
	pbrColor.indirectDiffuse = vec3(0);
	pbrColor.indirectSpecular = vec3(0);
	brdf(pbrMaterial, pbrLight, pbrColor);

#ifdef ENABLE_SHADOWS
	if (sign(dot(pbrMaterial.N, pbrMaterial.V)) > 0.0) {
		vec3 shadowCoord = worldPos - lightPos;
		float visibility = getShadowVisibility(4, shadowCube, shadowCoord, 512, shadowRange, 0.00);
		pbrColor.directDiffuse *= visibility;
		pbrColor.directSpecular *= visibility;
	}
#endif

	color.rgb = vec3(0)
		+ pbrColor.directDiffuse
		+ pbrColor.directSpecular
	;
	color.a = 1.0;
}

