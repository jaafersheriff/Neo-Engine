#include "shadowreceiver.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec2 fragTex;

#ifdef ENABLE_SHADOWS
uniform vec2 shadowMapResolution;
uniform mat4 lightTransform;
layout(binding = 4) uniform sampler2D shadowMap;
#endif

layout(binding = 0) uniform sampler2D gAlbedo;
layout(binding = 2) uniform sampler2D gWorld;
layout(binding = 3) uniform sampler2D gNormal;

uniform vec3 camPos;

uniform vec3 lightCol;
uniform vec3 lightDir;

out vec4 color;

void main() {
	vec4 albedo = texture(gAlbedo, fragTex);
	vec3 worldPos = texture(gWorld, fragTex).rgb;
	vec3 norm = texture(gNormal, fragTex).rgb;

	// TODO - normal mapping
	vec3 N = normalize(norm.xyz);
	vec3 V = normalize(camPos - worldPos.xyz);

	float attFactor = 1;
	vec3 L = normalize(lightDir);

	color.rgb = lambertianDiffuse(L, N, albedo.rgb, lightCol, attFactor);

#ifdef ENABLE_SHADOWS
	vec4 shadowCoord = lightTransform * vec4(worldPos, 1.0);
	float visibility = max(getShadowVisibility(1, shadowMap, shadowMapResolution, shadowCoord, 0.002), 0.2);
	color.rgb *= visibility;
#endif

	color.a = 1.0;
}

