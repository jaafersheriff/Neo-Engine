#include "alphaDiscard.glsl"
#include "phong.glsl"

in vec4 fragPos;
in vec3 fragNor;
in vec2 fragTex;

uniform vec4 albedo;

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
	// TODO - normal mapping
	vec3 N = normalize(fragNor);
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

	color.rgb = getPhong(V, N, L, albedo.rgb * 0.2, albedo.rgb, vec3(1.0), 13.0, lightCol, attFactor);

	color.a = 1.0;
}

